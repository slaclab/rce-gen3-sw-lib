/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP BDI Test Driver.
**
**  Implementation of PGP BDI test driver.
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - November 09, 2015
**
**  Revision History:
**	None.
**
** --
*/

#include <strings.h>
#include <stdint.h>
#include <stdio.h>

#define __need_getopt_newlib
#include <getopt.h>

#include <rtems/shell.h>

#include "elf/linker.h"
#include "pgp/Bdi.h"

#define VIRT_CHAN     -1

#define LINK          -1

#define GTC_REGISTER  0xF8F00200

#define READ_GTC(_val) _val = *gtc

#define RESOURCE_COUNT   64
#define RESOURCE_SIZE    (1024*1024) // default 1Mbyte pages

static int  _processor(BDI_Page*, BDI_Arg arg);

static void _open(uint32_t);
static void _close(void);
static void _reset(uint32_t);
static void _measure(void);
static void _bind(uint32_t);
static void _wait(uint32_t);
static void _dump(uint32_t);

static void _usage(void);
static void _addCommands(void);

static volatile uint32_t *gtc = (uint32_t*)GTC_REGISTER;

int _vc = VIRT_CHAN;

int _link = LINK;

uint32_t _psize = RESOURCE_SIZE; // page size

uint64_t _page_cnt = 0;
uint64_t _page_elp = 0;

uint32_t _page_st  = 0;
uint32_t _page_et  = 0;

uint32_t _page_bytes = 0;

uint32_t _tid      = 0;  // transaction ID
uint32_t _output   = 1;  // enable/disable for print output

static int _processor(BDI_Page *page, BDI_Arg arg)
  {  
  if(_output)
    {
    printf("BDI_Test %s page 0x%x link %d arg 0x%x\n",__func__,page,page->link,arg);
    printf("  size %d seq %u status 0x%x\n",page->size,page->seq,page->status);
    if(_output > 1)
      {
      int i;
      uint32_t *data = (uint32_t*)page->data;
      for(i=0;i<page->size/sizeof(uint32_t);i++)
        printf("  data[%d] 0x%x-0x%x\n",i,&data[i],data[i]);
      }
    }
      
  return 0;
  }

/*
** ++
**
**
** --
*/

static void _open(uint32_t link)
  {
  int status;
  
  if(_vc < 0)
    {
    printf("BDI_Test: set vc before opening device\n");
    return _usage();
    }
    
  status = BDI_Open(_vc,link,RESOURCE_COUNT,_psize,1);
  if(status)
    printf("BDI_Test: error in BDI_Open %d\n",status);
    
  _link = link;
  }

/*
** ++
**
**
** --
*/

static void _close(void)
  {
  BDI_Close();
  }


/*
** ++
**
**
** --
*/

static void _bind(uint32_t arg)
  {
  if(_vc < 0)
    {
    printf("BDI_Test: set vc before binding device\n");
    return _usage();
    }

  if(_link < 0)
    {
    printf("BDI_Test: open link before binding device\n");
    return _usage();
    }
  
  BDI_Bind(_vc,_link,(BDI_Processor)_processor,(BDI_Arg)arg);
  }

/*
** ++
**
**
** --
*/

static void _measure(void)
  {
  READ_GTC(_page_et);
  if(_page_st)
    {
    if(_page_st>_page_et)
      _page_elp += (0xffffffff-_page_st) + _page_st;
    else
      _page_elp += _page_et-_page_st;
    }    
  _page_st   = _page_et;
  _page_cnt++;
  
  }
  
/*
** ++
**
**
** --
*/

static void _wait(uint32_t arg)
  {
  if(_vc < 0)
    {
    printf("BDI_Test: set vc before waiting on device\n");
    return _usage();
    }
  
  do {  
    BDI_Page *page = BDI_Wait(_vc,_link);
    
    _page_bytes+=page->size;
    
    _measure();
    
    if(_output)
      {
      printf("BDI_Test %s page 0x%x link %d\n",__func__,page,page->link);
      printf("  size %d seq %u status 0x%x\n",page->size,page->seq,page->status);
      }

    BDI_Free(page);
    }  while(arg);
  }

/*
** ++
**
**
** --
*/

static const char line0[] = "BDI test driver timing\n";
static const char line1[] = "  Page count            %llu\n";
static const char line2[] = "  Elapsed tics(2.5ns)   %llu\n";
static const char line3[] = "  Average tics/page     %llu\n";
static const char line4[] = "  Throughput            %.3f MB/s\n";
static const char line5[] = "                        %.3f Mbps\n";

static void _dump(uint32_t link)
  {
  uint64_t avg = 0;
  double tput = 0;

  if(_vc < 0)
    {
    printf("BDI_Test: set vc before dumping device\n");
    return _usage();
    }

  BDI_Dump(_vc,link);
  
  if(_page_cnt)
    avg =  _page_elp/_page_cnt;

  if(_page_bytes)
    {
    tput = (_page_bytes/1024)/1024;
    if(tput)
      tput = tput/(0.000000001*(_page_elp*2.5)); //2.5ns per tic
    printf(line0);  
    printf(line1,_page_cnt);
    printf(line2,_page_elp);  
    printf(line3,avg);
    printf(line4,tput);
    printf(line5,tput*8);
    }  
  }

/*
** ++
**
**
** --
*/

static void _reset(uint32_t link)
  {
  if(_vc < 0)
    {
    printf("BDI_Test: set vc before resetting device\n");
    return _usage();
    }
  
  BDI_Reset(_vc,link);
  
  _page_cnt = 0;
  _page_st  = 0;
  _page_et  = 0;
  _page_elp = 0;  
  _page_bytes = 0;
  }

/*
** ++
**
** This function is the image constructor
**
** --
*/

unsigned lnk_prelude(void* attributes, void* elf)
  {
  _addCommands();
  return 0;
  }
  
/*
** ++
**
** BDI test command usage
** 
** --
*/

const char USAGE_BDI_TEST[] =
  "\n  BDI Test Driver\n\
  Usage: bdi_test [OPTIONS] [args]\n\
    -v <vc>            Set device virtual channel\n\
    -o <link>          Open device link\n\
    -c                 Close device(s)\n\
    -s <size>          Set page size (bytes)\n\
    -d <link>          Dump device link statistics\n\
    -r <link>          Reset link statistics\n\
    -p <0|1>           Enable(1) or disable(2) bulk data print output\n\
    -b <arg>           Bind processor and arg to device\n\
    -t <0|1>           Wait for page and return(0), or wait forever(1)\n";

static void _usage(void)
  {
  optind = 0;
  printf("%s",USAGE_BDI_TEST);
  }

/*
** ++
**
** This function executes the bdi_test shell command
**
** --
*/
  
void _main(int argc, char** argv)
  {
  int theOpt;

  if(argc == 1) return _usage();

  struct getopt_data getopt_reent;
  memset(&getopt_reent, 0, sizeof(getopt_data));
  
  while ( (theOpt = getopt_r(argc,argv,
                             "o:cb:t:d:r:p:s:v:", 
                             &getopt_reent)) != EOF )
    {  
    switch (theOpt)
      {      
      case 'o': _open(strtoul(getopt_reent.optarg, 0, 0));     break;
      case 'c': _close();                                      break;
      case 's': _psize = strtoul(getopt_reent.optarg, 0, 0);   break;
      case 'r': _reset(strtoul(getopt_reent.optarg, 0, 0));    break;
      case 'b': _bind(strtoul(getopt_reent.optarg, 0, 0));     break;
      case 'p': _output = (strtoul(getopt_reent.optarg, 0, 0));break;
      case 't': _wait(strtoul(getopt_reent.optarg, 0, 0));     break;      
      case 'd': _dump(strtoul(getopt_reent.optarg, 0, 0));     break;
      case 'v': _vc = strtoul(getopt_reent.optarg, 0, 0);      break;
      default:  return _usage();
      }
    }    
  }

/*
** ++
**
** This function registers BDI test commands with the rtems shell.
**
** --
*/

static const char _cmd[]   = "bdi_test";
static const char _topic[] = "lsst";

static void _addCommands(void)
  {
  rtems_shell_cmd_t * retCmd;
  // Call the long form for adding this command.  This
  // allocates the structure space inside RTEMS

  retCmd = rtems_shell_lookup_cmd(_cmd);
  
  /* cannot add command more than once */
  if(retCmd) return;

  retCmd = rtems_shell_add_cmd(_cmd,
                               _topic,                             
                               USAGE_BDI_TEST,                             
                               (rtems_shell_command_t)_main);

  rtems_shell_add_cmd_struct( retCmd );
  }

