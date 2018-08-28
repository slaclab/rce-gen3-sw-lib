/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP RRI Emulator Test Driver (RRIEM).
**
**  Implementation of PGP RRI Emulator test driver.
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - Feb 20, 2017
**
**  Revision History:
**	None.
**
** --
*/

#include <strings.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#define __need_getopt_newlib
#include <getopt.h>

#include <rtems/shell.h>

#include "system/fs.h"
#include "elf/linker.h"
#include "pgp/Pgp.h"
#include "pgp/Rri.h"
#include "pgp/RriEm.h"
#include "BdiEm_Test.h"

//#define USE_REMOTE_FS
#define USE_LOCAL_FS
#define USE_SD
//#define USE_NFS

#define RRIEM_PREFS_NAME      "RRIEM_CONFIG"
#define RRIEM_PREFS_FILE      "config:rriEm.svt"

static void _open(uint32_t link);
static void _bind(uint32_t reg, uint32_t link);
static void _close(void);
static void _read(uint32_t reg, uint32_t link);
static void _write(uint32_t reg, uint32_t val, uint32_t link);
static void _bis(uint32_t reg, uint32_t val, uint32_t link);
static void _bic(uint32_t reg, uint32_t val, uint32_t link);
static void _wait(void);
static void _dump(uint32_t socket);
static void _usage(void);
static void _addCommands(void);

RRIEM_Device _dev = (RRIEM_Device)0;

RRIEM_Cfg **_cfg;

const char SCRATCH_MOUNT_POINT[]  = "/mnt/scratch";

static const char RRIEM_SVT[] = RRIEM_PREFS_FILE; // Name of RriEm svt file

static const char RRIEM_PREFS[] = RRIEM_PREFS_NAME;  // Name of RriEm prefs

/*
** ++
**
**
** --
*/

const char _mount_error1[] = "SD mount point create failure 0x%x errno 0x%x: %s\n";
const char _mount_error2[] = "SD mount failure 0x%x: %s to %s\n";

static void _mount_scratch(const char* name)
  {
  int error = rtems_mkdir(name, S_IRWXU|S_IRWXG|S_IRWXO);

  if(error) dbg_bugcheck(_mount_error1, error, strerror(errno), name);

  error = mount(SD_PARTITION_SCRATCH, name, SD_PARTITION_TYPE_SCRATCH, RTEMS_FILESYSTEM_READ_WRITE, NULL);

  if(error) dbg_bugcheck(_mount_error2, error, SD_PARTITION_SCRATCH, name);
  }

/*
** ++
**
**
** --
*/

static uint32_t _process(RRI_Result *result)
  {
    
  uint32_t operation = result->instr.operation;
  uint32_t operand   = result->instr.operand;
        
  if(RRI_OPCODE(operation) == RRI_GET)
    RRIEM_Read(_dev,result->link,RRI_ADDRESS(operation),&result->instr.operand);
  else if(RRI_OPCODE(operation) == RRI_PUT)
    {
    RRIEM_Write(_dev,result->link,RRI_ADDRESS(operation),operand);    
    /* a write to register 9 with value 4 triggers BdiEm_test file transmit */
    if((RRI_ADDRESS(result->instr.operation) == 9) && (result->instr.operand == 4))    
      BDIEMT_Trigger(result->link);
    }
  else if(RRI_OPCODE(operation) == RRI_BIS)
    RRIEM_Bis(_dev,result->link,RRI_ADDRESS(operation),operand);
  else if(RRI_OPCODE(operation) == RRI_BIC)
    RRIEM_Bic(_dev,result->link,RRI_ADDRESS(operation),operand);

  //printf("rriEm_test %s reg 0x%x val 0x%x\n",__func__,result->instr.operation,result->instr.operand);

  return 0;
  }
  
/*
** ++
**
**
** --
*/

static int _vc = 1;
static const char _ip[] = "192.168.204.1";

static void _open(uint32_t link)
  {
  
  if(!_dev)
    {
    RRIEM_Device device = RRIEM_Open(PGP_DECODE_SOCK(link),_cfg);
    if(!device)
      {
      printf("rriEm_test %s: device open failure\n",__func__);
      return;
      }
    _dev = device;
    }

  BDIEMT_Vc(_vc);
  BDIEMT_Ip((char*)_ip);
  BDIEMT_Open(link);  
  }

/*
** ++
**
**
** --
*/

static void _bind(uint32_t reg, uint32_t link)
  {
  
  if(_dev)
    RRIEM_Bind(_dev,link,_process,reg);
  }

/*
** ++
**
**
** --
*/

static void _file(char *filename, uint32_t link)
  {
  
  if(_dev)    
    {
    BDIEMT_File(filename, link);

#ifdef USE_REMOTE_FS
    /* preload file from network server */
    BDIEMT_Remote(link);
#endif
    
#ifdef USE_LOCAL_FS
    /* preload file from local filesystem */
    BDIEMT_Local(link);
#endif
    }
  }

/*
** ++
**
**
** --
*/

static void _close(void)
  {
  
  if(!_dev) return;
  
  RRIEM_Close(_dev);
  
  _dev = (RRIEM_Device)0;
  }

/*
** ++
**
**
** --
*/

static void _read(uint32_t reg, uint32_t link)
  {
  
  uint32_t val = 0;

  if(!_dev) return;
    
  uint32_t fault = RRIEM_Read(_dev,link,reg,&val);
  }


/*
** ++
**
**
** --
*/

static void _write(uint32_t reg, uint32_t val, uint32_t link)
  {
  
  if(!_dev) return;
  
  uint32_t fault = RRIEM_Write(_dev,link,reg,val);
  }

/*
** ++
**
**
** --
*/

static void _bis(uint32_t reg, uint32_t val, uint32_t link)
  {
  
  if(!_dev) return;
  
  uint32_t fault = RRIEM_Bis(_dev,link,reg,val);
  }

/*
** ++
**
**
** --
*/

static void _bic(uint32_t reg, uint32_t val, uint32_t link)
  {
  
  if(!_dev) return;
  
  uint32_t fault = RRIEM_Bic(_dev,link,reg,val);
  }

/*
** ++
**
**
** --
*/

static void _wait(void)
  {
  
  if(!_dev) return;
  
  RRI_Result *result;
  
  while(1)
    result = RRIEM_Wait(_dev);
  }

/*
** ++
**
**
** --
*/

static void _dump(uint32_t socket)
  {
  
  RRIEM_Dump(socket);
  }

/*
** ++
**
** This function is the image constructor
**
** --
*/

#ifdef USE_REMOTE_FS
char f0[] = "emu_image0.bin";
char f1[] = "emu_image1.bin";
char f2[] = "emu_image2.bin";
#endif

#ifdef USE_LOCAL_FS
#ifdef USE_SD
char f0[] = "/mnt/scratch/emu_image0.bin";
char f1[] = "/mnt/scratch/emu_image1.bin";
char f2[] = "/mnt/scratch/emu_image2.bin";
#endif

#ifdef USE_NFS
char f0[] = "/mnt/nfs/emu_image0.bin";
char f1[] = "/mnt/nfs/emu_image1.bin";
char f2[] = "/mnt/nfs/emu_image2.bin";
#endif
#endif

unsigned lnk_prelude(void* attributes, void* elf)
  {
  
  _addCommands();

  int error = Svt_Install(SVT_RRIEM_TABLE_NUM, RRIEM_SVT);
  if(error)
    return RRIEM_INSTALL;
  
  _cfg = (RRIEM_Cfg**)Svt_Translate(RRIEM_PREFS, SVT_RRIEM_TABLE);
  if(_cfg == NULL)
    return RRIEM_TRANSLATE;

  _mount_scratch(SCRATCH_MOUNT_POINT);
  
  _open(0);
  _open(1);
  _open(2);
  
  _bind(9,0);
  _bind(9,1);
  _bind(9,2);

  _file(f0,0);
  _file(f1,1);
  _file(f2,2);

  rtems_status_code sc;
  rtems_id tid;

  sc = rtems_task_create(rtems_build_name('R','E','M','T'),
                         80,
                         RTEMS_MINIMUM_STACK_SIZE * 10,
                         RTEMS_DEFAULT_ATTRIBUTES,
                         RTEMS_DEFAULT_MODES,
                         &tid);

  sc = rtems_task_start(tid, (rtems_task_entry)_wait, 0);
    
  return 0;
  }
  
/*
** ++
**
** RRIEM test command usage
** 
** --
*/

const char USAGE_RRIEM_TEST[] =
  "\n  RRI Emulator Test Driver\n\
  Usage: rriEm_test [OPTIONS] [args]\n\
    -o <socket>           Open device on PGP socket(0-2)\n\
    -b <reg> <link>       Bind a processor to a register access\n\
    -c                    Close device\n\
    -d <socket>           Dump device statistics\n\
    -r <reg> <link>       Read operation\n\
    -w <reg> <val> <link> Write operation\n\
    -s <reg> <val> <link> Bit set operation\n\
    -l <reg> <val> <link> Bit clear operation\n\
    -f <filename> <link>  Load BdiEm_Test file\n\
    -t                    Wait forever for register data\n";           

static void _usage(void)
  {
  
  optind = 0;
  printf("%s",USAGE_RRIEM_TEST);
  }

/*
** ++
**
** This function executes the rriEm_test shell command
**
** --
*/

  
void rriEm_test(int argc, char** argv)
  {
  
  int theOpt;

  if(argc == 1) return _usage();

  struct getopt_data getopt_reent;
  memset(&getopt_reent, 0, sizeof(getopt_data));
  
  while ( (theOpt = getopt_r(argc,argv,
                             "o:b:cr:w:ts:l:d:f:", 
                             &getopt_reent)) != EOF )
    {  
    switch (theOpt)
      {
      case 'o': _open(strtoul(getopt_reent.optarg,  0, 0)); break;
      case 'b': if(argc != 4) return _usage();
                _bind  (strtoul(getopt_reent.optarg,  0, 0),\
                         strtoul(argv[3], 0, 0));  break;
      case 'f': if(argc != 4) return _usage();
                _file  (getopt_reent.optarg,\
                         strtoul(argv[3], 0, 0));  break;
      case 'c': _close();                          break;
      case 't': _wait();                           break;
      case 'r': if(argc != 4) return _usage();
                _read   (strtoul(getopt_reent.optarg,  0, 0),\
                         strtoul(argv[3], 0, 0));  break;
      case 'w': if(argc != 5) return _usage();
                _write   (strtoul(getopt_reent.optarg,  0, 0),\
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
      case 'd': _dump(strtoul(getopt_reent.optarg,  0, 0)); break;
      default:  return _usage();
      }
    }
  }

/*
** ++
**
** This function registers RRI emulator test commands with the rtems shell.
**
** --
*/

static const char _cmd[]   = "rriEm_test";
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
                               USAGE_RRIEM_TEST,                             
                               (rtems_shell_command_t)rriEm_test);

  rtems_shell_add_cmd_struct( retCmd );
  }

