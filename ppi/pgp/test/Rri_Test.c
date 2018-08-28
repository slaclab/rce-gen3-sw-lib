/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP RRI Test Driver.
**
**  Implementation of PGP RRI test driver.
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
#include "pgp/Rri.h"

static int  _processor(RRI_Result *result, RRI_Arg arg);

static void _open(uint32_t socket);
static void _close(void);
static void _register(uint32_t);
static void _read(uint32_t reg, uint32_t link);
static void _write(uint32_t reg, uint32_t val, uint32_t link);
static void _writem(uint32_t reg, uint32_t val, uint32_t link);
static void _bis(uint32_t reg, uint32_t val, uint32_t link);
static void _bic(uint32_t reg, uint32_t val, uint32_t link);
static void _post(uint32_t opcode, uint32_t operand, uint32_t link);
static void _postVc(uint32_t opcode, uint32_t operand, uint32_t link,uint32_t vc);
static void _wait(void);
static void _dump(void);

static void _usage(void);
static void _addCommands(void);

RRI_Device dev = (RRI_Device)0;

uint32_t _tid  = 0;

static int _processor(RRI_Result *result, RRI_Arg arg)
  {
  uint32_t opcode  = RRI_OPCODE(result->instr.operation);
  uint32_t address = RRI_ADDRESS(result->instr.operation);
  uint32_t fault   = RRI_FAULT(result->instr.operation);
  
//  printf("RRI_Test %s opcode 0x%x address 0x%x fault 0x%x\n",__func__,opcode,address,fault);
    
//  printf("RRI_Test %s operand 0x%x tid 0x%x arg 0x%x\n",\
//         __func__,
//         instr->operand,
//         tid,
//         arg);
  
  printf("RRI_Test %s opcode %d reg 0x%x val 0x%x fault %d\n",__func__,opcode,address,result->instr.operand,fault);
  
  return 0;
  }

/*
** ++
**
**
** --
*/

static void _open(uint32_t socket)
  {
  
  dev = RRI_Open(socket);
  }

/*
** ++
**
**
** --
*/

static void _close(void)
  {
  if(!dev) return;
  
  RRI_Close(dev);
  
  dev = (RRI_Device)0;
  }


/*
** ++
**
**
** --
*/

static void _bind(uint32_t arg)
  {
  if(dev) RRI_Bind(dev,(RRI_Processor)_processor,(RRI_Arg)arg);
  }


/*
** ++
**
**
** --
*/

static void _read(uint32_t reg, uint32_t link)
  {
  uint32_t val;

  if(!dev) return;
  
  if(!Pgp_LinkIsReady(link)) return;
  
  uint32_t fault = RRI_Read(dev,link,reg,&val);
  
  printf("RRI_Test %s reg 0x%x val 0x%x fault %d\n",__func__,reg,val,fault);
  
  }


/*
** ++
**
**
** --
*/

static void _write(uint32_t reg, uint32_t val, uint32_t link)
  {
  if(!dev) return;
  
  uint32_t fault = RRI_Write(dev,link,reg,val);
  
  printf("RRI_Test %s reg 0x%x val 0x%x fault %d\n",__func__,reg,val,fault);
  }

/*
** ++
**
**
** --
*/

static void _writem(uint32_t reg, uint32_t val, uint32_t link)
  {
  if(!dev) return;
  
  uint32_t cntr = 0;
  while(link)
    {
    if(link & 0x1)
      RRI_Write(dev,cntr,reg,val);
    cntr++;
    link >>= 1;
    }
  }

/*
** ++
**
**
** --
*/

static void _bis(uint32_t reg, uint32_t val, uint32_t link)
  {
  if(!dev) return;
  
  uint32_t fault = RRI_Bis(dev,link,reg,val);
  
  printf("RRI_Test %s reg 0x%x val 0x%x fault %d\n",__func__,reg,val,fault);  
  }

/*
** ++
**
**
** --
*/

static void _bic(uint32_t reg, uint32_t val, uint32_t link)
  {
  if(!dev) return;
  
  uint32_t fault = RRI_Bic(dev,link,reg,val);

  printf("RRI_Test %s reg 0x%x val 0x%x fault %d\n",__func__,reg,val,fault);
  }

/*
** ++
**
**
** --
*/

static void _post(uint32_t opcode, uint32_t operand, uint32_t link)
  {
  if(dev) RRI_Post(dev,link,opcode,operand,_tid);
  }

/*
** ++
**
**
** --
*/

static void _postVc(uint32_t opcode, uint32_t operand, uint32_t link, uint32_t vc)
  {
  if(dev) RRI_PostVc(dev,link,opcode,operand,_tid,vc);
  }

/*
** ++
**
**
** --
*/

void _wait(void)
  {
  if(!dev) return;
  
  RRI_Result *result;
  
  result = RRI_Wait(dev);  
  
  printf("RRI_Test %s link %d tid 0x%x operation 0x%x operand 0x%x fault %d\n",\
         __func__,
         result->link,
         result->tid,
         result->instr.operation,
         result->instr.operand,
         RRI_FAULT(result->instr.operation));
  }

/*
** ++
**
**
** --
*/

void _dump(void)
  {
  if(dev) return RRI_Dump(dev);
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
** RRI test command usage
** 
** --
*/

const char USAGE_RRI_TEST[] =
  "\n  RRI Test Driver\n\
  Usage: rri_test [OPTIONS] [args]\n\
    -o <socket>           Open device on PGP socket(0-2)\n\
    -c                    Close device\n\
    -d                    Dump device\n\
    -b <arg>              Bind processor and arg to device\n\
    -i <tid>              Set transaction ID\n\
    -j <link>             Enable loopback\n\
    -k <link>             Disable loopback\n\
    -r <reg> <link>       Read operation\n\
    -w <reg> <val> <link> Write operation\n\
    -m <reg> <val> <mask> Write operation (multiple links)\n\
    -s <reg> <val> <link> Bit set operation\n\
    -l <reg> <val> <link> Bit clear operation\n\
    -p <operation> <operand> <link>\n\
                          Post instruction to link\n\
    -v <operation> <operand> <link> <vc>\n\
                          Post instruction to link VC\n\
    -t, --wait            Wait for register data\n";           

static void _usage(void)
  {
  optind = 0;
  printf("%s",USAGE_RRI_TEST);
  }

/*
** ++
**
** This function executes the rri_test shell command
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
                             "o:cb:i:r:w:m:p:ts:l:dj:k:v:", 
                             &getopt_reent)) != EOF )
    {  
    switch (theOpt)
      {
      case 'o': _open(strtoul(getopt_reent.optarg,  0, 0));     break;
      case 'c': _close();                                       break;
      case 'b': _bind(strtoul(getopt_reent.optarg,  0, 0));     break;
      case 'i': _tid = (strtoul(getopt_reent.optarg,  0, 0));   break;
      case 't': _wait();                                        break;
      case 'r': if(argc != 4) return _usage();
                _read    (strtoul(getopt_reent.optarg,  0, 0),\
                          strtoul(argv[3], 0, 0)); break;
      case 'p': if(argc != 5) return _usage();
                _post    (strtoul(getopt_reent.optarg,  0, 0),\
                          strtoul(argv[3], 0, 0),\
                          strtoul(argv[4], 0, 0)); break;
      case 'v': if(argc != 6) return _usage();
                _postVc  (strtoul(getopt_reent.optarg,  0, 0),\
                          strtoul(argv[3], 0, 0),\
                          strtoul(argv[4], 0, 0),
                          strtoul(argv[5], 0, 0)); break;
      case 'w': if(argc != 5) return _usage();
                _write   (strtoul(getopt_reent.optarg,  0, 0),\
                          strtoul(argv[3], 0, 0),\
                          strtoul(argv[4], 0, 0)); break;
      case 'm': if(argc != 5) return _usage();
                _writem  (strtoul(getopt_reent.optarg,  0, 0),\
                          strtoul(argv[3], 0, 0),\
                          strtoul(argv[4], 0, 0)); break;
      case 's': if(argc != 5) return _usage();
                _bis     (strtoul(getopt_reent.optarg,  0, 0),\
                          strtoul(argv[3], 0, 0),\
                          strtoul(argv[4], 0, 0)); break;
      case 'l': if(argc != 5) return _usage();
                _bic     (strtoul(getopt_reent.optarg,  0, 0),\
                          strtoul(argv[3], 0, 0),\
                          strtoul(argv[4], 0, 0)); break;
      case 'd': _dump();                           break;
      case 'j': Pgp_SetLoopback(strtoul(getopt_reent.optarg,  0, 0),1);     break;
      case 'k': Pgp_SetLoopback(strtoul(getopt_reent.optarg,  0, 0),0);     break;
      default:  return _usage();
      }
    }
  }

/*
** ++
**
** This function registers RRI test commands with the rtems shell.
**
** --
*/

static const char _cmd[]   = "rri_test";
static const char _topic[] = "pgp";

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
                               USAGE_RRI_TEST,                             
                               (rtems_shell_command_t)_main);

  rtems_shell_add_cmd_struct( retCmd );
  }

