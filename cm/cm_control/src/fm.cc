/* Support code for configuring shell commands in an RTEM
 *
 *  Author:  <Jim Panetta: panetta@slac.stanford.edu>
 *  Created: <2009-03-24>
 *  Time-stamp: <2009-05-01 14:38:28 panetta>
 *
 *
 *
 *
*/

//#include "bldInfo.h"


#include "cm_svc/CmHw.hh"
#include "cm_svc/ChannelMap.hh"
#include "cm_svc/PlxBridge.hh"
#include "Manager.hh"
#include "fm/FmTahoeMgr.hh"
#include "fm/FmTahoe.hh"
#include "fm/FmConfig.hh"
#include "fm/FmPort.hh"
#include "fm/SwmRequest.hh"
#include "fm/SwmReply.hh"
#include "fm/SwmPortMibCounters.hh"
#include "fm/SwmEplRegs.hh"

#include <getopt.h>

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <time.h>

#define strcasestr strstr

static cm::net::Manager* _mgr = 0;
static FmConfig _config;
static int _dev = 0;

static void load_config(const char*);
static void save_config(const char*,const FmConfig&);

static char* buff = 0;

namespace cm {
namespace shell {

static int main_fm   (int argc, char** argv);

#if 0
CMD_CLASS(FM_Command,
          (char*)"fm",                          /* command name */
          (char*)"fm <subcmd...> [? for help]", /* usage */
          (char*)"switch",                      /* help topic */
          main_fm);                             /* main function */
#endif
}
}

typedef struct {
  const char** args;
  const char** lines;
} DtmHelp_t;

static const char* fm_lines[] = { "fm reset",
                                  "fm load ...",
                                  "fm save ...",
                                  "fm set ...",
                                  "fm show ...",
                                  "fm enable ...",
                                  "fm disable ...",
                                  NULL };
static const char* reset_args[] = { "reset",NULL };
static const char* reset_lines[] = {"fm reset\t(reset switch and apply configuration)",
				     NULL };

static const char* load_args[] = { "load",NULL };
static const char* load_lines[]= { "fm load <filename>\t(load configuration from <filename>)",
                                   NULL };

static const char* save_args[] = { "save",NULL };
static const char* save_lines[]= { "fm save <filename>\t(save configuration to <filename>)",
                                   NULL };

static const char* set_args [] = { "set",NULL };
static const char* set_lines[] = { "fm set base \t(set base switch as target)",
                                   "fm set fabric \t(set fabric switch as target)",
                                   "fm set register\t(set fulcrum register)",
                                   NULL };
static const char* set_reg_args [] = { "set","register",NULL };
static const char* set_reg_lines[] = { "rm set register <addr> <value> [<repeat>]", NULL };
static const char* show_args [] = { "show",NULL };
static const char* show_lines[] = { "fm show int \t(show interface status)",
                                    "fm show stat\t(show interface statistics)",
                                    "fm show macaddr\t(show mac address table)",
                                    "fm show reg\t(show register value)",
                                    "fm show plx\t(show pcie bridge status)",
                                    NULL };
static const char* show_int_args [] = { "show","int",NULL} ;
static const char* show_int_lines[] = { "fm show int brief \t(show status summary)",
                                        "fm show int <port>\t(show status for <port> [e.g. dtm0.0]",
                                        NULL };
static const char* show_int_brief_args [] = { "show","int","brief",NULL };
static const char* show_int_brief_lines[] = { show_int_lines[0],
                                              NULL };
static const char* show_int_dpm_args [] = { "show","int","dpm",NULL };
static const char* show_int_dpm_lines[] = { show_int_lines[1],
                                            NULL };
static const char* show_stat_args [] = { "show","statistics",NULL };
static const char* show_stat_lines[] = { "fm show statistics brief \t(show statistics summary)",
                                         "fm show statistics <port>\t(show detailed statistics for port)",
                                         NULL };
static const char* show_stat_brief_args [] = { "show","statistics","brief",NULL };
static const char* show_stat_brief_lines[] = { show_stat_lines[0],
                                               NULL };
static const char* show_stat_dpm_args [] = { "show","statistics","dpm",NULL };
static const char* show_stat_dpm_lines[] = { show_stat_lines[1],
                                             NULL };
static const char* show_macaddr_args [] = { "show","macaddr",NULL };
static const char* show_macaddr_lines[] = { show_lines[2],
                                            NULL };
static const char* show_reg_args [] = { "show","register",NULL };
static const char* show_reg_lines[] = { "fm show register <reg>\t(show register)",
                                        NULL };
static const char* show_plx_args [] = { "show","plx",NULL };
static const char* show_plx_lines[] = { "fm show plx\t(show pcie bridge)",
                                        NULL };

static const char* enable_args [] = { "enable", NULL };
static const char* enable_lines[] = { "fm enable <port>\t(enable port at 10Gb/s)",
                                      "fm enable <port> 1Gb/s\t(enable port at 1Gb/s)",
                                      NULL };
static const char* disable_args [] = { "disable", NULL };
static const char* disable_lines[] = { "fm disable <port>\t(disable port)",
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
                                { show_stat_dpm_args, show_stat_dpm_lines },
                                { show_macaddr_args, show_macaddr_lines },
                                { show_reg_args, show_reg_lines },
                                { show_plx_args, show_plx_lines },
                                { enable_args, enable_lines },
                                { disable_args, disable_lines },
                                { NULL, NULL } };

typedef struct {
  void (*cmd)(int, char**);
  const char** args;
} DtmCommand_t;

typedef struct {
  const char* name;
  unsigned    port;
} DtmInterface_t;


static DtmInterface_t _interfaces(unsigned i)
{
  static char* name = new char[16];

  DtmInterface_t v;
  v.name = name;
  v.port = 0;

  const cm::svc::ChannelMap* map = &_mgr->map(_dev);
  if (!map) {
    v.name = NULL;
    return v;
  }

  for(int j=0; j<32; j++) {
    int port = map->b(j);
    if (port < 0) continue;
    if (i==0) {
      v.port = port;
      if (j<16)
        sprintf(name,"slot%d",j+2);
      else if (j<24)
        sprintf(name,"dpm%d.%d",(j-16)/2,(j-16)%2);
      else if (j==28)
        sprintf(name,"fp_sfp");
      else if (j==29)
        sprintf(name,"fp_sfp+");
      else if (j==31)
        sprintf(name,"dtm");
      else
        printf("interface parse error\n");
      return v;
    }
    else
      i--;
  }

  v.name = NULL;
  return v;
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
    printf("set <switch> : too many arguments (%d)\n",argc);
  else {
    for(unsigned i=0; titles[i]!=NULL; i++)
      if (strcasecmp(argv[0],titles[i])==0) {
        if (i<_mgr->nDevices())
          _dev = i;
        return;
      }
    printf("set %s - failed\n", argv[0]);
    printf("Valid options are:");
    for(unsigned i=0; titles[i]!=NULL; i++)
      printf(" %s",titles[i]);
    printf("\n");
  }
}

static void _set_reg(int argc, char** argv)
{
  if (argc<2)
    printf("set register <addr> <value> [<repeat>] : incorrect # args (%d)\n",argc);
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
                                  NULL };
  if (argc>0)
    printf("show int brief : too many arguments (%d)\n",argc);
  else {
    const FmPortMask up_ports = _mgr->deviceMgr(_dev).portsUp();

    for(unsigned i=0; titles[i]!=NULL; i++)
      printf("%s%c",titles[i],titles[i+1] ? '\t':'\n');
    for(unsigned i=0; titles[i]!=NULL; i++)
      printf("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
    for(unsigned i=0; _interfaces(i).name!=NULL; i++) {
      printf("%-*s\t", strlen(titles[0]), _interfaces(i).name);
      printf("%*s\t" , strlen(titles[1]), (_enabled(_interfaces(i).port)) ? "T":"F");
      printf("%*s\n" , strlen(titles[2]), (up_ports.contains(FmPort(_interfaces(i).port))) ? "UP":"DOWN");
    }
  }
}

static void _show_int(int argc, char** argv)
{
  if (argc>1)
    printf("show int <port> : too many arguments (%d)\n",argc);
  else {
    for(int i=0; _interfaces(i).name!=NULL; i++) {
      if (strcasecmp(argv[0],_interfaces(i).name)==0) {
        printf("interface %s\n", argv[0]);

        char* reply = new char[0x80000];
        unsigned reply_len;
        SwmEplReadRequest* req = new SwmEplReadRequest( &_mgr->deviceMgr(_dev).dev().epl_phy[_interfaces(i).port-1] );
        _mgr->process(*req, (void*)reply, reply_len);
        delete req;

        if (!reply_len) {
          printf("Error fetching port status\n");
        }
        else {
          cm::fm::PortSpeed speed = _mgr->deviceMgr(_dev).portSpeed(FmPort(_interfaces(i).port));
          if (speed == cm::fm::None) {
            printf("\tport is disabled\n");
          }
          else {
            printf("\tport is enabled at %s\n",_speed[speed]);

            const SwmEplReadReply* r = reinterpret_cast<const SwmEplReadReply*>(reply);
            unsigned v = r->result().serdes_ip & 0xfff;
            if (v&0x924)
              printf("\t%40.40s%x\n","disparity error  lanes ",_lanes(v>>2));
            if (v&0x492)
              printf("\t%40.40s%x\n","out-of-band char lanes ",_lanes(v>>1));
            if (v&0x249)
              printf("\t%40.40s%x\n","loss of signal   lanes ",_lanes(v>>0));
            printf("\tpcs status %x\n", r->result().pcs_ip);
            printf("\tmac status %x\n", r->result().mac_ip);
          }
        }
        delete[] reply;
        return;
      }
    }
    printf("show int %s : interface not found\n",argv[0]);
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
    printf("show stat brief : too many arguments (%d)\n",argc);
  else {

    char* reply = new char[0x80000];
    unsigned reply_len;
    SwmAllPortMibRequest* req = new SwmAllPortMibRequest( &_mgr->deviceMgr(_dev).dev() );
    _mgr->process(*req, (void*)reply, reply_len);
    delete req;
    if (!reply_len) {
      printf("Error fetching port statistics\n");
      delete[] reply;
      return;
    }

    const SwmAllPortMibReply& rrep = *(const SwmAllPortMibReply*)reply;

    for(unsigned i=0; titles[i]!=NULL; i++)
      printf("%s%c",titles[i],titles[i+1] ? '\t':'\n');
    for(unsigned i=0; titles[i]!=NULL; i++)
      printf("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
    for(unsigned i=0; _interfaces(i).name!=NULL; i++) {
      const SwmPortMibCounters& c = rrep.result().port(_interfaces(i).port);
      printf("%-*s\t", strlen(titles[0]), _interfaces(i).name);
      unsigned long long rxPkts = c.rxUcast + c.rxBcast + c.rxMcast;
      printf("%*lld\t" , strlen(titles[1]), rxPkts);
      unsigned long long txPkts = c.txUnicast + c.txBroadcast + c.txMulticast;
      printf("%*lld\t" , strlen(titles[2]), txPkts);
      unsigned long long rxErrs = c.rxFcsErrors + c.rxSymbolErrors;
      printf("%*lld\t" , strlen(titles[3]), rxErrs);
      unsigned long long txErrs = c.txTimeoutDrops + c.txErrorDrops;
      printf("%*lld\n" , strlen(titles[4]), txErrs);
    }
    delete[] reply;
  }
}

static char _buff[64];

#define printStat1( var1 ) {                                            \
    printf("%30.30s = %15lld\n", # var1, (unsigned long long) c.var1);  \
  }

#define printStat( var1, var2 ) {                                       \
    printf("%30.30s = %15lld"  , # var1, (unsigned long long) c.var1);  \
    printf("%30.30s = %15lld\n", # var2, (unsigned long long) c.var2);  \
  }

#define printStats( var1, var2, len ) {                                 \
    for(unsigned j=0; j<len; j++) {                                     \
      sprintf(_buff,"%s[%d]",# var1,j);                                 \
      printf("%30.30s = %15lld  ", _buff, (unsigned long long)c.var1[j]); \
      sprintf(_buff,"%s[%d]",# var2,j);                                 \
      printf("%30.30s = %15lld\n", _buff, (unsigned long long)c.var2[j]); \
    }                                                                   \
  }

static void _show_stat(int argc, char** argv)
{
  if (argc>1)
    printf("show stat <port> : too many arguments (%d)\n",argc);
  else {
    for(int i=0; _interfaces(i).name!=NULL; i++) {
      if (strcasecmp(argv[0],_interfaces(i).name)==0) {
        printf("interface %s\n", argv[0]);

        char* reply = new char[0x80000];
        unsigned reply_len;
        SwmPortMibRequest* req = new SwmPortMibRequest( &_mgr->deviceMgr(_dev).dev().port_mib[_interfaces(i).port] );
        _mgr->process(*req, (void*)reply, reply_len);
        delete req;

        if (!reply_len) {
          printf("Error fetching port statistics\n");
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
          printf("\n");
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
    printf("show stat %s : interface not found\n",argv[0]);
  }
}

static void _show_macaddr(int argc, char** argv)
{
  if (argc>0)
    printf("macaddr: too many arguments (%d)\n",argc);
  else {
    char* reply = new char[0x80000];
    unsigned reply_len;
    SwmMacTableDirectRequest* req = new SwmMacTableDirectRequest( &_mgr->deviceMgr(_dev).dev() );
    _mgr->process(*req, (void*)reply, reply_len);
    delete req;
    
    if (!reply_len) {
      printf("Error fetching mac address table\n");
    }
    else {
      const SwmMacTableReadReply& r = *reinterpret_cast<const SwmMacTableReadReply*>(reply);
      const SwmMacTable& c = r.result();
      printf("%8.8s%20.20s%10.10s%10.10s%6.6s%6.6s%4.4s\n",
             "Entry", "MAC Address", "Ports", "Lock", "Fid", "Trig", "P");
      for(unsigned j=0; j<SwmMacTable::Entries; j++) {
        const SwmMacTableEntry& e = c.entry[j];
        if (e.valid()) {
          printf("%8.8d",j);
          char buf[32];
          sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",
                  e.addr_octet(5),
                  e.addr_octet(4),
                  e.addr_octet(3),
                  e.addr_octet(2),
                  e.addr_octet(1),
                  e.addr_octet(0));
          printf("%20.20s",buf);
          sprintf(buf,"%08x",unsigned(e.port_mask()));
          printf("%10.10s",buf);
          if (e.locked())
            printf("%10.10s","Static ");
          else
            printf("%10.10s","Dynamic ");
          printf("%6.6x %6.6x %4.4x",
                 e.learning_group(), e.trigger(), e.parity());
          printf("\n");
        }
      }
    }
  }
}

static void _show_reg(int argc, char** argv)
{
  if (argc!=1) {
    printf("show reg <reg>\n");
  }
  else {
    unsigned off = strtoul(argv[0],NULL,0);
    const uint32_t* addr = 0;
    cm::svc::PlxBridge& bridge = cm::svc::PlxBridge::instance();
    bridge.set_verbose(1);
    unsigned v = bridge.read(addr+off);
    bridge.set_verbose(0);
    printf("reg[0x%x] = %x\n",off,v);
  }
}

static void _show_plx(int argc, char** argv)
{
  printf(" *** entered %s:%s\n", __FILE__, __FUNCTION__);
  cm::svc::PlxBridge& b = cm::svc::PlxBridge::instance();
  b.dump();
}

static void _enable_port(int argc, char** argv)
{
  if (argc>2)
    printf("enable <port> (<speed>): too many arguments (%d)\n",argc);
  else {
    cm::fm::PortSpeed speed = cm::fm::None;
    if (argc==2) {
      for(int i=cm::fm::X10Mb; i<=cm::fm::X10Gb; i++)
        if (strcmp(argv[1],_speed[i])==0) {
          speed = cm::fm::PortSpeed(i);
          break;
        }
      if (speed == cm::fm::None) {
        printf("fm enable %s %s - error parsing speed %s\n",
               argv[0],argv[1],argv[1]);
        return;
      }
    }
    else
      speed = cm::fm::X10Gb;

    for(int i=0; _interfaces(i).name!=NULL; i++) {
      if (strcasecmp(argv[0],_interfaces(i).name)==0) {
        //  bring up port & wait
        _mgr->deviceMgr(_dev).configurePort(FmPort(_interfaces(i).port), speed);
        //  clear counters
        char* reply = new char[0x80000];
        unsigned reply_len;
        SwmEplReadRequest* req = new SwmEplReadRequest( &_mgr->deviceMgr(_dev).dev().epl_phy[_interfaces(i).port-1] );
        _mgr->process(*req, (void*)reply, reply_len);
        delete req;
        delete[] reply;
        return;
      }
    }
    printf("fm enable %s %s: interface %s not found\n",
           argv[0], argc>1 ? argv[1]:"", argv[0]);
  }
}

static void _disable_port(int argc, char** argv)
{
  if (argc>1)
    printf("disable <port> : too many arguments (%d)\n",argc);
  else {
    for(int i=0; _interfaces(i).name!=NULL; i++) {
      if (strcasecmp(argv[0],_interfaces(i).name)==0) {
        //  bring down port
        _mgr->deviceMgr(_dev).configurePort(FmPort(_interfaces(i).port), cm::fm::None);
        return;
      }
    }
    printf("disable %s : interface %s not found\n",argv[0],argv[0]);
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
                                      { _show_stat      ,  show_stat_args },
                                      { _show_macaddr   ,  show_macaddr_args },
                                      { _show_reg       ,  show_reg_args },
                                      { _show_plx       ,  show_plx_args },
                                      { _enable_port    ,  enable_args },
                                      { _disable_port   ,  disable_args },
                                      { NULL, NULL } };

static void parse_help(int argc, char** argv)
{
  if (argc==0) {
    for(int iline=0; fm_lines[iline]!=NULL; iline++)
      printf("%s\n",fm_lines[iline]);
    return;
  }
  for(int icmd=0; dtm_help[icmd].args!=NULL; icmd++) {
    for(int iarg=0; iarg<argc; ) {
      const char* arg = dtm_help[icmd].args[iarg];
      if (arg==NULL || strcasestr(arg,argv[iarg])!=arg)
        break;
      if (dtm_help[icmd].args[++iarg]==NULL && iarg==argc) { // complete match
        for(int iline=0; dtm_help[icmd].lines[iline]!=NULL; iline++)
          printf("%s\n",dtm_help[icmd].lines[iline]);
        return;
      }
    }
  }
}

static void parse_command(int argc, char** argv)
{
  for(int icmd=0; dtm_command[icmd].cmd!=NULL; icmd++) {
    for(int iarg=0; iarg<=argc; iarg++) {
      const char* arg = dtm_command[icmd].args[iarg];
      if (arg==NULL) {
        printf("parse_command executing %s\n",iarg>0?dtm_command[icmd].args[iarg-1]:"err");
        dtm_command[icmd].cmd(argc-iarg, &argv[iarg]);
        return;
      }
      if (strcasestr(arg,argv[iarg])!=arg)
        break;
    }
  }
  printf("fm parse error : ");
  for(int iarg=0; iarg<argc; iarg++)
    printf("%s ",argv[iarg]);
  printf("\n");
}

static const size_t max_size = 0x100000;

int cm::shell::main_fm(int argc, char** argv)
{
  argc--;
  argv++;

  char* last = argc > 0 ? argv[argc - 1] : 0;

  if (argc > 0 && *(last + strlen(last) - 1)=='?') {
    *(last + strlen(last) -1) = 0;  // strip off the '?'
    if (strlen(last)==0)            // remove last word if empty
      argc--;
    parse_help(argc, argv);
  }
  else {
    buff = new char[max_size];
    const char* cfgname = ".current";
    load_config(cfgname);
    _mgr = new cm::net::Manager(&_config);

    parse_command(argc, argv);

//    save_config(".current",_mgr->deviceMgr(_dev)._config);
    delete[] buff;
  }
  return 0;
}

void load_config(const char* fname)
{
  FILE* f = fopen(fname,"r");
  if (f) {
    size_t nb = fread(buff, 1, max_size, f);
    printf("Read %d bytes from configuration .current\n",nb);
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
    printf("Wrote %d bytes to configuration .current\n",nb);
    fclose(f);
  }
}

int main(int argc, char** argv)
{
  return cm::shell::main_fm(argc,argv); 
}
