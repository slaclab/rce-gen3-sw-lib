#include "TahoeShell.hh"
#include "Manager.hh"
#include "DtmInterface.hh"

#include "logger/FormatMessage.hh"

#include "cm_svc/CmHw.hh"
#include "cm_svc/ChannelMap.hh"
#include "cm_svc/PlxBridge.hh"
#include "cm_svc/Print.hh"
#include "cm_svc/ClusterIpInfo.hh"
#include "cm_shell/ShellCommon.hh"
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

#define strcasestr strstr

static cm::net::Manager* _mgr = 0;
static DtmInterface* __interfaces;
static int _dev = 0;
static cm::shell::AbsConsole* _console = 0;
static bool lverbose=true;
//static unsigned* beg = new unsigned[1512>>2];  // packet buffer

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
static cm::shell::AbsConsole* _log = _dtmlog;

static const size_t max_size = 0x100000;
static char _buff[64];

/*

static void load_config(const char*);
static void save_config(const char*,const FmConfig&);

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
*/

//
//  Shell commands
//
typedef struct {
  const char** args;
  const char** lines;
} DtmHelp_t;

static const char* fm_lines[] = { "reset",
//                                   "load ...",
//                                   "save ...",
                                  "set ...",
                                  "show ...",
                                  "enable ...",
                                  "disable ...",
                                  "lanetest ...",
                                  NULL };
static const char* reset_args[] = { "reset",NULL };
static const char* reset_lines[] = {"reset\t(reset switch and apply configuration)",
                 NULL };

// static const char* load_args[] = { "load",NULL };
// static const char* load_lines[]= { "load <filename>\t(load configuration from <filename>)",
//                                    NULL };

// static const char* save_args[] = { "save",NULL };
// static const char* save_lines[]= { "save <filename>\t(save configuration to <filename>)",
//                                    NULL };

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
//                                 { load_args, load_lines },
//                                 { save_args, save_lines },
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

static DtmInterface _interfaces(unsigned i)
{
  return __interfaces[i];
}

#if 0
static int _port(const char* name)
{
  for(int i=0; i<32; i++) {
    DtmInterface intf = _interfaces(i);
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
    DtmInterface intf = _interfaces(i);
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
  FmConfig& cfg = _mgr->deviceMgr(_dev)._config;
  _mgr->deviceMgr(_dev).reset(cfg);
}

/*
static void _load(int argc, char** argv)
{
  load_config(argv[0]);
  _mgr->deviceMgr(_dev).configure(_config);
}

static void _save(int argc, char** argv)
{
  save_config(argv[0],_mgr->deviceMgr(_dev)._config);
}
*/

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
      DtmInterface intf = _interfaces(i);
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
      DtmInterface intf = _interfaces(i);
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
              
            _console->printv("\t------------------------------\n");
            _console->printv("\tserdes_ctrl_1 0x%08x\n", r->result().serdes_ctrl_1);
            _console->printv("\tserdes_ctrl_2 0x%08x\n", r->result().serdes_ctrl_2);
            _console->printv("\tserdes_ctrl_3 0x%08x\n", r->result().serdes_ctrl_3);
            _console->printv("\tserdes_test_mode 0x%08x\n", r->result().serdes_test_mode);
            _console->printv("\tserdes_status 0x%08x\n", r->result().serdes_status);
            _console->printv("\tserdes_ip 0x%08x\n", r->result().serdes_ip);
            _console->printv("\tserdes_im 0x%08x\n", r->result().serdes_im);
            _console->printv("\tserdes_bist_err_cnt 0x%08x\n", r->result().serdes_bist_err_cnt);
            _console->printv("\tpcs_cfg_1 0x%08x\n", r->result().pcs_cfg_1);
            _console->printv("\tpcs_cfg_2 0x%08x\n", r->result().pcs_cfg_2);
            _console->printv("\tpcs_cfg_3 0x%08x\n", r->result().pcs_cfg_3);
            _console->printv("\tpcs_cfg_4 0x%08x\n", r->result().pcs_cfg_4);
            _console->printv("\tpcs_cfg_5 0x%08x\n", r->result().pcs_cfg_5);
            _console->printv("\tpcs_ip 0x%08x\n", r->result().pcs_ip);
            _console->printv("\tpcs_im 0x%08x\n", r->result().pcs_im);
            _console->printv("\tmac_cfg_1 0x%08x\n", r->result().mac_cfg_1);
            _console->printv("\tmac_cfg_2 0x%08x\n", r->result().mac_cfg_2);
            _console->printv("\tmac_cfg_3 0x%08x\n", r->result().mac_cfg_3);
            _console->printv("\tmac_cfg_4 0x%08x\n", r->result().mac_cfg_4);
            _console->printv("\tmac_cfg_5 0x%08x\n", r->result().mac_cfg_5);
            _console->printv("\tmac_cfg_6 0x%08x\n", r->result().mac_cfg_6);
            _console->printv("\tmac_status 0x%08x\n", r->result().mac_status);
            _console->printv("\tmac_ip 0x%08x\n", r->result().mac_ip);
            _console->printv("\tmac_im 0x%08x\n", r->result().mac_im);
            _console->printv("\tstat_epl_error1 0x%08x\n", r->result().stat_epl_error1);
            _console->printv("\tstat_tx_pause 0x%08x\n", r->result().stat_tx_pause);
            _console->printv("\tstat_tx_crc 0x%08x\n", r->result().stat_tx_crc);
            _console->printv("\tstat_epl_error2 0x%08x\n", r->result().stat_epl_error2);
            _console->printv("\tstat_rx_jabber 0x%08x\n", r->result().stat_rx_jabber);
            _console->printv("\tepl_led_status 0x%08x\n", r->result().epl_led_status);
            _console->printv("\tepl_int_detect 0x%08x\n", r->result().epl_int_detect);
            _console->printv("\tstat_tx_bytecount 0x%08x\n", r->result().stat_tx_bytecount);
            _console->printv("\t------------------------------\n");
            _console->printv("\tpcs status (pcs_ip) %x\n", r->result().pcs_ip);
            _console->printv("\tmac status (mac_ip) %x\n", r->result().mac_ip);
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
      DtmInterface intf = _interfaces(i);
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
      DtmInterface intf = _interfaces(i);
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
    for(unsigned i=1; i<=FmPort::MAX_PORT; i++) {
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
      DtmInterface intf = _interfaces(i);
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
      DtmInterface intf = _interfaces(i);
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
      DtmInterface intf = _interfaces(i);
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
//                                       { _load           ,  load_args },
//                                       { _save           ,  save_args },
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

TahoeShell::TahoeShell(FmConfig& cfg,
                       DtmInterface* intf)
{
  if (_mgr) delete _mgr;
  _mgr = new cm::net::Manager(&cfg);
  __interfaces = intf;
}

void TahoeShell::open   (cm::shell::AbsConsole* console)
{
  console->printv("\n@@@ Cluster Manager @@@\n");
  console->printv("** Get help with command completion using '?' **\n");
  console->printv("dtm> ");
}

void TahoeShell::process(int argc, char** argv, cm::shell::AbsConsole* console)
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

