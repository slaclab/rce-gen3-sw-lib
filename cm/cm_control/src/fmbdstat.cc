/*
**  Daemon process for controlling Fulcrum switch
**
*/

//#include "bldInfo.h"

#include "ManagerB.hh"
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
#include "fmb/FmBaliMgr.hh"
#include "fmb/FmBali.hh"
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

#define EPL_PORT_OFFSET 0

static cm::net::ManagerB* _mgr = 0;
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

static void _show_reg_range(unsigned off, unsigned len, bool verbose);
static void _show_pcs_ip();

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
/**
static void load_config(const char*);
static void save_config(const char*,const FmConfig&);
**/
static char* buff = 0;

static const size_t max_size = 0x100000;

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
                                  "dump ...",
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
                                    "show glort\t(show glort tables)",
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
static const char* show_glort_args [] = { "show","glort",NULL };
static const char* show_glort_lines[] = { show_lines[6],
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

static const char* dump_args [] = { "dump", NULL };
static const char* dump_lines[] = { "dump cpuport\t(dump packets received on cpu port)",
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
                                { show_glort_args, show_glort_lines },
                                { show_reg_args, show_reg_lines },
                                { show_plx_args, show_plx_lines },
                                { show_log_args, show_log_lines },
                                { enable_args, enable_lines },
                                { disable_args, disable_lines },
                                { lanetest_args, lanetest_lines },
                                { dump_args, dump_lines },
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
    { "dpm2.0", 8  },
    { "dpm2.1", 9  },
    { "dpm3.0", 6  },
    { "dpm3.1", 7  },
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
  for(int i=0; i<32; i++) {
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
  bool rv = _mgr->deviceMgr(_dev).portSpeed(FmPort(port))!=cm::fm::None;
  // printf("[_enabled(%u) = %s]\n\r", port, rv ? "true" : "false");
  return rv;
}

static void _reset(int argc, char** argv)
{
  _mgr->deviceMgr(_dev).reset(_config);
}

static void _load(int argc, char** argv)
{
/**
  load_config(argv[0]);
  _mgr->deviceMgr(_dev).configure(_config);
**/
}

static void _save(int argc, char** argv)
{
/**
  save_config(argv[0],_mgr->deviceMgr(_dev)._config);
**/
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
//_console->printv("[Entered %s]\n", __PRETTY_FUNCTION__);
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
      _console->printv("%*u\t" , strlen(titles[3]), _mgr->deviceMgr(_dev).dev().port_cfg_1[intf.port]&0xfff);
      _console->printv("[port=%d]\n", intf.port);
    }
  }
}

static void _show_int(int argc, char** argv)
{
  _console->printv("[Entered %s (argc=%d)]\n", __PRETTY_FUNCTION__, argc);
  if (argc<1) {
    _console->printv("show int <port> : too few arguments (%d)\n", argc);
  } else if (argc>1) {
    _console->printv("show int <port> : too many arguments (%d)\n", argc);
  } else if (strcasecmp(argv[0],"raw")==0) {
    _show_reg_range(0x50000, 36u, false);
  } else if (strcasecmp(argv[0],"pcs_ip")==0) {
    _show_pcs_ip();
  } else {
    for(int i=0; i<32; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      if (strcasecmp(argv[0],intf.name.c_str())==0 ||
          strcasecmp(argv[0],"all")==0) {
        _console->printv("interface %s [port=%d]\n",intf.name.c_str(),intf.port);

        char* reply = new char[0x80000];
        unsigned reply_len;
        _console->printv("&_mgr->deviceMgr(_dev).dev().epl_phy[intf.port-EPL_PORT_OFFSET] == %p\n\r", &_mgr->deviceMgr(_dev).dev().epl_phy[intf.port-EPL_PORT_OFFSET]);
        SwmEplReadRequestB* req = new SwmEplReadRequestB( &_mgr->deviceMgr(_dev).dev().epl_phy[intf.port-EPL_PORT_OFFSET] );
        _mgr->process(*req, (void*)reply, reply_len);
        delete req;

        if (!reply_len) {
          _console->printv("Error fetching port status\n");
        }
        else {
          _console->printv("reply_len = %u\n", reply_len);
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
            const SwmEplReadReplyB* r = reinterpret_cast<const SwmEplReadReplyB*>(reply);
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
            _console->printv("\tsyncbuf_cfg 0x%08x\n", r->result().syncbuf_cfg);
            _console->printv("\tmac_cfg_1 0x%08x\n", r->result().mac_cfg_1);
            _console->printv("\tmac_cfg_2 0x%08x\n", r->result().mac_cfg_2);
            _console->printv("\tmac_cfg_3 0x%08x\n", r->result().mac_cfg_3);
                               /* reserved4 */
            _console->printv("\tmac_cfg_5 0x%08x\n", r->result().mac_cfg_5);
            _console->printv("\tmac_cfg_6 0x%08x\n", r->result().mac_cfg_6);
            _console->printv("\ttx_vpri_map_1 0x%08x\n", r->result().tx_vpri_map_1);
            _console->printv("\ttx_vpri_map_2 0x%08x\n", r->result().tx_vpri_map_2);
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

        Bali::FmBali& d = _mgr->deviceMgr(_dev).dev();
        unsigned v = d.port_cfg_2[intf.port];
        _console->printv("\tfwd mask   %08x\n",v);
        /**
        v = d.trunk_port_map[intf.port];
        if (v&(1<<4))
          _console->printv("\tagg-group  %02d\n",v&0xf);
        **/        

        return;
      }
    }
    _console->printv("show int %s : interface not found\n",argv[0]);
  }
}

static void _show_stat_brief(int argc, char**)
{
//_console->printv("[Entered %s]\n", __PRETTY_FUNCTION__);
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
    SwmAllPortMibRequestB* req = new SwmAllPortMibRequestB( &_mgr->deviceMgr(_dev).dev() );
    _mgr->process(*req, (void*)reply, reply_len);
    delete req;
    if (!reply_len) {
      _console->printv("Error fetching port statistics\n");
      delete[] reply;
      return;
    }

    const SwmAllPortMibReplyB& rrep = *(const SwmAllPortMibReplyB*)reply;

    for(unsigned i=0; titles[i]!=NULL; i++)
      _console->printv("%s%c",titles[i],titles[i+1] ? '\t':'\n');
    for(unsigned i=0; titles[i]!=NULL; i++)
      _console->printv("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
    for(unsigned i=0; i<32; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      const Bali::SwmPortMibCounters& c = rrep.result().port(intf.port);
      _console->printv("%-*s\t", strlen(titles[0]), intf.name.c_str());
      //unsigned long long rxPkts = c.RxUcstPktsIPv4 + c.RxBcstPktsIPv4 + c.RxMcstPktsIPv4;
      unsigned long long rxPkts = c.RxUcstPktsNonIp + c.RxBcstPktsNonIp + c.RxMcstPktsNonIp;
      _console->printv("%*lld\t" , strlen(titles[1]), rxPkts);
      unsigned long long txPkts = c.TxUcstPkts + c.TxBcstPkts + c.TxMcstPkts;
      _console->printv("%*lld\t" , strlen(titles[2]), txPkts);
      unsigned long long rxErrs = c.RxFcsErrors + c.RxSymbolErrors + c.RxFrameSizeErrors;
      _console->printv("%*lld\t" , strlen(titles[3]), rxErrs);
      unsigned long long txErrs = c.TxTimeoutDrop + c.TxErrorDrop + c.TxLoopbackDrop;
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
//_console->printv("[Entered %s]\n", __PRETTY_FUNCTION__);
  if (argc>0)
    _console->printv("show stat global : too many arguments (%d)\n",argc);
  else {
    /**
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
    **/
  }
}

static void _show_stat(int argc, char** argv)
{
//_console->printv("[Entered %s]\n", __PRETTY_FUNCTION__);
  if (argc<1) {
    _console->printv("show stat <port> : too few arguments (%d)\n",argc);
  } else if (argc>1) {
    _console->printv("show stat <port> : too many arguments (%d)\n",argc);
  } else {
    bool lfound=false;
    for(int i=0; i<32; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      if (strcasecmp(argv[0],intf.name.c_str())==0 ||
          strcasecmp(argv[0],"all")==0) {
        _console->printv("interface %s [port=%d]\n",
                         intf.name.c_str(),intf.port);

        char* reply = new char[0x80000];
        unsigned reply_len;
        SwmPortMibRequestB* req = new SwmPortMibRequestB( &_mgr->deviceMgr(_dev).dev().port_mib(intf.port) );
        _mgr->process(*req, (void*)reply, reply_len);
        delete req;

        if (!reply_len) {
          _console->printv("Error fetching port statistics\n");
        }
        else {
          const SwmPortMibReplyB* r = reinterpret_cast<const SwmPortMibReplyB*>(reply);
          const Bali::SwmPortMibCounters& c = r->result();
#define ADD_RSRV(group,len)
#define ADD_STAT(group,member0,member1) printStat(member0, member1)
          ADD_STAT_PORTCOUNTERS
#undef ADD_STAT
#undef ADD_RSRV
        }
        delete[] reply;
        lfound=true;
      }
    }
    if (!lfound)
      _console->printv("show stat %s : interface not found\n",argv[0]);
  }
}

static void _show_macaddr(int argc, char** argv)
{
  _console->printv("[Entered %s]\n", __PRETTY_FUNCTION__);
  if (argc>0)
    _console->printv("show macaddr: too many arguments (%d)\n",argc);
  else {
    char* reply = new char[sizeof(SwmMacEntryReadReplyB)];
    unsigned reply_len;

    unsigned nentries=0;
    unsigned nstatic=0;

    _console->printv("%8.8s%20.20s%10.10s%10.10s%10.10s%6.6s%6.6s%4.4s\n",
                     "Entry", "MAC Address", "Ports", "Glort", "Lock", "Fid", "Trig", "P");

    for(unsigned j=0; j<SwmMacTable::Entries; j++) {
      SwmMacEntryReadRequestB* req = new SwmMacEntryReadRequestB( &_mgr->deviceMgr(_dev).dev().ma_table_entry[j] );
      _mgr->process(*req, (void*)reply, reply_len);
      delete req;
    
      if (!reply_len) {
        _console->printv("Error fetching mac address table entry %u\n",j);
      }
      else {
        const SwmMacEntryReadReplyB& r = *reinterpret_cast<const SwmMacEntryReadReplyB*>(reply);
        const Bali::SwmMacTableEntry& e = r.result();
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
          if (e.dst_is_glort())
            buf[0]=0;
          else
            sprintf(buf,"%08x",unsigned(e.port_mask()));
          _console->printv("%10.10s",buf);
          if (e.dst_is_glort())
            sprintf(buf,"%08x",unsigned(e.glort()));
          else
            buf[0]=0;
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
    }
    _console->printv("Found %d entries, %d static\n", nentries, nstatic);
  }
}

static void _show_vlans(int argc, char** argv)
{
  if (argc>0)
    _console->printv("show vlans: too many arguments (%d)\n",argc);
  else {
    _console->printv("%5.5s%8.8s%8.8s%5.5s\n",
                     "VID", "Members", "Tagged", "Cnt");
    char* reply = new char[sizeof(SwmIngVlanEntryReadReplyB)];
    unsigned reply_len;
    //    for(unsigned id=0; id<0x1000; id++) {
    for(unsigned id=0; id<0x10; id++) {
      SwmIngVlanEntryReadRequestB* req = new SwmIngVlanEntryReadRequestB( &_mgr->deviceMgr(_dev).dev().ingress_vid_table[id] );
      _mgr->process(*req, (void*)reply, reply_len);
      delete req;
    
      if (!reply_len) {
        _console->printv("Error fetching vlan entry\n");
      }
      else {
        const SwmIngVlanEntryReadReplyB& r = *reinterpret_cast<const SwmIngVlanEntryReadReplyB*>(reply);
        const Bali::SwmIngressVlanTableEntry& c = r.result();
        if (!c.empty()) {
          _console->printv("%5.5d %07x %07x %4.4d\n",
                           id,
                           unsigned(c.members()),
                           unsigned(c.tagged()),
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
    /**
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
    **/
  }
}

static void _show_trunks(int argc, char** argv)
{
  if (argc>0)
    _console->printv("show trunks: too many arguments (%d)\n",argc);
  else {
    /**
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
    **/
  }
}

static void _show_glorts(int argc, char** argv)
{
  if (argc>0)
    _console->printv("show glorts: too many arguments (%d)\n",argc);
  else {
    { 
      _console->printv("GLORT DEST TABLE\n");
    
      char* reply = new char[sizeof(SwmGlortDestEntryReadReplyB)];
      unsigned reply_len;

      for(unsigned i=0; i<4096; i++) {
        SwmGlortDestEntryReadRequestB* req = new SwmGlortDestEntryReadRequestB( &_mgr->deviceMgr(_dev).dev().glort_dest_table[i] );
        _mgr->process(*req, (void*)reply, reply_len);
        delete req;

        if (reply_len) {
          SwmGlortDestEntryReadReplyB& r = *reinterpret_cast<SwmGlortDestEntryReadReplyB*>(reply);
          if (r.result().destMask()) {
            _console->printv("[%6.6d] %08x",i,unsigned(r.result().destMask()));
            if (r.result().ip_mcast_index())
              _console->printv(" %04x",r.result().ip_mcast_index());
            _console->printv("\n");
          }
        }
      }
      delete[] reply;
    }
    {
      _console->printv("GLORT CAM/RAM\n");
    
      char* reply = new char[sizeof(SwmGlortRamEntryReadReplyB)];
      unsigned reply_len;

      for(unsigned i=0; i<256; i++) {
        SwmRegReadRequest* req = new SwmRegReadRequest( &_mgr->deviceMgr(_dev).dev().glort_cam[i] );
        _mgr->process(*req, (void*)reply, reply_len);
        delete req;

        if (reply_len) {
          SwmRegReadReply& r = *reinterpret_cast<SwmRegReadReply*>(reply);
          if (r.result()&0xffff0000) {
            _console->printv("[%6.6d] %04x &%04x",i,r.result()&0xffff,r.result()>>16);

            SwmGlortRamEntryReadRequestB* req = new SwmGlortRamEntryReadRequestB( &_mgr->deviceMgr(_dev).dev().glort_ram[i] );
            _mgr->process(*req, (void*)reply, reply_len);
            delete req;

            if (reply_len) {
              SwmGlortRamEntryReadReplyB& v = *reinterpret_cast<SwmGlortRamEntryReadReplyB*>(reply);
              if (v.result().strict()&1) {
                _console->printv(": %03x %s",v.result().destIndex(),v.result().parityError()?"E":" ");
              }
              else {
                _console->printv(": %03x +[%d.%d] %%[%d] %s",
                                 v.result().destIndex(),
                                 v.result().subIndexA_pos(),
                                 v.result().subIndexA_nb(),
                                 v.result().destCount(),
                                 v.result().parityError()?"E":" ");
              }
            }
            _console->printv("\n");
          }
        }
      }
      delete[] reply;
    }
  }
}

static unsigned _read_reg(unsigned offset)
{
  const uint32_t* addr = 0;
  cm::svc::PlxBridge& bridge = cm::svc::PlxBridge::instance();
  return (bridge.read(addr+offset));
}

static void _show_pcs_ip()
{
  unsigned off = 0x5000e;
  const uint32_t* addr = 0;
  cm::svc::PlxBridge& bridge = cm::svc::PlxBridge::instance();
  for (unsigned int uu=0; uu<=24; uu++) {
    _console->printv("reg[0x%x] = %x\n",off+(uu*0x400), bridge.read(addr+off+(uu*0x400)));
  }
}

static void _show_reg_range(unsigned off, unsigned len, bool verbose)
{
  const uint32_t* addr = 0;
  cm::svc::PlxBridge& bridge = cm::svc::PlxBridge::instance();
  if (verbose) {
    bridge.set_verbose(1);
  }
  for (unsigned int uu=0; uu<len; uu++) {
    _console->printv("reg[0x%x] = %x\n",off+uu, bridge.read(addr+off+uu));
  }
  if (verbose) {
    bridge.set_verbose(0);
  }
}

static void _show_reg(int argc, char** argv)
{
  if (argc!=1) {
    _console->printv("show reg <reg>\n");
  }
  else {
    unsigned off = strtoul(argv[0],NULL,0);
    _show_reg_range(off, 1u, true);
  }
}
static void _show_plx(int argc, char** argv)
{
  if (lverbose) {
    printf("[Entered %s:%s]\n\r", __FILE__, __FUNCTION__);
  }
  cm::svc::PlxBridge& b = cm::svc::PlxBridge::instance();
  b.dump();
}
static void _show_log(int argc, char** argv)
{
  _dtmlog->dump(_console);
}

static void _print_regs()
{
  _console->printv("  reg[0x%05x] = %8x (VPD)\n", 0x304, _read_reg(0x304));

  _console->printv("  reg[0x%05x] = %8x (Port 0 serdes_ctrl_3)\n", 0x50002, _read_reg(0x50002));
  _console->printv("  reg[0x%05x] = %8x (Port 0 pcs_ip)\n", 0x5000e, _read_reg(0x5000e));
  _console->printv("  reg[0x%05x] = %8x (Port 0 mac_status)\n", 0x50022, _read_reg(0x50022));
  _console->printv("  reg[0x%05x] = %8x (Port 0 mac_im)\n", 0x50024, _read_reg(0x50024));

  _console->printv("  reg[0x%05x] = %8x (Port 1 serdes_ctrl_3)\n", 0x50402, _read_reg(0x50402));
  _console->printv("  reg[0x%05x] = %8x (Port 1 pcs_ip)\n", 0x5040e, _read_reg(0x5040e));
  _console->printv("  reg[0x%05x] = %8x (Port 1 mac_status)\n", 0x50422, _read_reg(0x50422));
  _console->printv("  reg[0x%05x] = %8x (Port 1 mac_im)\n", 0x50424, _read_reg(0x50424));

  _console->printv("  reg[0x%05x] = %8x (Port 2 serdes_ctrl_3)\n", 0x50802, _read_reg(0x50802));
  _console->printv("  reg[0x%05x] = %8x (Port 2 pcs_ip)\n", 0x5080e, _read_reg(0x5080e));
  _console->printv("  reg[0x%05x] = %8x (Port 2 mac_status)\n", 0x50822, _read_reg(0x50822));
  _console->printv("  reg[0x%05x] = %8x (Port 2 mac_im)\n", 0x50824, _read_reg(0x50824));
}

static void _enable_port(int argc, char** argv)
{
  if (argc<2)
    _console->printv("enable <port> (<speed>): too few arguments (%d)\n",argc);
  else if (argc>3)
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
#if 0
        _console->printv("\rBefore:\n");
        _print_regs();
#endif
        sleep(1);
        //  bring up port & wait
        _mgr->deviceMgr(_dev).configurePort(FmPort(intf.port), speed, 0, 0, drive);
        sleep(1);
#if 0
        _console->printv("\rAfter:\n");
        _print_regs();
#endif
        //  clear counters
        char* reply = new char[0x80000];
        unsigned reply_len;
        SwmEplReadRequestB* req = new SwmEplReadRequestB( &_mgr->deviceMgr(_dev).dev().epl_phy[intf.port-EPL_PORT_OFFSET] );
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
  if (argc<1)
    _console->printv("disable <port> : too few arguments (%d)\n",argc);
  else if (argc>1)
    _console->printv("disable <port> : too many arguments (%d)\n",argc);
  else {
    for(int i=0; i<32; i++) {
      DtmInterface_t intf = _interfaces(i);
      if (intf.name.empty()) continue;
      if (strcasecmp(argv[0],intf.name.c_str())==0) {
        _console->printv("\rBefore:\n");
        _print_regs();
        sleep(1);
        //  bring down port
        _mgr->deviceMgr(_dev).configurePort(FmPort(intf.port), cm::fm::None);
        sleep(1);
        _console->printv("\rAfter:\n");
        _print_regs();
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

        Bali::FmEplRegs& epl = _mgr->deviceMgr(_dev).dev().epl_phy[intf.port-EPL_PORT_OFFSET];

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

static void _dump(int argc, char** argv)
{
  if (argc<2)
    _console->printv("dump: incorrect # args (%d)\n",argc);
  else {
    _console->printv("Dumping CPU port\n");
    unsigned* beg = new unsigned[1512>>2];
    int len;
    while( (len=_mgr->deviceMgr(_dev).dev().rxPacket(beg)) ) {
      for(unsigned i=0; i<len; i++)
        _console->printv("%08x%c",beg[i],(i%8)==7?'\n':' ');
      _console->printv("\n");
    }
    delete[] beg;
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
                                      { _show_glorts    ,  show_glort_args },
                                      { _show_reg       ,  show_reg_args },
                                      { _show_plx       ,  show_plx_args },
                                      { _show_log       ,  show_log_args },
                                      { _enable_port    ,  enable_args },
                                      { _disable_port   ,  disable_args },
                                      { _lanetest       ,  lanetest_args },
                                      { _dump           ,  dump_args },
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
  bool tooFew = false;

  if (lverbose) {
    _console->printv("parse_command parsing ");
    for(int i=0; i<argc; i++)
      _console->printv("[%s] ",argv[i]);
    _console->printv("\n");
  }

  for(int icmd=0; dtm_command[icmd].cmd!=NULL; icmd++) {
    for(int iarg=0; iarg<=argc; iarg++) {
      const char* arg = dtm_command[icmd].args[iarg];
      if (arg==NULL) {
//      _console->printv("parse_command executing %s\n",
//                       iarg>0?dtm_command[icmd].args[iarg-1]:"err");
        dtm_command[icmd].cmd(argc-iarg, &argv[iarg]);
        return;
      }
      if (argv[iarg]==NULL) {
        tooFew = true;
        break;
      }
      if (strcasestr(arg,argv[iarg])!=arg) {
        break;
      }
    }
  }
  _console->printv("parse error : ");
  for(int iarg=0; iarg<argc; iarg++)
    _console->printv("%s ",argv[iarg]);
  if (tooFew) {
    _console->printv("(too few arguments)");
  }
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

    if (!argc) {
      console->printv("dtm> ");
      return;
    }

    if (lverbose) {
      _console->printv("process parsing ");
      for(int i=0; i<argc; i++)
        _console->printv("[%s] ",argv[i]);
      _console->printv("\n");
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
  printf("options: -m\tLoad configuration from BSI memory\n"
         "         -M\tDo NOT load configuration from BSI memory (Default)\n"
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
/**
static void* poll_bsi(void*);
**/

int main(int argc, char** argv)
{
  //  extern char* optarg;
  //  char* endPtr;
  bool lteln=false;
  bool lreadfrombsi=false;
  bool lreset=false;
  int c;
  while ( (c=getopt( argc, argv, "mrtvh")) != EOF ) {
    switch(c) {
    case 't': lteln =true; break;
    case 'v': lverbose=true; break;
    case 'm': lreadfrombsi=true; break;
    case 'r': lreset=true; break;
    case 'h':
    case '?': 
    default : usage(argv[0]); break;
    }
  }

  //
  //  Continue about our business
  //

  DtmEnv env;

  char* p = new char[max_size];
  buff = new char[max_size];

  if (lreadfrombsi) {
    printf("%s: calling load_bsi()\n", __FILE__);
    load_bsi   (&env);
  } else {
    printf("%s: skipping load_bsi()\n", __FILE__);
  }

  cm::net::ManagerB* mgr = new(p) cm::net::ManagerB(1,&_config);

  if (lreset) {
    // initialization handled in load_bsi
    _config.swCfg.port_vlan = SwmPortVlanConfig(SwmPortVlanConfig::Init);
    mgr->deviceMgr(_dev).reset(_config);
  }

  _mgr = mgr;

  pthread_t id;
  pthread_attr_t flags;
  size_t min_stack,stacksize=0x10000;
  pthread_attr_init(&flags);
  pthread_attr_setdetachstate(&flags, PTHREAD_CREATE_DETACHED);
  pthread_attr_getstacksize(&flags, &min_stack);
  if (stacksize < min_stack) stacksize = min_stack;
  pthread_attr_setstacksize(&flags, stacksize);

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

void sigintHandler(int iSignal)
{
  if (lverbose)
    cm::service::printv("fmd caught signal %d\n",iSignal);

  /**
  save_config(".current",_mgr->deviceMgr(_dev)._config);

  _mgr->read(&_config);
  save_config(".recovered",_config);
  **/
  
  delete[] buff;
  exit(0);
}

