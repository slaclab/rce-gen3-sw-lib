/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP Plugin Driver.
**
**  Implementation of PGP Plugin driver.
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

#include <stdlib.h>
#include <strings.h>
#include <stdint.h>
#include <stdio.h>   // printf...

#define __need_getopt_newlib
#include <getopt.h>

#include <rtems/shell.h>

#include "task/Task.h"
#include "pgp/Pgp.h"

#include "elf/linker.h"
#include "svt/Svt.h"

#include "Pgp_p.h"

#define PLUGIN_PREFS_NAME "SAS_PLUGIN_PREFS"
#define PGP0_ATTRS_NAME   "PGP0_TASK_ATTRS"
#define PGP1_ATTRS_NAME   "PGP1_TASK_ATTRS"
#define PGP2_ATTRS_NAME   "PGP2_TASK_ATTRS"

/*
**
**
*/

typedef struct {
 unsigned      received;   // # of frames received on this link
 PGP_Processor processor;  // Handler for frame received on this link
 uint32_t      arg;        // Argument to handler
} Handler;

/*
**
**
*/

typedef struct {
 SAS_Session session;
 SAS_IbMbx   mbx;
 rtems_id    task;
 uint32_t    sock;
 uint64_t    received;
 uint64_t    errors;
 uint64_t    rx[PGP_NUMOF_VCS][PGP_NUMOF_LANES];
 Handler     handler[PGP_NUMOF_VCS];
} Plugin;

/*
** Forward declarations for local functions...
*/

static unsigned     _construct(SAS_Preferences *prefs, Ldr_elf*);
static unsigned     _destruct(Plugin*, uint32_t error);

static void         _setup(Pgp_Registers*);
static void         _addCommands(void);

static uint32_t     _process(SAS_Message, void*, SAS_IbMbx);
static SAS_IbOpcode _discard(SAS_Frame, SAS_IbMbx, uint32_t arg);

static const char PLUGIN_PREFS[] = PLUGIN_PREFS_NAME;  // Name of SAS plugin prefs
static const char *PGP_TASK_ATTRS[] = {PGP0_ATTRS_NAME,PGP1_ATTRS_NAME,PGP2_ATTRS_NAME};  // Name of PGP plugin task attributes

Plugin *driver[PGP_NUMOF_PLUGINS];

Pgp_Registers *registers[PGP_NUMOF_PLUGINS];
 
/*
** ++
**
**
** --
*/

int const lnk_options = LNK_INSTALL;

unsigned lnk_prelude(void* arg, Ldr_elf* elf)
  {

  SAS_Preferences **prefs;
  SAS_Status       status = SAS_SUCCESS;
  
  bzero(driver,sizeof(driver));
  bzero(registers,sizeof(registers));
    
  /* find the list of plugins to bind */
  SAS_Preferences **list = (SAS_Preferences**)Svt_Translate(PLUGIN_PREFS, SVT_SAS_TABLE);
  if(!list) return status;
  
  /* bind PGP plugins to drivers */
  for(prefs=list;*prefs;prefs++)
    {
    if((!strncmp((*prefs)->name,PGP0_PLUGIN_NAME,sizeof(PGP0_PLUGIN_NAME))) ||
       (!strncmp((*prefs)->name,PGP1_PLUGIN_NAME,sizeof(PGP1_PLUGIN_NAME))) ||
       (!strncmp((*prefs)->name,PGP2_PLUGIN_NAME,sizeof(PGP2_PLUGIN_NAME))))
      {
      status = _construct(*prefs,elf);
      if(status != PGP_SUCCESS)
        return status;
      }
    }
  _addCommands();

  return PGP_SUCCESS;
  }

/*
** ++
**
**
** --
*/

#define ARGC   1   // # of arguments passed to task... 
#define PLUGIN 0   // First and only argument is handle...

static const char _attrs_error[]   = "Failure loading PGP task attrs from %s\n";

static unsigned _construct(SAS_Preferences *prefs, Ldr_elf *elf)
  { 
  int i;
  
  uint32_t sock = strtoul(prefs->bitfile,0,0);
  
  Plugin *plugin = (Plugin*)malloc(sizeof(Plugin));

  if(!plugin) return PGP_INSFMEM;
  
  driver[sock] = plugin;
    
  bzero(plugin,sizeof(Plugin));

  plugin->sock = sock;

  SAS_Session session = SAS_Open();

  if(!(plugin->session = session)) return _destruct(plugin, PGP_NO_SESSION);

  /* bring plugin reset->offline */
  SAS_Off(prefs->name,session); 

  /* bring plugin offline->online */
  SAS_On(prefs->name,session); 

  Pgp_Registers *regs = (Pgp_Registers*)SAS_Registers(prefs->name, session);
  registers[sock] = regs;
  
  /* setup all PGP link registers */
  for(i=0;i<PGP_NUMOF_LANES;i++)
    {
    _setup(&regs[i]);
    }

  for(i=0;i<PGP_NUMOF_VCS;i++)
    {
    plugin->handler[i].received  = 0;
    plugin->handler[i].arg       = 0;
    plugin->handler[i].processor = _discard;
    }    

  SAS_IbMbx mbx = SAS_IbBind(prefs->name, _process, (void*)plugin, session);

  if(!(plugin->mbx = mbx)) return _destruct(plugin, PGP_NO_IBMBX);

  plugin->received = 0;
  plugin->task     = 0;

  Task_Attributes *attributes = (Task_Attributes*)Svt_Translate(PGP_TASK_ATTRS[sock], SVT_SYS_TABLE);
  if(!attributes) 
    {
    printf(_attrs_error,PGP_TASK_ATTRS[sock]);
    return _destruct(plugin, PGP_NO_ATTRS);
    }
    
  const char* argv[ARGC];

  argv[PLUGIN] = (char*)plugin;

  unsigned error = Task_Run(elf, attributes, ARGC, argv, &plugin->task);

  return !error ? PGP_SUCCESS : _destruct(plugin, error);
  }

/*
** ++
**
**
** --
*/

static void _setup(Pgp_Registers* base)
  {
  base->config.cnt_reset = 1;
  base->config.cnt_reset = 0;

  base->config.rx_reset  = 1;
  base->config.rx_reset  = 0;

  base->config.flush     = 1;  
  base->config.flush     = 0;  

  base->config.loopback  = 0;  
  }

/*
** ++
**
**
** --
*/

static unsigned _destruct(Plugin* plugin, uint32_t error)
  { 

  unsigned sock = plugin->sock;
  
  SAS_Session session = plugin->session;

  if(session) SAS_Close(session);

  free((void*)plugin);
  
  driver[sock] = NULL;

  return error;
  }

/*
** ++
**
**
** --
*/

void Task_Start(int argc, const char** argv)
  { 

  Plugin* plugin = (Plugin*)argv[PLUGIN];

  SAS_IbEnable(plugin->mbx);

  SAS_Wait(plugin->session);

  return;
  }

/*
**
**
*/

static const char _link_error[] = "Error detected in PGP frame 0x%x\n";
static const char _vc_error[]   = "Invalid vc %d in PGP link %d fd 0x%x\n";

static uint32_t _process(SAS_Frame frame, void* arg, SAS_IbMbx mbx)
  {
  Plugin*  plugin  = (Plugin*)arg;

  plugin->received++;

  SAS_Fd *fd = SAS_IbFd(frame,mbx);

  uint32_t lane = SAS_TypeOf(frame);
  
  if((lane >= PGP_NUMOF_LANES) || (SAS_IbError(frame)))
    {
    plugin->errors++;
    printf(_link_error,frame);
    SAS_IbPost(_discard(frame,0,0),frame,mbx);
    return SAS_REENABLE;
    }

  uint8_t link = PGP_ENCODE_LINK(plugin->sock,lane);  
  
  /* Re-encode frame source as a link, this is a bit kludgy! */
  frame = SAS_ObSet(frame, link, 0);

  Pgp_Header* hdr = (Pgp_Header*)((uint32_t)SAS_IbFd(frame, mbx)+sizeof(SAS_Fd));

  uint32_t vc = hdr->dst;

  if(vc >= PGP_NUMOF_VCS)
    {
    plugin->errors++;
    printf(_vc_error,vc,link,fd);
    SAS_IbPost(_discard(frame,0,0),frame,mbx);
    return SAS_REENABLE;
    } 

  plugin->rx[vc][lane]++;
  
  Handler* handler = &plugin->handler[vc];

  handler->received++; 

  SAS_IbOpcode opcode = (*handler->processor)(frame, mbx, handler->arg);

  SAS_IbPost(opcode, frame, mbx);

  return SAS_REENABLE;  
  } 

/*
**
**
*/

static SAS_IbOpcode _discard(SAS_Frame frame, SAS_IbMbx mbx, uint32_t arg)
  {
  return SAS_IbPayload(frame) ? SAS_IB_FLUSH : SAS_IB_FREE;
  } 
 
/*
** ++
**
**
** --
*/

void Task_Rundown() 
  { 
  return;
  }

/*
** ++
**
**
** --
*/

void Pgp_Bind(PGP_Processor processor, uint32_t vc, uint32_t arg)
  {
  int i;
  
  for(i=0; i<PGP_NUMOF_PLUGINS; i++)
    {
    if(vc < PGP_NUMOF_VCS)
      {
      if(!driver[i]) continue;
      driver[i]->handler[vc].processor = processor;
      driver[i]->handler[vc].arg = arg;    
      }
    }
  }

/*
** ++
**
**
** --
*/

void Pgp_Unbind(uint32_t vc)
  {
  int i;
  
  for(i=0; i<PGP_NUMOF_PLUGINS; i++)
    { 
    if(vc < PGP_NUMOF_VCS)
      {
      if(!driver[i]) continue;
      driver[i]->handler[vc].processor = _discard;
      driver[i]->handler[vc].arg = 0;    
      }
    }
  }

/*
** ++
**
**
** --
*/

int Pgp_LinkIsReady(uint32_t link)
  {
  uint8_t sock = PGP_DECODE_SOCK(link);
  uint8_t lane = PGP_DECODE_LANE(link);  

  Pgp_Registers* regs = registers[sock];

  Pgp_Config* config = &(regs[lane].config);
  
  return Pgp_IS(PGP_LOCAL_READY,config->link_status);
  }

/*
** ++
**
**
** --
*/

uint32_t Pgp_GetRemoteLinkData(uint32_t link)
  {
  uint8_t sock = PGP_DECODE_SOCK(link);
  uint8_t lane = PGP_DECODE_LANE(link);  

  Pgp_Registers* regs = registers[sock];

  Pgp_Counters* counters = &(regs[lane].counters);
  
  return counters->rlink_data;
  }

/*
** ++
**
**
** --
*/

uint32_t Pgp_GetSideband(uint32_t link)
  {
  uint8_t sock = PGP_DECODE_SOCK(link);
  uint8_t lane = PGP_DECODE_LANE(link);  

  Pgp_Registers* regs = registers[sock];

  Pgp_Config* config = &(regs[lane].config);
  
  return config->sideband;
  }

/*
** ++
**
**
** --
*/

void Pgp_SetSideband(uint32_t link, uint32_t value)
  {
  uint8_t sock = PGP_DECODE_SOCK(link);
  uint8_t lane = PGP_DECODE_LANE(link);  
    
  Pgp_Registers* regs = registers[sock];

  Pgp_Config* config = &(regs[lane].config);
  
  config->sideband = value;
  }

/*
** ++
**
**
** --
*/

void Pgp_SetLoopback(uint32_t link, uint32_t value)
  {
  uint8_t sock = PGP_DECODE_SOCK(link);
  uint8_t lane = PGP_DECODE_LANE(link);  
    
  Pgp_Registers* regs = registers[sock];

  Pgp_Config* config = &(regs[lane].config);
  
  config->loopback = value;
  }

/*
** ++
**
**
** --
*/

static const char _plugin_info[]    = "PGP plugin driver:\n";
static const char _plugin_stats[]   = "  Rx frame count       %llu\n";
static const char _plugin_errors[]  = "  Error frame count    %llu\n";

static const char _link_stats[]     = "  Link %d statistics:\n";
static const char _rx_frames[]      = "    Rx frame count VC%d %llu\n";

void Pgp_Stats(uint32_t link, int reset)
  {
  int i,j;

  uint8_t sock = PGP_DECODE_SOCK(link);
  uint8_t lane = PGP_DECODE_LANE(link);  

  Plugin *plugin = driver[sock];
  
  if(!plugin) return;
    
  if(reset)
    {
    for(i=0;i<PGP_NUMOF_VCS;i++)
      plugin->rx[i][lane] = 0;
    return;
    }
  printf(_plugin_info);
  printf(_plugin_stats,plugin->received);
  printf(_plugin_errors,plugin->errors);
  
  printf(_link_stats,link);
  for(i=0;i<PGP_NUMOF_VCS;i++)
    printf(_rx_frames,i,plugin->rx[i][lane]);
  }
  
  
/*
** ++
**
**
** --
*/

const char USAGE_PGP[] =
  "\n  Pgp Plugin Driver\n\
  Usage: pgp [OPTIONS] [args]\n\
    -s, --stats <link>       Dump plugin driver statistics\n\
    -r, --reset <link>       Reset plugin driver statistics\n\
    -d, --dump  <link>       Dump plugin link registers\n";

/*
** ++
**
**
** --
*/

void _usage(void)
  {
  optind = 0;
  printf("%s",USAGE_PGP);
  }

/*
** ++
**
** This function executes the PGP shell command
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
                             "d:s:r:", 
                             &getopt_reent)) != EOF )
    {  
    switch (theOpt)
      {
      case 'd': 
        Pgp_Dump(strtoul(getopt_reent.optarg, 0, 0));   break;
      case 's': 
        Pgp_Stats(strtoul(getopt_reent.optarg,  0, 0),\
                  0); break;
      case 'r':
        Pgp_Stats(strtoul(getopt_reent.optarg,  0, 0),\
                  1); break;
      default:  return _usage();
      }
    }
  }

/*
** ++
**
** This function registers PGP commands with the rtems shell.
**
** --
*/

static const char _cmd[]   = "pgp";
static const char _topic[] = "ppi";

void _addCommands(void)
  {
  rtems_shell_cmd_t * retCmd;
  // Call the long form for adding this command.  This
  // allocates the structure space inside RTEMS
  
  retCmd = rtems_shell_lookup_cmd(_cmd);

  /* cannot add command more than once */
  if(retCmd) return;

  retCmd = rtems_shell_add_cmd(_cmd,
                               _topic,
                               USAGE_PGP,                             
                               (rtems_shell_command_t)_main);

  rtems_shell_add_cmd_struct( retCmd );
  }
