/*
** ++
**  Package: SAS
**	
**
**  Abstract: SAS shell commands
**
**  Implementation of RTEMS SAS facility. See the corresponding header (".h") file for a
**  more complete description of the functions implemented below.      
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - Sept 27, 2014
**
**  Revision History:
**	None.
**
** --
*/


#include <getopt.h>

#include <rtems.h>
#include <rtems/shell.h>

#include "memory/mem.h"
#include "kvt/Kvt.h"

#include "sas/Sas.h"

#include "../Map.h"

const char USAGE_SAS[] =
 "Usage: sas [OPTIONS]\n\
  Modify/dump SAS sessions, mailboxes, and sockets.\n\
    -s, --session <0-31>             Dump session and mailboxes\n\
    -a, --attrs   <plugin_name>      Dump socket attributes\n\
    -p, --prefs,  <plugin_name>      Dump socket preferences\n\
    -r, --errors  <plugin_name>      Dump socket errors\n\
    -m, --metrics <plugin_name>      Dump socket metrics\n\
    -f, --faults  <plugin_name>      Dump socket faults\n\
    -c, --control                    Dump SAS control map\n\
    -e, --enable  <0-31>             Enable SAS_Wait session profiling\n\
    -d, --disable <0-31>             Disable SAS_Wait session profiling\n";

void _usage(void)
  {
  optind = 0;
  printf("%s",USAGE_SAS);
  }

/*
** ++
**
** This function returns the offset of a socket
**
** --
*/

const char lookup_error[] = "Plugin name %s not found\n";

static int _lookupSocket(const char* name)
  {
  int offset = (uint32_t)KvtLookup(KvtHash(name),(KvTable)((unsigned)SasOcm+SasOcmMap->socketKvt));
  if(offset)
    return (offset-SOCKET_AXI_FIFO_BASE-SOCKET_AXI_BASE)/SOCKET_AXI_SIZE;
  else
    {
    printf(lookup_error,name);
    return -1;
    }
  }

/*
** ++
**
** This function enables or disable SAS session profiling
**
** --
*/

const char session_error[] = "Session %d is not valid\n";

static void _profileSession(uint32_t idx, uint32_t enab)
  {
  if(idx >= MAX_SESSIONS)
    {
    printf(session_error,idx);
    return _usage();
    }
  
  if(enab)
    {
    /* reset the profiling counters at enable */
    SasOcmMap->sessions[idx].valid   = 0;
    SasOcmMap->sessions[idx].invalid = 0;  
    SasOcmMap->sessions[idx].wait    = 0;
    SasOcmMap->sessions[idx].disable = 0;
    }    
    
  SasOcmMap->sessions[idx].profile = enab ? 1 : 0;
  }

/*
** ++
**
** This function dumps a mailbox.
**
** --
*/

static const char _line0[]  = "    Mailbox handle            : 0x%x\n";
static const char _line1[]  = "    Mapped ocm base           : 0x%x\n";
static const char _line2[]  = "    Session index             : %u\n";
static const char _line3[]  = "    Mid (rundown only)        : %u\n";
static const char _line4[]  = "    Interrupt enable register : 0x%x\n";
static const char _line5[]  = "    Interrupt enable offset   : 0x%x\n";
static const char _line6[]  = "    Interrupt base            : 0x%x\n";
static const char _line7[]  = "    Pending fifo address      : 0x%x\n";
static const char _line8[]  = "    Callback handler          : 0x%x\n";
static const char _line9[]  = "    Socket address            : 0x%x\n";
static const char _line10[] = "    Context value             : 0x%x\n";

static void _dumpMailbox(Mbx m)
  {
  printf(_line0, m);
  printf(_line1, m->ocm);
  printf(_line2, m->idx);
  printf(_line3, m->mid);
  printf(_line4, m->enab);
  printf(_line5, m->mask);
  printf(_line6, m->intrBase);
  printf(_line7, m->pend);
  printf(_line8, m->handler);
  printf(_line9, m->socket);
  printf(_line10,m->context);
  }

/*
** ++
**
** This function dumps a session.
**
** --
*/

static const char _line11[] = "  Session ID      : %d\n  Session handle  : 0x%x\n";
static const char _line12[] = "  Mailbox enable  : 0x%x\n";
static const char _line13[] = "  Profile enable  : 0x%x\n";
static const char _line14[] = "  Mailbox count   : %u\n";
static const char _line15[] = "  Sync handle     : 0x%x\n";
static const char _line16[] = "  Mapped axi base : 0x%x\n";
static const char _line17[] = "  Mapped ocm base : 0x%x\n";
static const char _line18[] = "  Profile valid   : %u\n";
static const char _line19[] = "  Profile invalid : %u\n";
static const char _line20[] = "  Profile wait    : %u\n";
static const char _line21[] = "  Profile disable : %u\n";
static const char _line22[] = "  ----------------------------------------\n";
static const char _line23[] = "  Mailbox %u\n";
static const char _line24[] = "  ----------------------------------------\n    State                     : %s\n";

/*
** ++
**
** This function dumps a SAS session
**
** --
*/

static void _dumpSession(uint32_t idx)
  {
  if(idx >= MAX_SESSIONS)
    {
    printf(session_error,idx);    
    return _usage();
    }
    
  SAS_Session s = &SasOcmMap->sessions[idx];
  int i;
  
  printf(_line11, idx,s);
  printf(_line15, s->sync);
  printf(_line16, s->axi);
  printf(_line17, s->ocm);
  printf(_line13, s->profile);
  printf(_line18, s->valid);
  printf(_line19, s->invalid);
  printf(_line20, s->wait);
  printf(_line21, s->disable);
  printf(_line14, s->srcs);
  printf(_line12, s->enabled);
  
  for(i=0; i<s->srcs; i++)  
    {
    printf(_line22,i);
    printf(_line23,i);  
    printf(_line24,((1<<i)&s->enabled) ? "ENABLED" : "DISABLED");  
    _dumpMailbox(&s->mbx[i]);    
    }
  }


/*
** ++
**
** This function dumps a socket SAS attributes
**
** --
*/

static const char _attr_hdr[]  = "%s socket %u attributes:\n";
static const char _attr_moh[]  = "  moh  : %u\n";
static const char _attr_obFd[] = "  obFd : %u\n";
static const char _attr_mib[]  = "  mib  : %u\n";
static const char _attr_ibFd[] = "  ibFd : %u\n";

static void _dumpAttrs(const char *name)
  {
  int idx = _lookupSocket(name);
  if(idx == -1) return;
  
  printf(_attr_hdr,name,idx);
  printf(_attr_moh, SasOcmMap->plugins[idx].attrs->moh);
  printf(_attr_obFd,SasOcmMap->plugins[idx].attrs->obFd);
  printf(_attr_mib, SasOcmMap->plugins[idx].attrs->mib);
  printf(_attr_ibFd,SasOcmMap->plugins[idx].attrs->ibFd);

  }


/*
** ++
**
** This function dumps a socket SAS preferences
**
** --
*/

static const char _pref_hdr[]    = "%s socket %u preferences:\n";
static const char _pref_name[]   = "  name            : %s\n";
static const char _pref_file[]   = "  bitfile         : %s\n";
static const char _pref_region[] = "  register region : 0x%x\n";
static const char _pref_offset[] = "  register offset : 0x%x\n";

static void _dumpPrefs(const char *name)
  {
  int idx = _lookupSocket(name);
  if(idx == -1) return;
  
  printf(_pref_hdr,name,idx);
  printf(_pref_name,  SasOcmMap->plugins[idx].name);
  printf(_pref_file,  SasOcmMap->plugins[idx].bitfile);
  printf(_pref_region,SasOcmMap->plugins[idx].region);
  printf(_pref_offset,SasOcmMap->plugins[idx].offset);
  
  }
  
/*
** ++
**
** This function dumps a socket SAS metrics
**
** --
*/

static const char _metric_hdr[]     = "%s socket %u metrics:\n";
static const char _metric_ibFull[]  = "  ibFull   : %u\n";   
static const char _metric_obEmpty[] = "  obEmpty  : %u\n";  
static const char _metric_obFull[]  = "  obFull   : %u\n";   

static void _dumpMetrics(const char *name)
  {
  int idx = _lookupSocket(name);
  if(idx == -1) return;
  
  SAS_Metrics *metrics = (SAS_Metrics*)&SasOcmMap->metricsTable[idx*METRICS_MAX_SOCKET];  
  
  printf(_metric_hdr,name,idx);
  printf(_metric_ibFull, metrics->ibFull);   
  printf(_metric_obEmpty,metrics->obEmpty);  
  printf(_metric_obFull, metrics->obFull);   
  
  }

/*
** ++
**
** This function dumps a socket SAS errors
**
** --
*/

static const char _error_hdr[]   = "%s socket %u error counters:\n";
static const char _error_obHdr[] = "  obHdr    : %u\n";
static const char _error_obPay[] = "  obPay    : %u\n";
static const char _error_ibHdr[] = "  ibFrame  : %u\n";
static const char _error_ibPay[] = "  ibPay    : %u\n";

static void _dumpErrors(const char *name)
  {
  int idx = _lookupSocket(name);
  if(idx == -1) return;
  
  SocketMgmt *mgmt = (SocketMgmt*)(SasAxi+SOCKET_AXI_BASE+(idx*SOCKET_AXI_SIZE));
  volatile SAS_Errors *errors = &mgmt->errors;
    
  printf(_error_hdr,name,idx);
  printf(_error_obHdr,errors->obHdr);
  printf(_error_obPay,errors->obPay);
  printf(_error_ibHdr,errors->ibHdr);
  printf(_error_ibPay,errors->ibPay);  
  
  }

/*
** ++
**
** This function dumps a socket SAS faults
**
** --
*/

static const char _fault_hdr[]      = "%s socket %u faults:\n";
static const char _fault_obRead[]   = "  obRead   : %u\n";
static const char _fault_obWrite[]  = "  obWrite  : %u\n";
static const char _fault_obFrame[]  = "  obFrame  : %u\n";
static const char _fault_obOvflow[] = "  obOvflow : %u\n";
static const char _fault_ibRead[]   = "  ibRead   : %u\n";
static const char _fault_ibWrite[]  = "  ibWrite  : %u\n";
static const char _fault_ibFrame[]  = "  ibFrame  : %u\n";
static const char _fault_ibOvflow[] = "  ibOvflow : %u\n";

static void _dumpFaults(const char *name)
  {
  int idx = _lookupSocket(name);
  if(idx == -1) return;
  
  SAS_Faults *faults = (SAS_Faults*)&SasOcmMap->faultTable[idx];  
  
  printf(_fault_hdr,name,idx);
  printf(_fault_obRead,  faults->obRead);
  printf(_fault_obWrite, faults->obWrite);
  printf(_fault_obFrame, faults->obFrame);
  printf(_fault_obOvflow,faults->obOvflow);
  printf(_fault_ibRead,  faults->ibRead);
  printf(_fault_ibWrite, faults->ibWrite);
  printf(_fault_ibFrame, faults->ibFrame);
  printf(_fault_ibOvflow,faults->ibOvflow);
  
  }

static const char _ctl_hdr0[]      = "SAS OCM usage :\n";
static const char _ctl_csize[]    = "  control bytes                  : %u\n";
static const char _ctl_ksize[]    = "  kvt bytes                      : %u\n";
static const char _ctl_dsize[]    = "  descriptor bytes               : %u\n";
static const char _ctl_ocmAlloc[] = "  total bytes used               : %u\n";
static const char _ctl_ocmRem[]   = "  bytes available                : %u\n";
static const char _ctl_hdr1[]     = "SAS control memory map :\n";
static const char _ctl_res[]      = "  reserved[8]                    : 0x%x\n";
static const char _ctl_metrics[]  = "  metricsTable[MAX_INTR_METRICS] : 0x%x\n";
static const char _ctl_faults[]   = "  faultTable[MAX_SOCKETS]        : 0x%x\n";
static const char _ctl_intrTbl[]  = "  intrTable                      : 0x%x\n";
static const char _ctl_intrMap[]  = "  intrMap[SAS_INTR_COUNT]        : 0x%x\n";
static const char _ctl_sessions[] = "  sessions[MAX_SESSIONS]         : 0x%x\n";
static const char _ctl_plugins[]  = "  plugins[MAX_SOCKETS]           : 0x%x\n";
static const char _ctl_appTbl[]   = "  appTable[MAX_APP_CHANNELS]     : 0x%x\n";
static const char _ctl_sockKvt[]  = "  socketKvt         (0x%x) : 0x%x\n";            
static const char _ctl_appKvt[]   = "  appKvt            (0x%x) : 0x%x\n";            
static const char _ctl_descrB[]   = "  descriptorBase    (0x%x) : 0x%x\n";
static const char _ctl_descrF[]   = "  descriptorFree    (0x%x) : 0x%x\n";

static void _dumpControl(void)
  {
  
  printf(_ctl_hdr0);
  printf(_ctl_csize    ,sizeof(OcmMap));
  printf(_ctl_ksize    ,SasOcmMap->descriptorBase-sizeof(OcmMap));
  printf(_ctl_dsize    ,SasOcmMap->descriptorFree-SasOcmMap->descriptorBase);
  printf(_ctl_ocmAlloc ,SasOcmMap->descriptorFree);
  printf(_ctl_ocmRem   ,mem_Region_size(MEM_REGION_OCM)-SasOcmMap->descriptorFree);
  printf(_ctl_hdr1);
  printf(_ctl_res     ,(unsigned)&SasOcmMap->reserved[0]);
  printf(_ctl_metrics ,(unsigned)&SasOcmMap->metricsTable[0]);
  printf(_ctl_faults  ,(unsigned)&SasOcmMap->faultTable[0]);
  printf(_ctl_intrTbl ,(unsigned)&SasOcmMap->intrTable);
  printf(_ctl_intrMap ,(unsigned)&SasOcmMap->intrMap[0]);
  printf(_ctl_sessions,(unsigned)&SasOcmMap->sessions[0]);
  printf(_ctl_plugins ,(unsigned)&SasOcmMap->plugins[0]);
  printf(_ctl_appTbl  ,(unsigned)&SasOcmMap->appTable[0]);
  printf(_ctl_sockKvt ,(unsigned)SasOcmMap+(unsigned)SasOcmMap->socketKvt,
                       (unsigned)&SasOcmMap->socketKvt);
  printf(_ctl_appKvt  ,(unsigned)SasOcmMap+(unsigned)SasOcmMap->appKvt,
                       (unsigned)&SasOcmMap->appKvt);
  printf(_ctl_descrB  ,(unsigned)SasOcmMap+(unsigned)SasOcmMap->descriptorBase,
                       (unsigned)&SasOcmMap->descriptorBase);
  printf(_ctl_descrF  ,(unsigned)SasOcmMap+(unsigned)SasOcmMap->descriptorFree,
                       (unsigned)&SasOcmMap->descriptorFree);
  
  }

static struct option opts[] = 
  {
    { (char*)"session",     required_argument, 0, 's' },
    { (char*)"attrs",       required_argument, 0, 'a' },
    { (char*)"prefs",       required_argument, 0, 'p' },
    { (char*)"errors",      required_argument, 0, 'r' },
    { (char*)"metrics",     required_argument, 0, 'm' },
    { (char*)"faults",      required_argument, 0, 'f' },
    { (char*)"control",     no_argument,       0, 'c' },
    { (char*)"enable",      required_argument, 0, 'e' },
    { (char*)"disable",     required_argument, 0, 'd' },
    { (char*)"help",        no_argument,       0, 'h' },
    { 0, 0, 0, 0 }
  };
  
/*
** ++
**
** This function executes the SAS shell command
**
** --
*/
  
void _main(int argc, char** argv)
  {
  int optcnt = 0;
  if(argc == 1) return _usage();
  
  int opt;
  if ((opt = getopt_long(argc, argv, "che:s:d:a:p:m:r:f:", opts, NULL)) == -1)
    return _usage();

  optcnt++;
  switch (opt)
    {
    case 's': _dumpSession   (strtol(optarg, 0, 0));   break;
    case 'a': _dumpAttrs            (optarg);          break;
    case 'p': _dumpPrefs            (optarg);          break;
    case 'r': _dumpErrors           (optarg);          break;
    case 'm': _dumpMetrics          (optarg);          break;
    case 'f': _dumpFaults           (optarg);          break;
    case 'c': _dumpControl          ();                break;
    case 'e': _profileSession(strtol(optarg, 0, 0),1); break;
    case 'd': _profileSession(strtol(optarg, 0, 0),0); break;
    case 'h':
    default:  return _usage();
    }
    
  if(!optcnt) return _usage();    
  
  optind = 0;
  }
  
/*
** ++
**
** This function registers SAS commands with the rtems shell.
**
** --
*/

void SAS_AddCommands(void)
  {
  rtems_shell_cmd_t * retCmd;
  // Call the long form for adding this command.  This
  // allocates the structure space inside RTEMS

  retCmd = rtems_shell_add_cmd("sas",
                               "ppi",                             
                               USAGE_SAS,                             
                               (rtems_shell_command_t)_main);

  rtems_shell_add_cmd_struct( retCmd );
  }
