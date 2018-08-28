/*
**  Daemon process for controlling Fulcrum switch
**
*/

//#include "bldInfo.h"

#include "Manager.hh"
//#include "MeshContainer.hh"
//#include "MeshBoundary.hh"
#include "logger/FormatMessage.hh"

#include "cm_svc/CmHw.hh"
#include "cm_svc/ChannelMap.hh"
#include "cm_svc/PlxBridge.hh"
#include "cm_svc/Print.hh"
#include "cm_svc/ClusterIpInfo.hh"
#include "cm_shell/ShellCommon.hh"
//#include "cm_net/Manager.hh"
#include "fm/FmTahoeMgr.hh"
#include "fm/FmTahoe.hh"
#include "fm/FmConfig.hh"
#include "fm/FmPort.hh"
#include "fm/FmLCIPacket.hh"
#include "fm/SwmRequest.hh"
#include "fm/SwmReply.hh"
#include "fm/SwmPortMibCounters.hh"
#include "fm/SwmEplRegs.hh"

#include <string>
#include <vector>
#include <sstream>

#include <getopt.h>

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdarg.h>

extern "C" {
#include "bsi/CMB_switch_cfg.h"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"
#include "map/MapBsi.h"
}

#define strcasestr strstr

static cm::net::Manager* _mgr = 0;
static FmConfig& _config = *new FmConfig;
static unsigned _zone2 = 0, _fp = ((1<<23) | (1<<24));
static int _dev = 0;
static cm::shell::AbsConsole* _console = 0;
static unsigned _bsi_poll_period = 10000; // microseconds
static bool lfifo=false;
static bool lverbose=true;
//static unsigned* beg = new unsigned[1512>>2];  // packet buffer

//  BSI Fabric Map bit shifts,masks
enum { s_slot=0, s_chan=4, s_port=8, s_stat=13 };
enum { m_slot=(1<<(s_chan-s_slot))-1 };
enum { m_chan=(1<<(s_port-s_chan))-1 };
enum { m_port=(1<<(s_stat-s_port))-1 };
enum { m_stat=(1<<(16-s_stat))-1 };
enum { ok_stat=0x4 };
enum { MAX_PORT=FmPort::MAX_PORT, ALL_PORTS=(2<<MAX_PORT)-1 };
enum { FABRIC_MAP_WORDS=8 };  // 32b words per slot
enum { FABRIC_MAP_SLOTS=16 };

class DtmEnv {
public:
  DtmEnv() : bsi (LookupBsi())
  {
    if (lverbose) {
      printf("Mapped BSI to 0x%x\n",(int)bsi);
    }
  }
  Bsi  bsi;
};

class DtmLog : public cm::shell::AbsConsole {
public:
  DtmLog(unsigned sz) :
    _line  (new char[256]),
    _buffer(new char[sz]),
    _end   (_buffer+sz),
    _next  (_buffer),
    _last  (0)
  {
    memset(_buffer,0,sz);
  }
  ~DtmLog()
  {
    delete[] _buffer;
    delete[] _line;
  }
public:
  void printv(const char* msgformat, ...) 
  {
    uint32_t len;
    FORMAT_MESSAGE(msgformat, 256, _line, len);

    struct timespec tv;
    clock_gettime(CLOCK_REALTIME,&tv);

    char* p = _next;
    p += snprintf(p,_end-p, "[%u.%09u]%s",
                  unsigned(tv.tv_sec), 
                  unsigned(tv.tv_nsec), _line) + 1;
    if (p >= _end) {
      _last  = _next;
      p = _buffer + sprintf(_buffer, "[%u.%09u]%s", unsigned(tv.tv_sec), unsigned(tv.tv_nsec), _line) + 1;
    }
    *(_next = p) = 0;
  }
  void reserve(unsigned) {}
public:
  void dump(cm::shell::AbsConsole* c) 
  {
    char* p = _last ? _next+1 : _buffer;
    while(*p) {
      c->printv(p);
      p += strlen(p)+1;
      if (_last && p>=_last)
        p = _buffer;
    }    
  }
private:
  char* _line;
  char* _buffer;
  const char* _end;
  char* _next;
  const char* _last;
};

static DtmLog* _dtmlog = new DtmLog(0x1000);
cm::shell::AbsConsole* _log = _dtmlog;

static void load_bsi   (DtmEnv*);
static void load_config(const char*);
static void save_config(const char*,const FmConfig&);

static char* buff = 0;

static const size_t max_size = 0x100000;

void load_config(const char* fname)
{
  FILE* f = fopen(fname,"r");
  if (f) {
    size_t nb = fread(buff, 1, max_size, f);
    syslog(LOG_NOTICE,"Read %d bytes from configuration .current\n",nb);
    const char* TRAILER = "</Document>";
    strcpy(buff+nb,TRAILER);
    const char* p = buff;
    _config.load(p);
    fclose(f);
  }
}

void save_config(const char* fname, const FmConfig& c)
{
  FILE* f = fopen(fname,"w");
  if (f) {
    char* p = buff;
    c.save(p);
    size_t nb = fwrite(buff, 1, p-buff, f);
    syslog(LOG_NOTICE,"Wrote %d bytes to configuration %s\n",nb,fname);
    fclose(f);
  }
}

//
//  Shell commands
//
typedef struct {
  const char** args;
  const char** lines;
} DtmHelp_t;

static const char* fm_lines[] = { "reset",
                                  "load ...",
                                  "save ...",
                                  "set ...",
                                  "show ...",
                                  "enable ...",
                                  "disable ...",
                                  "lanetest ...",
                                  NULL };
static const char* reset_args[] = { "reset",NULL };
static const char* reset_lines[] = {"reset\t(reset switch and apply configuration)",
                 NULL };

static const char* load_args[] = { "load",NULL };
static const char* load_lines[]= { "load <filename>\t(load configuration from <filename>)",
                                   NULL };

static const char* save_args[] = { "save",NULL };
static const char* save_lines[]= { "save <filename>\t(save configuration to <filename>)",
                                   NULL };

static const char* set_args [] = { "set",NULL };
static const char* set_lines[] = { "set base \t(set base switch as target)",
                                   "set fabric \t(set fabric switch as target)",
                                   "set register\t(set fulcrum register)",
                                   NULL };
static const char* set_reg_args [] = { "set","register",NULL };
static const char* set_reg_lines[] = { "set register <addr> <value> [<repeat>]", NULL };
static const char* show_args [] = { "show",NULL };
static const char* show_lines[] = { "show int \t(show interface status)",
                                    "show stat\t(show interface statistics)",
                                    "show macaddr\t(show mac address table)",
                                    "show vlans\t(show vlan table)",
                                    "show triggers\t(show filtering/monitoring triggers)",
                                    "show trunks\t(show link aggregate groups)",
                                    "show reg\t(show register value)",
                                    "show plx\t(show pcie bridge status)",
                                    "show log\t(show log entries)",
                                    NULL };
static const char* show_int_args [] = { "show","int",NULL} ;
static const char* show_int_lines[] = { "show int brief \t(show status summary)",
                                        "show int <port>\t(show status for <port> [e.g. dtm0.0]",
                                        "show int all\t(show status for all ports)",
                                        NULL };
static const char* show_int_brief_args [] = { "show","int","brief",NULL };
static const char* show_int_brief_lines[] = { show_int_lines[0],
                                              NULL };
static const char* show_int_dpm_args [] = { "show","int","dpm",NULL };
static const char* show_int_dpm_lines[] = { show_int_lines[1],
                                            NULL };
static const char* show_stat_args [] = { "show","statistics",NULL };
static const char* show_stat_lines[] = { "show statistics brief \t(show statistics summary)",
                                         "show statistics global\t(show global counters)",
                                         "show statistics <port>\t(show detailed statistics for port)",
                                         NULL };
static const char* show_stat_brief_args [] = { "show","statistics","brief",NULL };
static const char* show_stat_brief_lines[] = { show_stat_lines[0],
                                               NULL };
static const char* show_stat_global_args[]  = { "show","statistics","global",NULL };
static const char* show_stat_global_lines[] = { show_stat_lines[1],
                                             NULL };
static const char* show_stat_dpm_args [] = { "show","statistics","dpm",NULL };
static const char* show_stat_dpm_lines[] = { show_stat_lines[2],
                                             NULL };
static const char* show_macaddr_args [] = { "show","macaddr",NULL };
static const char* show_macaddr_lines[] = { show_lines[2],
                                            NULL };
static const char* show_vlans_args [] = { "show","vlans",NULL };
static const char* show_vlans_lines[] = { show_lines[3],
                                            NULL };
static const char* show_triggers_args [] = { "show","triggers",NULL };
static const char* show_triggers_lines[] = { show_lines[4],
                                            NULL };
static const char* show_trunks_args [] = { "show","trunks",NULL };
static const char* show_trunks_lines[] = { show_lines[5],
                                            NULL };
static const char* show_reg_args [] = { "show","register",NULL };
static const char* show_reg_lines[] = { "show register <reg>\t(show register)",
                                        NULL };
static const char* show_plx_args [] = { "show","plx",NULL };
static const char* show_plx_lines[] = { "show plx\t(show pcie bridge)",
                                        NULL };
static const char* show_log_args [] = { "show","log",NULL };
static const char* show_log_lines[] = { "show log\t(show log entries)",
                                        NULL };

static const char* enable_args [] = { "enable", NULL };
static const char* enable_lines[] = { "enable <port>\t(enable port at 10Gb/s)",
                                      "enable <port> 1Gb/s\t(enable port at 1Gb/s)",
                                      "enable <port> 10Gb/s <drive>\t(enable port at 10Gb/s with drive)",
                                      NULL };
static const char* disable_args [] = { "disable", NULL };
static const char* disable_lines[] = { "disable <port>\t(disable port)",
                                      NULL };

static const char* lanetest_args [] = { "lanetest", NULL };
static const char* lanetest_lines[] = { "lanetest <port> start <mode>\t(start test <mode> on port)",
                                        "lanetest <port> status\t(show error count on port)",
                                        "lanetest <port> stop  \t(stop test on port)",
                                        NULL };

static DtmHelp_t dtm_help[] = { { reset_args, reset_lines },
                                { load_args, load_lines },
                                { save_args, save_lines },
                                { set_args, set_lines },
                                { set_reg_args, set_reg_lines },
                                { show_args, show_lines },
                                { show_int_args, show_int_lines },
                                { show_int_brief_args, show_int_brief_lines },
                                { show_int_dpm_args, show_int_dpm_lines },
                                { show_stat_args, show_stat_lines },
                                { show_stat_brief_args, show_stat_brief_lines },
                                { show_stat_global_args, show_stat_global_lines },
                                //{ show_stat_dpm_args, show_stat_dpm_lines },
                                { show_macaddr_args, show_macaddr_lines },
                                { show_vlans_args, show_vlans_lines },
                                { show_triggers_args, show_triggers_lines },
                                { show_trunks_args, show_trunks_lines },
                                { show_reg_args, show_reg_lines },
                                { show_plx_args, show_plx_lines },
                                { show_log_args, show_log_lines },
                                { enable_args, enable_lines },
                                { disable_args, disable_lines },
                                { lanetest_args, lanetest_lines },
                                { NULL, NULL } };

typedef struct {
  void (*cmd)(int, char**);
  const char** args;
} DtmCommand_t;

class DtmInterface_t {
public:
  std::string name;
  unsigned    port;
};

static DtmInterface_t __interfaces[] = 
  { { "slot2" , 13 },
    { "slot3" , 14 },
    { "slot4" , 15 },
    { "slot5" , 16 },
    { "slot6" , 17 },
    { "slot7" , 18 },
    { "slot8" , 19 },
    { "slot9" , 20 },
    { "slot10", 21 },
    { "slot11", 22 },
    { "slot12", 10 },
    { "slot13", 11 },
    { "slot14", 12 },
    { "", 0 },
    { "", 0 },
    { "", 0 },
    { "dpm0.0", 2  },
    { "dpm0.1", 3  },
    { "dpm1.0", 4  },
    { "dpm1.1", 5  },
    { "dpm2.0", 6  },
    { "dpm2.1", 7  },
    { "dpm3.0", 8  },
    { "dpm3.1", 9  },
    { "", 0 },
    { "", 0 },
    { "", 0 },
    { "", 0 },
    { "fp_sfp", 23 },
    { "fp_sfp+",24 },
    { "", 0 },
    { "dtm"   , 1  },
    { "mgmt"  , 0  }};

static DtmInterface_t _interfaces(unsigned i)
{
  return __interfaces[i];
}

#if 0
static int _port(const char* name)
{
  for(int i=0; i<32; i++) {
    DtmInterface_t intf = _interfaces(i);
    if (intf.name.empty()) continue;
    if (strcasecmp(name,intf.name.c_str())==0)
      return intf.port;
  }
  return -1;
}
#endif

static std::string _name(unsigned port)
{
  for(int i=0; i<33; i++) {
    DtmInterface_t intf = _interfaces(i);
    if (intf.name.empty()) continue;
    if (intf.port==port)
      return intf.name;
  }
  return std::string();
}

static const char* _speed[] = { "None",
                                "10Mb/s",
                                "100Mb/s",
                                "1Gb/s",
                                "2.5Gb/s",
                                "4Gb/s",
                                "10Gb/s" };

static unsigned _lanes(unsigned q)
{
  return
    ((q>>0)&1) |
    ((q>>2)&2) |
    ((q>>4)&4) |
    ((q>>6)&8) ;
}

static const char* line_sep = "---------------------";

static bool _enabled(unsigned port)
{
  return _mgr->deviceMgr(_dev).portSpeed(FmPort(port))!=cm::fm::None;
}

static void _reset(int argc, char** argv)
{
  _mgr->deviceMgr(_dev).reset(_config);
}

static void _load(int argc, char** argv)
{
  load_config(argv[0]);
  _mgr->deviceMgr(_dev).configure(_config);
}

static void _save(int argc, char** argv)
{
  save_config(argv[0],_mgr->deviceMgr(_dev)._config);
}

static void _set(int argc, char** argv)
{
  static const char* titles[] = { "base", "fabric", NULL };
  if (argc>1)
    _console->printv("set <switch> : too many arguments (%d)\n",argc);
  else {
    for(unsigned i=0; titles[i]!=NULL; i++)
      if (strcasecmp(argv[0],titles[i])==0) {
        if (i<_mgr->nDevices())
          _dev = i;
        return;
      }
    _console->printv("set %s - failed\n", argv[0]);
    _console->printv("Valid options are:");
    for(unsigned i=0; titles[i]!=NULL; i++)
      _console->printv(" %s",titles[i]);
    _console->printv("\n");
  }
}

static void _set_reg(int argc, char** argv)
{
  if (argc<2)
    _console->printv("set register <addr> <value> [<repeat>] : incorrect # args (%d)\n",argc);
  else {
    unsigned off = strtoul(argv[0],NULL,0);
    unsigned v   = strtoul(argv[1],NULL,0);
    unsigned n   = (argc>2) ? strtoul(argv[2],NULL,0) : 0;
    cm::svc::PlxBridge& bridge = cm::svc::PlxBridge::instance();
    bridge.set_verbose(1);
    uint32_t* addr = 0;
    for(unsigned i=0; i<=n; i++)
      bridge.write(addr+off,v);
    bridge.set_verbose(0);
  }
}
static void _show_int_brief(int argc, char**)
{
  static const char* titles[] = { "interface",
                                  "enabled",
                                  "status",
                                  "iVLAN",
                                  NULL };
  if (argc>0)
    _console->printv("show int brief : too many arguments (%d)\n",argc);
  else {
    const FmPortMask up_ports = _mgr->deviceMgr(_dev).portsUp();

    for(unsigned i=0; titles[i]!=NULL; i++)
      _console->printv("%s%c",titles[i],titles[i+1] ? '\t':'\n');
    for(unsigned i=0; titles[i]!=NULL; i++)
      _console->printv("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
    for(unsigned i=0; i<32; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      _console->printv("%-*s\t", strlen(titles[0]), intf.name.c_str());
      _console->printv("%*s\t" , strlen(titles[1]), (_enabled(intf.port)) ? "T":"F");
      _console->printv("%*s\t" , strlen(titles[2]), (up_ports.contains(FmPort(intf.port))) ? "UP":"DOWN");
      _console->printv("%*u\n" , strlen(titles[3]), _mgr->deviceMgr(_dev).dev().port_cfg_1[intf.port]&0xfff);
    }
  }
}

static void _show_int(int argc, char** argv)
{
  if (argc>1)
    _console->printv("show int <port> : too many arguments (%d)\n",argc);
  else {
    for(int i=0; i<32; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      if (strcasecmp(argv[0],intf.name.c_str())==0 ||
          strcasecmp(argv[0],"all")==0) {
        _console->printv("interface %s [%d]\n",intf.name.c_str(),intf.port);

        char* reply = new char[0x80000];
        unsigned reply_len;
        SwmEplReadRequest* req = new SwmEplReadRequest( &_mgr->deviceMgr(_dev).dev().epl_phy[intf.port-1] );
        _mgr->process(*req, (void*)reply, reply_len);
        delete req;

        if (!reply_len) {
          _console->printv("Error fetching port status\n");
        }
        else {
          cm::fm::PortSpeed speed = _mgr->deviceMgr(_dev).portSpeed(FmPort(intf.port));
          if (speed == cm::fm::None) {
            _console->printv("\tport is disabled\n");
          }
          else {
            _console->printv("\tport is enabled at %s\n",_speed[speed]);

            unsigned lanemask = 0;
            switch(speed) {
            case cm::fm::None : lanemask=0x0; break;
            case cm::fm::X4Gb :
            case cm::fm::X10Gb: lanemask=0xf; break;
            default: 
              { const FmPortConfig& pc = _mgr->deviceMgr(_dev)._config.portCfg;
                lanemask = (pc.portsRxFlip.contains(FmPort(intf.port))) ? 0x8 : 0x1; }
              break;
            }
            const SwmEplReadReply* r = reinterpret_cast<const SwmEplReadReply*>(reply);
            unsigned v = r->result().serdes_ip & 0xfff;
            if (v&0x924)
              _console->printv("\t%40.40s%x\n","disparity error  lanes ",_lanes(v>>2)&lanemask);
            if (v&0x492)
              _console->printv("\t%40.40s%x\n","out-of-band char lanes ",_lanes(v>>1)&lanemask);
            if (v&0x249)
              _console->printv("\t%40.40s%x\n","loss of signal   lanes ",_lanes(v>>0)&lanemask);

            unsigned errs = v>>12;
            if (errs)
              _console->printv("\t%40.40s%d\n","total errors",errs);

            _console->printv("\tpcs status %x\n", r->result().pcs_ip);
            _console->printv("\tmac status %x\n", r->result().mac_ip);
          }
        }
        delete[] reply;

        FmTahoe& d = _mgr->deviceMgr(_dev).dev();
        unsigned v = d.port_cfg_2[intf.port];
        _console->printv("\tfwd mask   %08x\n",v);
        v = d.trunk_port_map[intf.port];
        if (v&(1<<4))
          _console->printv("\tagg-group  %02d\n",v&0xf);
        

        return;
      }
    }
    _console->printv("show int %s : interface not found\n",argv[0]);
  }
}

static void _show_stat_brief(int argc, char**)
{
  static const char* titles[] = { "interface",
                                  "in packets",
                                  "out packets",
                                  "in errors",
                                  "out errors",
                                  NULL };
  if (argc>0)
    _console->printv("show stat brief : too many arguments (%d)\n",argc);
  else {

    char* reply = new char[0x80000];
    unsigned reply_len;
    SwmAllPortMibRequest* req = new SwmAllPortMibRequest( &_mgr->deviceMgr(_dev).dev() );
    _mgr->process(*req, (void*)reply, reply_len);
    delete req;
    if (!reply_len) {
      _console->printv("Error fetching port statistics\n");
      delete[] reply;
      return;
    }

    const SwmAllPortMibReply& rrep = *(const SwmAllPortMibReply*)reply;

    for(unsigned i=0; titles[i]!=NULL; i++)
      _console->printv("%s%c",titles[i],titles[i+1] ? '\t':'\n');
    for(unsigned i=0; titles[i]!=NULL; i++)
      _console->printv("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
    for(unsigned i=0; i<33; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      const SwmPortMibCounters& c = rrep.result().port(intf.port);
      _console->printv("%-*s\t", strlen(titles[0]), intf.name.c_str());
      unsigned long long rxPkts = c.rxUcast + c.rxBcast + c.rxMcast;
      _console->printv("%*lld\t" , strlen(titles[1]), rxPkts);
      unsigned long long txPkts = c.txUnicast + c.txBroadcast + c.txMulticast;
      _console->printv("%*lld\t" , strlen(titles[2]), txPkts);
      unsigned long long rxErrs = c.rxFcsErrors + c.rxSymbolErrors;
      _console->printv("%*lld\t" , strlen(titles[3]), rxErrs);
      unsigned long long txErrs = c.txTimeoutDrops + c.txErrorDrops;
      _console->printv("%*lld\n" , strlen(titles[4]), txErrs);
    }
    delete[] reply;
  }
}

static char _buff[64];

#define printStat1( var1 ) {                                            \
    _console->printv("%30.30s = %15lld\n", # var1, (unsigned long long) c.var1);  \
  }

#define printStat( var1, var2 ) {                                       \
    _console->printv("%30.30s = %15lld"  , # var1, (unsigned long long) c.var1);  \
    _console->printv("%30.30s = %15lld\n", # var2, (unsigned long long) c.var2);  \
  }

#define printStats1( var1, len ) {                                      \
    for(unsigned j=0; j<len; j++) {                                     \
      sprintf(_buff,"%s[%d]",# var1,j);                                 \
      _console->printv("%30.30s = %15lld\n", _buff, (unsigned long long)c.var1[j]); \
    }                                                                   \
  }

#define printStats( var1, var2, len ) {                                 \
    for(unsigned j=0; j<len; j++) {                                     \
      sprintf(_buff,"%s[%d]",# var1,j);                                 \
      _console->printv("%30.30s = %15lld  ", _buff, (unsigned long long)c.var1[j]); \
      sprintf(_buff,"%s[%d]",# var2,j);                                 \
      _console->printv("%30.30s = %15lld\n", _buff, (unsigned long long)c.var2[j]); \
    }                                                                   \
  }

#define printStatsQ( var1, len  ) {                                     \
    for(unsigned j=0; j<len;) {                                         \
      sprintf(_buff,"%s[%d:%d]",# var1,j,(j+3)<len ? j+3:len-1);        \
      _console->printv("%30.30s = ", _buff);                            \
      for(unsigned k=0; k<4&&j<len; k++,j++)                            \
        _console->printv("%15lld  ", (unsigned long long)c.var1[j]);    \
      _console->printv("\n");                                           \
    } }

static void _show_stat_global(int argc, char**)
{
  if (argc>0)
    _console->printv("show stat global : too many arguments (%d)\n",argc);
  else {
    char* reply = new char[0x80000];
    unsigned reply_len;
    SwmGlobalMibRequest* req = new SwmGlobalMibRequest( &_mgr->deviceMgr(_dev).dev() );
    _mgr->process(*req, (void*)reply, reply_len);
    delete req;
    if (!reply_len) {
      _console->printv("Error fetching global statistics\n");
      delete[] reply;
      return;
    }
    const SwmGlobalMibReply* r = reinterpret_cast<const SwmGlobalMibReply*>(reply);
    const SwmGlobalMibCounters& c = r->result();

    printStat1(cmGlobalLowDrops);
    printStat1(cmGlobalHighDrops);
    printStat1(cmGlobalPrivilegeDrops);
    printStatsQ(cmTxDrops,25);
    printStatsQ(trigCount,17);
    printStatsQ(vlanUnicast,32);
    printStatsQ(vlanXcast,32);
    printStatsQ(vlanUnicastOctets,32);
    printStatsQ(vlanXcastOctets,32);

    delete[] reply;
  }
}

static void _show_stat(int argc, char** argv)
{
  if (argc>1)
    _console->printv("show stat <port> : too many arguments (%d)\n",argc);
  else {
    for(int i=0; i<33; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      if (strcasecmp(argv[0],intf.name.c_str())==0) {
        _console->printv("interface %s\n", argv[0]);

        char* reply = new char[0x80000];
        unsigned reply_len;
        SwmPortMibRequest* req = new SwmPortMibRequest( &_mgr->deviceMgr(_dev).dev().port_mib[intf.port] );
        _mgr->process(*req, (void*)reply, reply_len);
        delete req;

        if (!reply_len) {
          _console->printv("Error fetching port statistics\n");
        }
        else {
          const SwmPortMibReply* r = reinterpret_cast<const SwmPortMibReply*>(reply);
          const SwmPortMibCounters& c = r->result();
          printStat(rxUcast, txUnicast);
          printStat(rxBcast, txBroadcast);
          printStat(rxMcast, txMulticast);
          printStat1(rxPause);
          printStat(rxFcsErrors   , txTimeoutDrops);
          printStat(rxSymbolErrors, txErrorDrops);
          printStats(rxPri, rxOctetsPri, 8);
          printStat(rxMinto63   , txMinto63);
          printStat(rx64        , tx64);
          printStat(rx65to127   , tx65to127);
          printStat(rx128to255  , tx128to255);
          printStat(rx256to511  , tx256to511);
          printStat(rx512to1023 , tx512to1023);
          printStat(rx1024to1522, tx1024to1522);
          printStat(rx1523to2047, tx1523to2047);
          printStat(rx2048to4095, tx2048to4095);
          printStat(rx4096to8191, tx4096to8191);
          printStat(rx8192to10239,tx8192to10239);
          printStat(rx10240toMax, tx10240toMax);
          printStat1(rxUndersized);
          printStat1(rxOversized);
          printStat1(rxFragments);
          printStat1(rxGoodOctets);
          printStat1(rxBadOctets);
          _console->printv("\n");
          printStat(fidForwarded, floodForwarded);
          printStat(stpDrops    , reservedTraps);
          printStat(securityViolationDrops, vlanTagDrops);
          printStat(vlanIngressBVDrops, vlanEgressBVDrops);
          printStat(triggerRedirAndDrops, triggerMirrored);
          printStat(dlfDrops, broadcastDrops);
          printStat1(cmrxDrops);
        }
        delete[] reply;
        return;
      }
    }
    _console->printv("show stat %s : interface not found\n",argv[0]);
  }
}

static void _show_macaddr(int argc, char** argv)
{
  if (argc>0)
    _console->printv("show macaddr: too many arguments (%d)\n",argc);
  else {
    char* reply = new char[sizeof(SwmMacTableReadReply)];
    unsigned reply_len;
    SwmMacTableDirectRequest* req = new SwmMacTableDirectRequest( &_mgr->deviceMgr(_dev).dev() );
    _mgr->process(*req, (void*)reply, reply_len);
    delete req;
    
    if (!reply_len) {
      _console->printv("Error fetching mac address table\n");
    }
    else {
      unsigned nentries=0;
      unsigned nstatic=0;
      const SwmMacTableReadReply& r = *reinterpret_cast<const SwmMacTableReadReply*>(reply);
      const SwmMacTable& c = r.result();
      _console->printv("%8.8s%20.20s%10.10s%10.10s%6.6s%6.6s%4.4s\n",
             "Entry", "MAC Address", "Ports", "Lock", "Fid", "Trig", "P");
      for(unsigned j=0; j<SwmMacTable::Entries; j++) {
        const SwmMacTableEntry& e = c.entry[j];
        if (e.valid()) {
          nentries++;
          _console->printv("%8.8d",j);
          char buf[32];
          sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",
                  e.addr_octet(5),
                  e.addr_octet(4),
                  e.addr_octet(3),
                  e.addr_octet(2),
                  e.addr_octet(1),
                  e.addr_octet(0));
          _console->printv("%20.20s",buf);
          sprintf(buf,"%08x",unsigned(e.port_mask()));
          _console->printv("%10.10s",buf);
          if (e.locked()) {
            nstatic++;
            _console->printv("%10.10s","Static ");
          }
          else
            _console->printv("%10.10s","Dynamic ");
          _console->printv("%6.6x %6.6x %4.4x",
                 e.learning_group(), e.trigger(), e.parity());
          _console->printv("\n");
        }
      }
      _console->printv("Found %d entries, %d static\n", nentries, nstatic);
    }
  }
}

static void _show_vlans(int argc, char** argv)
{
  if (argc>0)
    _console->printv("show vlans: too many arguments (%d)\n",argc);
  else {
    _console->printv("%5.5s%8.8s%8.8s%5.5s%5.5s\n",
                     "VID", "Members", "Tagged", "Trig", "Cnt");
    char* reply = new char[sizeof(SwmVlanEntryReadReply)];
    unsigned reply_len;
    for(unsigned id=0; id<0x1000; id++) {
      SwmVlanEntryReadRequest* req = new SwmVlanEntryReadRequest( &_mgr->deviceMgr(_dev).dev().vid_table[id] );
      _mgr->process(*req, (void*)reply, reply_len);
      delete req;
    
      if (!reply_len) {
        _console->printv("Error fetching vlan entry\n");
      }
      else {
        const SwmVlanEntryReadReply& r = *reinterpret_cast<const SwmVlanEntryReadReply*>(reply);
        const SwmVlanTableEntry& c = r.result();
        if (!c.empty()) {
          _console->printv("%5.5d %07x %07x %4.4d %4.4d\n",
                           id,
                           unsigned(c.members()),
                           unsigned(c.tagged()),
                           c.trigger(),
                           c.counter());
        }
      }
    }
  }
}

static void _show_triggers(int argc, char** argv)
{
  if (argc>0)
    _console->printv("show triggers: too many arguments (%d)\n",argc);
  else {
    char* reply = new char[sizeof(SwmAllTriggersReply)];
    unsigned reply_len;
    SwmAllTriggersRequest* req = new SwmAllTriggersRequest( &_mgr->deviceMgr(_dev).dev() );
    _mgr->process(*req, (void*)reply, reply_len);
    delete req;
    
    if (!reply_len) {
      _console->printv("Error fetching trigger set\n");
    }
    else {
      const SwmAllTriggersReply& r = *reinterpret_cast<const SwmAllTriggersReply*>(reply);

      for(unsigned i=0; SwmTrigger::header(i); i++) {
        _console->printv("%10.10s", SwmTrigger::header(i));
        for(unsigned j=0; j<16; j++)
          if (r.result().trigger(j).enabled())
          _console->printv("%8.8s", r.result().trigger(j).field(i));
        _console->printv("\n");
      }
    }
  }
}

static void _show_trunks(int argc, char** argv)
{
  if (argc>0)
    _console->printv("show trunks: too many arguments (%d)\n",argc);
  else {
    _console->printv("port(trunk):");
    for(unsigned i=1; i<=MAX_PORT; i++) {
      unsigned v = _mgr->deviceMgr(_dev).dev().trunk_port_map[i];
      if (v & (1<<4)) {
        std::string name = _name(i);
        if (name.length()==0)
          _console->printv(" %02d(%02d)",i,v&0xf);
        else
          _console->printv(" %s(%02d)",name.c_str(),v&0xf);
      }
    }
    _console->printv("\n");

    for(unsigned i=0; i<12; i++) {
      unsigned n = _mgr->deviceMgr(_dev).dev().trunk_group_3[i]&0x1f;
      if (n) {
        _console->printv("trunk %02d:",i);
        std::vector<unsigned> ports;
        unsigned v = _mgr->deviceMgr(_dev).dev().trunk_group_1[i];
        ports.push_back((v>> 0)&0x1f);
        ports.push_back((v>> 5)&0x1f);
        ports.push_back((v>>10)&0x1f);
        ports.push_back((v>>15)&0x1f);
        ports.push_back((v>>20)&0x1f);
        ports.push_back((v>>25)&0x1f);
        v = _mgr->deviceMgr(_dev).dev().trunk_group_2[i];
        ports.push_back((v>> 0)&0x1f);
        ports.push_back((v>> 5)&0x1f);
        ports.push_back((v>>10)&0x1f);
        ports.push_back((v>>15)&0x1f);
        ports.push_back((v>>20)&0x1f);
        ports.push_back((v>>25)&0x1f);
        for(unsigned j=0; j<n; j++) {
          std::string name = _name(ports[j]);
          if (name.length()==0)
            _console->printv(" %02d",ports[j]);
          else
            _console->printv(" %s",name.c_str());
        }
        _console->printv("\n");
      }
    }
  }
}

static void _show_reg(int argc, char** argv)
{
  if (argc!=1) {
    _console->printv("show reg <reg>\n");
  }
  else {
    unsigned off = strtoul(argv[0],NULL,0);
    const uint32_t* addr = 0;
    cm::svc::PlxBridge& bridge = cm::svc::PlxBridge::instance();
    bridge.set_verbose(1);
    unsigned v = bridge.read(addr+off);
    bridge.set_verbose(0);
    _console->printv("reg[0x%x] = %x\n",off,v);
  }
}
static void _show_plx(int argc, char** argv)
{
  if (lverbose) {
    printf(" *** entered %s:%s\n", __FILE__, __FUNCTION__);
  }
  cm::svc::PlxBridge& b = cm::svc::PlxBridge::instance();
  b.dump();
}
static void _show_log(int argc, char** argv)
{
  _dtmlog->dump(_console);
}

static void _enable_port(int argc, char** argv)
{
  if (argc>3)
    _console->printv("enable <port> (<speed>): too many arguments (%d)\n",argc);
  else {
    unsigned drive=0x84;
    if (argc>=3)
      drive = strtoul(argv[2],NULL,0);

    cm::fm::PortSpeed speed = cm::fm::None;
    if (argc>=2) {
      for(int i=cm::fm::X10Mb; i<=cm::fm::X10Gb; i++)
        if (strcmp(argv[1],_speed[i])==0) {
          speed = cm::fm::PortSpeed(i);
          break;
        }
      if (speed == cm::fm::None) {
        _console->printv("enable %s %s - error parsing speed %s\n",
               argv[0],argv[1],argv[1]);
        return;
      }
    }
    else
      speed = cm::fm::X10Gb;

    for(int i=0; i<32; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      if (strcasecmp(argv[0],intf.name.c_str())==0) {
        //  bring up port & wait
        _mgr->deviceMgr(_dev).configurePort(FmPort(intf.port), speed, 0, 0, drive);
        //  clear counters
        char* reply = new char[0x80000];
        unsigned reply_len;
        SwmEplReadRequest* req = new SwmEplReadRequest( &_mgr->deviceMgr(_dev).dev().epl_phy[intf.port-1] );
        _mgr->process(*req, (void*)reply, reply_len);
        delete req;
        delete[] reply;
        return;
      }
    }
    _console->printv("enable %s %s: interface %s not found\n",
           argv[0], argc>1 ? argv[1]:"", argv[0]);
  }
}

static void _disable_port(int argc, char** argv)
{
  if (argc>1)
    _console->printv("disable <port> : too many arguments (%d)\n",argc);
  else {
    for(int i=0; i<32; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      if (strcasecmp(argv[0],intf.name.c_str())==0) {
        //  bring down port
        _mgr->deviceMgr(_dev).configurePort(FmPort(intf.port), cm::fm::None);
        return;
      }
    }
    _console->printv("disable %s : interface %s not found\n",argv[0],argv[0]);
  }
}

static void _lanetest(int argc, char** argv)
{
  if (argc<2)
    _console->printv("lanetest: incorrect # args (%d)\n",argc);
  else {
    for(int i=0; i<32; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      if (strcasecmp(argv[0],intf.name.c_str())==0) {
        //  bring down port
        //        _mgr->deviceMgr(_dev).configurePort(FmPort(intf.port), cm::fm::None);

        FmEplRegs& epl = _mgr->deviceMgr(_dev).dev().epl_phy[intf.port-1];

        if (strcmp(argv[1],"start")==0) {
          if (argc<3)
            _console->printv("lanetest start: incorrect # args (%d)\n",argc);
          else {
            unsigned itest = strtoul(argv[2],NULL,0);
            unsigned test_mode = 0x60 | (itest&0x8);
            
            epl.serdes_test_mode = test_mode;
            usleep(20);

            // check for sigdet
            unsigned status = epl.serdes_status;
            if ((status&0xf) != 0xf)
              _console->printv("Port unable to attain sigdet (0x%x)\n", status);

            test_mode &= ~0x40; // disable PCS framer
            epl.serdes_test_mode = test_mode;
            test_mode |= itest&0xf;  // enable BIST transmitter
            epl.serdes_test_mode = test_mode;
            test_mode  = itest&0xf;  // enable BIST checker
            epl.serdes_test_mode = test_mode;
          }
        }
        else if (strcmp(argv[1],"status")==0) {
          unsigned v = epl.serdes_bist_err_cnt; // clear BIST errcnt
          _console->printv("errcnt = 0x%x\n",v);
        }
        else if (strcmp(argv[1],"stop")==0) {
          epl.serdes_test_mode = 0x60;
        }
        return;
      }
    }
    _console->printv("lanetest %s : interface %s not found\n",argv[0],argv[0]);
  }
}

static DtmCommand_t dtm_command[] = { { _reset          ,  reset_args },
                                      { _load           ,  load_args },
                                      { _save           ,  save_args },
                                      { _set_reg        ,  set_reg_args },
                                      { _set            ,  set_args },
                                      { _show_int_brief ,  show_int_brief_args },
                                      { _show_int       ,  show_int_args },
                                      { _show_stat_brief,  show_stat_brief_args },
                                      { _show_stat_global, show_stat_global_args },
                                      { _show_stat      ,  show_stat_args },
                                      { _show_macaddr   ,  show_macaddr_args },
                                      { _show_vlans     ,  show_vlans_args },
                                      { _show_triggers  ,  show_triggers_args },
                                      { _show_trunks    ,  show_trunks_args },
                                      { _show_reg       ,  show_reg_args },
                                      { _show_plx       ,  show_plx_args },
                                      { _show_log       ,  show_log_args },
                                      { _enable_port    ,  enable_args },
                                      { _disable_port   ,  disable_args },
                                      { _lanetest       ,  lanetest_args },
                                      { NULL, NULL } };

static void parse_help(int argc, char** argv)
{
  if (lverbose) {
    printf("parse_help parsing ");
    for(int i=0; i<argc; i++)
      printf("[%s] ",argv[i]);
    printf("\n");
  }

  if (argc==0) {
    for(int iline=0; fm_lines[iline]!=NULL; iline++)
      _console->printv("%s\n",fm_lines[iline]);
    return;
  }
  for(int icmd=0; dtm_help[icmd].args!=NULL; icmd++) {
    for(int iarg=0; iarg<argc; ) {
      const char* arg = dtm_help[icmd].args[iarg];
      if (arg==NULL || strcasestr(arg,argv[iarg])!=arg)
        break;
      if (dtm_help[icmd].args[++iarg]==NULL && iarg==argc) { // complete match
        for(int iline=0; dtm_help[icmd].lines[iline]!=NULL; iline++)
          _console->printv("%s\n",dtm_help[icmd].lines[iline]);
        return;
      }
    }
  }
  _console->printv("No help for %s ...\n",argv[0]);
}

static void parse_command(int argc, char** argv)
{
  if (lverbose) {
    printf("parse_command parsing ");
    for(int i=0; i<argc; i++)
      printf("[%s] ",argv[i]);
    printf("\n");
  }

  for(int icmd=0; dtm_command[icmd].cmd!=NULL; icmd++) {
    for(int iarg=0; iarg<=argc; iarg++) {
      const char* arg = dtm_command[icmd].args[iarg];
      if (arg==NULL) {
//         _console->printv("parse_command executing %s\n",
//                          iarg>0?dtm_command[icmd].args[iarg-1]:"err");
        dtm_command[icmd].cmd(argc-iarg, &argv[iarg]);
        return;
      }
      if (strcasestr(arg,argv[iarg])!=arg)
        break;
    }
  }
  _console->printv("parse error : ");
  for(int iarg=0; iarg<argc; iarg++)
    _console->printv("%s ",argv[iarg]);
  _console->printv("\n");
}

class MainCommand : public cm::shell::Command {
public:
  MainCommand() {}
public:
  void open   (cm::shell::AbsConsole* console)
  {
    console->printv("\n@@@ Cluster Manager @@@\n");
    console->printv("** Get help with command completion using '?' **\n");
    console->printv("dtm> ");
  }
  void process(int argc, char** argv, cm::shell::AbsConsole* console)
  {
    _console = console;
    _console->printv("\n");

    if (lverbose) {
      printf("process parsing ");
      for(int i=0; i<argc; i++)
        printf("[%s] ",argv[i]);
      printf("\n");
    }

    char* last = argc > 0 ? argv[argc - 1] : 0;

    if (argc > 0 && *(last + strlen(last) - 1)=='?') {
      *(last + strlen(last) -1) = 0;  // strip off the '?'
      if (strlen(last)==0)            // remove last word if empty
        argc--;
      parse_help(argc, argv);
    }
    else if (argc > 0) {
      parse_command(argc, argv);
    }
    else
      console->printv("\n");

    console->printv("dtm> ");
  }
};

void usage(const char* p)
{
  printf("Usage: %s [options]\n",p);
  printf("options: -m\tLoad configuration from BSI memory (Default)\n"
         "         -l\tLoad configuration from file (.current)\n"
         "         -s\tRead configuration from switch\n"
         "         -r\tReset switch\n"
         "         -b\tPoll BSI for updates\n"
         "         -i\tPoll for interrupts\n"
         "         -t\tStart telnet server (port 30000)\n"
         "         -d\tDaemonize (fork)\n"
         "         -B <poll period(msec)>\tPeriod for BSI poll\n"
         "         -F\tuse BSI FIFO\n"
         "         -v\tverbose\n");
  exit(1);
}

static void sigintHandler(int);
static void* poll_bsi(void*);

int main(int argc, char** argv)
{
  enum { LoadBsi, LoadFile, ReadSw } init = LoadBsi;
  bool lreset=false;
  bool lbsi  =false;
  bool lteln =false;
  bool lDaemonize =false;

  //  extern char* optarg;
  //  char* endPtr;
  int c;
  while ( (c=getopt( argc, argv, "lsdmbrtvFB:")) != EOF ) {
    switch(c) {
    case 'l': init=LoadFile; break;
    case 's': init=ReadSw  ; break;
    case 'm': init=LoadBsi ; break;
    case 'b': lbsi  =true; break;
    case 'B':
      { unsigned v = strtoul(optarg,NULL,0); 
        if (v) _bsi_poll_period = v*1000;
      } break;
    case 'F': lfifo =true; break;
    case 'r': lreset=true; break;
    case 't': lteln =true; break;
    case 'd': lDaemonize=true; break;
    case 'v': lverbose=true; break;
    case 'h':
    case '?': 
    default : usage(argv[0]); break;
    }
  }

  //
  //  Daemonize
  //
  if (lDaemonize) {

    pid_t pid = fork();
  
    if (pid < 0)  /* Fork failed */
      exit(EXIT_FAILURE);

    if (pid > 0)  /* Close the parent */
      exit(EXIT_SUCCESS);

    umask(0);

    /* Open a connection to the syslog server */
    cm::service::print_init(argv[0]);
  
    /* Sends a message to the syslog daemon */
    cm::service::prints("Successfully started fm daemon\n");
  
    pid_t sid;
 
    /* Try to create our own process group */
    sid = setsid();
    if (sid < 0) {
      cm::service::prints("Could not create process group\n");
      exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0) {
      cm::service::prints("Could not change working directory to /\n");
      exit(EXIT_FAILURE);
    }
  }

  //
  //  Continue about our business
  //

  DtmEnv env;

  char* p = new char[max_size];
  buff = new char[max_size];

  switch(init) {
  case LoadFile: load_config(".current"); break;
  case LoadBsi : load_bsi   (&env); break;
  default: break;
  }

  cm::net::Manager* mgr = new(p) cm::net::Manager(1,&_config);

  if (init==ReadSw) {
    mgr->read(&_config);
    //  fill in any holes here
  }

  if (lreset)
    mgr->deviceMgr(_dev).reset(_config);
  else if (init!=ReadSw)
    mgr->configure(0,_config);

  _mgr = mgr;

  pthread_t id;
  pthread_attr_t flags;
  size_t min_stack,stacksize=0x10000;
  pthread_attr_init(&flags);
  pthread_attr_setdetachstate(&flags, PTHREAD_CREATE_DETACHED);
  pthread_attr_getstacksize(&flags, &min_stack);
  if (stacksize < min_stack) stacksize = min_stack;
  pthread_attr_setstacksize(&flags, stacksize);

  // Poll thread for BSI
  if (lbsi)
    pthread_create(&id, &flags, poll_bsi, &env);

  // telnet thread
  if (lteln) {
    MainCommand* cmd = new MainCommand;
    pthread_create(&id, &flags, cm::shell::initialize, cmd);
  }

  // Unix signal support
  struct sigaction int_action;

  int_action.sa_handler = sigintHandler;
  sigemptyset(&int_action.sa_mask);
  int_action.sa_flags = 0;
  int_action.sa_flags |= SA_RESTART;

  if (sigaction(SIGINT, &int_action, 0) > 0)
    printf("Couldn't set up SIGINT handler\n");
  if (sigaction(SIGKILL, &int_action, 0) > 0)
    printf("Couldn't set up SIGKILL handler\n");
  if (sigaction(SIGSEGV, &int_action, 0) > 0)
    printf("Couldn't set up SIGSEGV handler\n");
  if (sigaction(SIGABRT, &int_action, 0) > 0)
    printf("Couldn't set up SIGABRT handler\n");
  if (sigaction(SIGTERM, &int_action, 0) > 0)
    printf("Couldn't set up SIGTERM handler\n");

  if (!lverbose) {
    /* Close the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO); 
  } 

  sem_t _sem;
  sem_init(&_sem, 0, 0);
  sem_wait(&_sem);

  sigintHandler(0);
  return 0;
}

static const char* speedstr[] = { "NC","DIS","1GB","1GBL1","1GBL2","1GB-R","XAUI" };

static void setup_interface(unsigned  v, 
                            bool      lslot,
                            unsigned& mslot, 
                            unsigned& zone2)
{
  if (((v>>s_stat)&m_stat)==ok_stat) { // Done|Error|MBZ
    unsigned port  = (v>>s_port)&m_port;
    unsigned dslot = (v>>s_slot)&m_slot;
    unsigned dchan = (v>>s_chan)&m_chan;
    if (port < MAX_PORT) {
      mslot |= (1<<dslot);
      if (lslot) {
        zone2 |= (2<<port); // 1-based port numbers
        for(unsigned k=0; k<32; k++)
          if (__interfaces[k].port==port+1) {
            std::stringstream o;
            o << "lslot" << dslot+1
              << "-" << dchan+1;
            __interfaces[k].name = o.str();
            break;
          }
      }
    }
  }
}

void  load_bsi(DtmEnv* env)
{
  memset(&_config.portCfg,0,sizeof(_config.portCfg));
  memset(&_config.swCfg.trunk[0],0,FmSwitchConfig::Trunks*sizeof(FmPortMask));

  if (lfifo) {
    // Drain the FIFO
    uint32_t data = BsiRead32(env->bsi,BSI_FIFO_OFFSET);
    unsigned n = 0;
    while(BSI_FIFO_GET_VALID(data)) {
      data = BsiRead32(env->bsi,BSI_FIFO_OFFSET);
      n++;
    }
    if (lverbose)
      cm::service::printv("Drained %d entries from BSI FIFO\n",n);
  }


  if (lverbose)
    cm::service::printv("PORT  TYPE   GROUP\n");

  // Read from RAM
  for(unsigned port = 0; port<MAX_PORT; port++) {
    
    unsigned val = BsiRead32(env->bsi,BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + port);

    unsigned speed    = val&0xff;
    unsigned trunk    = (val>>16)&0xff;
    unsigned portMask = 1<<(port+1);

    switch(speed) {
    case CMB_SWITCH_PORT_TYPE_1000BASEX_LANE3 :
      _config.portCfg.portsRxFlip |= portMask;
      _config.portCfg.portsTxFlip |= portMask;
    case CMB_SWITCH_PORT_TYPE_1000BASEX_LANE0 :
      _config.portCfg.ports1Gb |= portMask;
      break;
    case CMB_SWITCH_PORT_TYPE_XAUI :
      _config.portCfg.ports10Gb |= portMask;
      break;
    default:
      break;
    }

    if (trunk)
      _config.swCfg.trunk[trunk-1] |= portMask;

    if (lverbose && speed >= CMB_SWITCH_PORT_TYPE_1000BASEX_LANE0)
      cm::service::printv("%4d  %5.5s  %d\n",port,speedstr[speed],trunk);
  }

  //  Get Fabric map to calculate port forwarding
  //    Need to get channel map
  //    Need to determine if we are a hub in dual star or not

  if (lverbose) {  // dump the whole thing
    Offset offset = BSI_FABRIC_MAP_OFFSET;
    for(unsigned i=0; i<FABRIC_MAP_SLOTS; i++) {
      for(unsigned j=0; j<FABRIC_MAP_WORDS; j++,offset++) {
        uint32_t v = BsiRead32(env->bsi, offset);
        cm::service::printv(" %04x %04x", v&0xffff, v>>16);
      }
      cm::service::printv("\n");
    }
    uint32_t v = BsiRead32(env->bsi, offset);
    cm::service::printv("[%08x]\n",v);
  }

  unsigned lslot = 0;
  while(!(lslot&0x80000000))
    lslot = BsiRead32(env->bsi,BSI_FABRIC_MAP_OFFSET + 
                      FABRIC_MAP_SLOTS*FABRIC_MAP_WORDS);
  
  //  bool fp_hub = lslot&0x40000000;
  bool fp_hub = true;

  lslot &= 0xf;

  unsigned node_slots=0, hub_slots=0;
  unsigned zone2(0);

  { Offset offset = BSI_FABRIC_MAP_OFFSET;
    for(unsigned i=0; i<FABRIC_MAP_SLOTS; i++) {
      unsigned mslot=0;
      for(unsigned j=0; j<FABRIC_MAP_WORDS; j++,offset++) {
        uint32_t v = BsiRead32(env->bsi, offset);
        setup_interface(v>> 0,i==lslot,mslot,zone2);
        setup_interface(v>>16,i==lslot,mslot,zone2);
      }
      
      //  Require at least three interconnects for a potential hub
      unsigned hslot=mslot;
      hslot &= (hslot-1);
      hslot &= (hslot-1);
      hslot &= (hslot-1);
      if (hslot)
        hub_slots  |= 1<<i;
      else if (mslot)
        node_slots |= 1<<i;
    }
  }

  _zone2 = zone2;

  if (lverbose) {
    cm::service::printv("fmd lslot %d: zone 2 %08x  hubs %08x  nodes %08x\n",
                        lslot, zone2, hub_slots, node_slots);
    for(unsigned i=0; i<32; i++)
      if (!__interfaces[i].name.empty())
        cm::service::printv("%s [%d]\n",
                            __interfaces[i].name.c_str(),
                            __interfaces[i].port);
  }

  bool lmesh = node_slots==0 || hub_slots==0;
  if (lmesh) {
    for(unsigned i=0; i<MAX_PORT; i++) {
      unsigned mask = unsigned(FmPortMask::allPorts());
      //
      //  Don't forward from zone2 to zone2 in a full mesh
      //
      if (zone2&(1<<(i+1)))
        mask &= ~zone2;

      _config.swCfg.portfwd_table.forward[i] = FmPortMask(mask);
    }
  }
  else if (hub_slots & (1<<lslot)) {

    for(unsigned i=0; i<MAX_PORT; i++)
      _config.swCfg.portfwd_table.forward[i] = FmPortMask::allPorts();

    //
    //  Don't forward from (other) hub to zone2
    //
    Offset offset = BSI_FABRIC_MAP_OFFSET + FABRIC_MAP_WORDS*lslot;
    for(unsigned j=0; j<FABRIC_MAP_WORDS; j++, offset++) {
      uint32_t v = BsiRead32(env->bsi, offset);
      if (((v>>s_stat)&m_stat)==ok_stat) { // Done|Error|MBZ
        unsigned port  = (v>>s_port)&m_port;
        unsigned dslot = (v>>s_slot)&m_slot;
        if (hub_slots & (1<<dslot))
          _config.swCfg.portfwd_table.forward[port] &= ~zone2;
      }
      v >>= 16;
      if (((v>>s_stat)&m_stat)==ok_stat) { // Done|Error|MBZ
        unsigned port  = (v>>s_port)&m_port;
        unsigned dslot = (v>>s_slot)&m_slot;
        if (hub_slots & (1<<dslot))
          _config.swCfg.portfwd_table.forward[port] &= ~zone2;
      }
    }
  }
  else {
    //
    //  For nodes in a dual star, trunk together the hub connections
    //
    for(unsigned i=0; i<MAX_PORT; i++) {
      unsigned mask = FmPortMask::allPorts();
      if (zone2&(1<<(i+1)))
        mask &= ~zone2;
      _config.swCfg.portfwd_table.forward[i] = FmPortMask(mask);
    }

    memset(_config.swCfg.trunk, 0, FmSwitchConfig::Trunks*sizeof(FmPortMask));
    _config.swCfg.trunk[FmSwitchConfig::Trunks-1] |= zone2;
  }

  //
  //  Front panel forwarding
  //
  for(unsigned i=0; i<MAX_PORT; i++) {
    //
    //  Don't forward from fp to fp
    //
    if (_fp&(1<<(i+1)))
      _config.swCfg.portfwd_table.forward[i] &= ~_fp;
    if (!fp_hub) {
      //
      // Don't forward from zone2 to fp unless a hub
      //
      if (zone2&(1<<(i+1)))
        _config.swCfg.portfwd_table.forward[i] &= ~_fp;
      //
      //  Don't forward from fp to zone2 unless a hub
      //
      if (_fp&(1<<(i+1)))
        _config.swCfg.portfwd_table.forward[i] &= ~zone2;
    }
  }

  //
  //  VLAN setup (for ingress/egress frames on external ports)
  //
  cm::svc::ClusterIpInfo info;
  _config.swCfg.port_vlan = SwmPortVlanConfig(SwmPortVlanConfig::Init);
  
  //
  //  Per port configuration
  //
  if (info.use_vlan()) {
    unsigned vlanId = info.vlan_id();
    if (info.reject_untagged())     // require tagged
      _config.swCfg.port_vlan.tagged     |=  _fp;
    if (info.reject_tagged())     // require untagged
      _config.swCfg.port_vlan.untagged   |=  _fp;
    if (info.reject_boundary_violation())     // filter ingress boundary violations
      _config.swCfg.port_vlan.strict     |=  _fp;

    for(unsigned i=0; i<FmPort::MAX_PORT; i++)
      _config.swCfg.port_vlan.defaultVlan[i] = vlanId;

    //
    //  Apply ingress tag rule to egress policy
    //
    for(unsigned i=0; i<0x1000; i++)
      _config.swCfg.vid_table[i] = SwmVlanTableEntry(0,0,0,0,0);

    _config.swCfg.vid_table[vlanId] = SwmVlanTableEntry(unsigned(FmPortMask::allPorts()),
                                                        info.reject_untagged() ? _fp : 0,
                                                        0, vlanId&0x1f, 0);
  }
  
  if (lverbose) {
    for(unsigned i=0; i<MAX_PORT; i++)
      cm::service::printv("fwd[%d] %08x\n",
                          i, unsigned(_config.swCfg.portfwd_table.forward[i]));
    for(unsigned i=0; i<FmSwitchConfig::Trunks; i++)
      cm::service::printv("trunk[%d] %08x\n",
                          i, unsigned(_config.swCfg.trunk[i]));
  }
}

void* poll_bsi(void* arg)
{
  DtmEnv* env = reinterpret_cast<DtmEnv*>(arg);
  cm::net::Manager* mgr = _mgr;

  uint32_t val;
  uint32_t addr;

  uint32_t* val_cache = new uint32_t[MAX_PORT];
  memset(val_cache, 0, sizeof(uint32_t)*MAX_PORT);

//   MeshBoundary* mesh = new MeshBoundary(mgr->deviceMgr(0), 
//                                         FmPortMask::phyPorts()&~(_fp|_zone2), 
//                                         _fp, 
//                                         _zone2);

  while(1) {
    usleep(_bsi_poll_period);

    bool lHeader=false;

    unsigned port=0;
    while(1) {

      if (lfifo) {
        uint32_t data = BsiRead32(env->bsi,BSI_FIFO_OFFSET);
        if (!BSI_FIFO_GET_VALID(data)) break;
        
        addr = BSI_FIFO_GET_ADDR(data) >> 2;
        val  = BsiRead32 (env->bsi,addr);
        data = BsiRead32 (env->bsi,BSI_FIFO_OFFSET);
      }
      else {
        // Read from RAM
        // Mock up as if read from FIFO
        if (port==MAX_PORT) break;

        addr = port + BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET;
        val  = BsiRead32(env->bsi,addr);
        port++;
      }

      unsigned vspd  = val&0xff;
      unsigned trunk = (val>>16)&0xff;

      if (addr >= BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET &&
          addr <  BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET+MAX_PORT) {
        
        unsigned chan = addr - BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET;
        if (val == val_cache[chan]) continue;

        val_cache[chan] = val;
        FmPort sw_port(1+chan);
        cm::fm::PortSpeed speed = cm::fm::None;
        unsigned flip=0;

        if (vspd==CMB_SWITCH_PORT_TYPE_1000BASEX_LANE3) {
          flip = 1;
          speed = cm::fm::X1Gb;
        }
        else if (vspd==CMB_SWITCH_PORT_TYPE_1000BASEX_LANE0)
          speed = cm::fm::X1Gb;
        else if (vspd==CMB_SWITCH_PORT_TYPE_XAUI)
          speed = cm::fm::X10Gb;
        else if (vspd==CMB_SWITCH_PORT_TYPE_NC)
          continue;
        else
          speed = cm::fm::None;

        if (_config.swCfg.trunk[FmSwitchConfig::Trunks-1].contains(sw_port))
          trunk = FmSwitchConfig::Trunks-1;

        if (lverbose) {
          if (!lHeader) {
            lHeader=true;
            cm::service::printv("PORT  TYPE   GROUP\n");
          }
          cm::service::printv("%4d  %5.5s  %d\n",chan,speedstr[speed],trunk);
        }

//         mesh->clear_port(sw_port);

        // SEM: first bring down the port
        mgr->deviceMgr(_dev).configurePort(sw_port, cm::fm::None);
        
        // SEM: and now configure it
        mgr->deviceMgr(_dev).configurePort(sw_port, speed, flip, flip, 0, trunk);
      }
    }

    //
    //  Poll for interrupts
    //
    for(unsigned i=0; i<mgr->nDevices(); i++)
      mgr->deviceMgr(i).handle_interrupt();

    //
    //  Poll for traffic directed to CPU port
    //
//     for(unsigned i=0; i<mgr->nDevices(); i++) {
//       int len;
//       while( (len=mgr->deviceMgr(i).dev().rxPacket(beg)) ) {
//         FmLCIPacket pkt(beg, len);
//         mesh->receive(pkt);
//       }
//     }

//    mesh->update();
  }
  return 0;
}

void sigintHandler(int iSignal)
{
  if (lverbose)
    cm::service::printv("fmd caught signal %d\n",iSignal);

  save_config(".current",_mgr->deviceMgr(_dev)._config);

  _mgr->read(&_config);
  save_config(".recovered",_config);
  
  delete[] buff;
  exit(0);
}

