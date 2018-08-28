//
//  Switch commands from the shell
//
//   fm show int brief
//   fm show int dpm 1
//   fm show stat brief
//   fm show stat dpm 1
//
//   fm enable dpm 1
//   fm disable dpm 1
//
#include "bldInfo.h"

#include "control/DtmShell.hh"
#include "shell/Command.hh"

#include "cm_svc/CmHw.hh"
#include "net/Manager.hh"
#include "fm/FmTahoeMgr.hh"
#include "fm/FmTahoe.hh"
#include "fm/FmConfig.hh"
#include "fm/FmPort.hh"
#include "fm/SwmRequest.hh"
#include "fm/SwmReply.hh"
#include "fm/SwmPortMibCounters.hh"
#include "fm/SwmEplRegs.hh"

#define __need_getopt_newlib
#include <getopt.h>

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <time.h>

#define strcasestr strstr

namespace cm {
namespace shell {

static cm::net::Manager* _mgr = 0;

static int main_vsn  (int argc, char** argv);
static int main_fm   (int argc, char** argv);
static int main_iicrr(int argc, char** argv);
static int main_iicrw(int argc, char** argv);
static int main_iic_raddr(int argc, char** argv);
static int main_iic_rresponse(int argc, char** argv);
static int main_iic_wresponse(int argc, char** argv);
static int main_iicdump (int argc, char** argv);
static int main_iicdebug(int argc, char** argv);

CMD_CLASS(VSN_Command,
          (char*)"vsn",                         /* command name */
          (char*)"vsn",                         /* usage */
          (char*)"switch",                      /* help topic */
          main_vsn);                            /* main function */

CMD_CLASS(FM_Command,
          (char*)"fm",                          /* command name */
          (char*)"fm <subcmd...> [? for help]", /* usage */
          (char*)"switch",                      /* help topic */
          main_fm);                             /* main function */

CMD_CLASS(IIC_RR_Command,
          (char*)"iicrr",                       /* command name */
          (char*)"iicrr <addr>",                /* usage */
          (char*)"iic",                         /* help topic */
          main_iicrr);                          /* main function */

CMD_CLASS(IIC_RW_Command,
          (char*)"iicrw",                       /* command name */
          (char*)"iicrw <addr> <value>",        /* usage */
          (char*)"iic",                         /* help topic */
          main_iicrw);                          /* main function */

CMD_CLASS(IIC_RADDR_Command,
          (char*)"iicraddr",                    /* command name */
          (char*)"iicraddr",                    /* usage */
          (char*)"iic",                         /* help topic */
          main_iic_raddr);                      /* main function */

CMD_CLASS(IIC_RRESP_Command,
          (char*)"iicrresp",                    /* command name */
          (char*)"iicrresp",                    /* usage */
          (char*)"iic",                         /* help topic */
          main_iic_rresponse);                  /* main function */

CMD_CLASS(IIC_WRESP_Command,
          (char*)"iicwresp",                    /* command name */
          (char*)"iicwresp <value>",            /* usage */
          (char*)"iic",                         /* help topic */
          main_iic_wresponse);                  /* main function */

CMD_CLASS(IIC_DUMP_Command,
          (char*)"iicdump",                     /* command name */
          (char*)"iicdump",                     /* usage */
          (char*)"iic",                         /* help topic */
          main_iicdump);                        /* main function */

CMD_CLASS(IIC_DEBUG_Command,
          (char*)"iicdebug",                    /* command name */
          (char*)"iicdebug <n>",                /* usage */
          (char*)"iic",                         /* help topic */
          main_iicdebug);                       /* main function */


void addCommands(cm::net::Manager* mgr)
{
  _mgr = mgr;

  VSN_Command        vsn;            vsn.initialize();
  FM_Command         fm;             fm.initialize();

  IIC_RR_Command     iicRr;          iicRr.initialize();
  IIC_RW_Command     iicRw;          iicRw.initialize();
  IIC_RADDR_Command  iic_rAddr;      iic_rAddr.initialize();
  IIC_RRESP_Command  iic_rResponse;  iic_rResponse.initialize();
  IIC_WRESP_Command  iic_wResponse;  iic_wResponse.initialize();
  IIC_DUMP_Command   iicDump;        iicDump.initialize();
  IIC_DEBUG_Command  iicDebug;       iicDebug.initialize();
}

typedef struct {
  const char** args;
  const char** lines;
} DtmHelp_t;

static const char* fm_lines[] = { "fm show ...",
				  NULL };
static const char* show_args [] = { "show",NULL };
static const char* show_lines[] = { "fm show int \t(show interface status)",
				    "fm show stat\t(show interface statistics)",
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
static const char* enable_args [] = { "enable", NULL };
static const char* enable_lines[] = { "fm enable <port>\t(enable port at 10Gb/s)",
				      "fm enable <port> 1Gb/s\t(enable port at 1Gb/s)",
				      NULL };
static const char* disable_args [] = { "disable", NULL };
static const char* disable_lines[] = { "fm disable <port>\t(enable port)",
				      NULL };

static DtmHelp_t dtm_help[] = { { show_args, show_lines },
				{ show_int_args, show_int_lines },
				{ show_int_brief_args, show_int_brief_lines },
				{ show_int_dpm_args, show_int_dpm_lines },
				{ show_stat_args, show_stat_lines },
				{ show_stat_brief_args, show_stat_brief_lines },
				{ show_stat_dpm_args, show_stat_dpm_lines },
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

static DtmInterface_t _interfaces[] = { { "dpm0.0",  3 },
					{ "dpm0.1",  5 },
					{ "dpm1.0", 11 },
					{ "dpm1.1", 15 },
					{ "dpm2.0",  2 },
					{ "dpm2.1",  1 },
					{ "dpm3.0",  7 },
					{ "dpm3.1",  9 },
					{ "rtm0"  , 13 },
					{ "rtm1"  , 19 },
					{ "fab1"  ,  8 },
					{ "fab2"  ,  6 },
					{ "fab3"  , 12 },
					{ "fab4"  , 10 },
					{ "fab5"  , 14 },
					{ "fab6"  , 20 },
					{ "fab7"  , 22 },
					{ "fab8"  , 16 },
					{ "fab9"  , 24 },
					{ "fab10" , 18 },
					{ "fab11" , 23 },
					{ "fab12" , 17 },
					{ "fab13" , 21 },
					{ "dtm"   ,  4 },
					{ NULL, 0 } };

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
  return _mgr->deviceMgr(0).portSpeed(FmPort(port))!=cm::fm::None;
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
    const FmPortMask up_ports = _mgr->deviceMgr(0).portsUp();

    for(unsigned i=0; titles[i]!=NULL; i++)
      printf("%s%c",titles[i],titles[i+1] ? '\t':'\n');
    for(unsigned i=0; titles[i]!=NULL; i++)
      printf("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
    for(unsigned i=0; _interfaces[i].name!=NULL; i++) {
      printf("%-*s\t", strlen(titles[0]), _interfaces[i].name);
      printf("%*s\t" , strlen(titles[1]), (_enabled(_interfaces[i].port)) ? "T":"F");
      printf("%*s\n" , strlen(titles[2]), (up_ports.contains(FmPort(_interfaces[i].port))) ? "UP":"DOWN");
    }
  }
}

static void _show_int(int argc, char** argv)
{
  if (argc>1)
    printf("show int <port> : too many arguments (%d)\n",argc);
  else {
    for(int i=0; _interfaces[i].name!=NULL; i++) {
      if (strcasecmp(argv[0],_interfaces[i].name)==0) {
	printf("interface %s\n", argv[0]);

	char* reply = new char[0x80000];
	unsigned reply_len;
	SwmEplReadRequest* req = new SwmEplReadRequest( &_mgr->deviceMgr(0).dev().epl_phy[_interfaces[i].port-1] );
	_mgr->process(*req, (void*)reply, reply_len);
	delete req;

	if (!reply_len) {
	  printf("Error fetching port status\n");
	}
	else {
          cm::fm::PortSpeed speed = _mgr->deviceMgr(0).portSpeed(FmPort(_interfaces[i].port));
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
    SwmAllPortMibRequest* req = new SwmAllPortMibRequest( &_mgr->deviceMgr(0).dev() );
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
    for(unsigned i=0; _interfaces[i].name!=NULL; i++) {
      const SwmPortMibCounters& c = rrep.result().port(_interfaces[i].port);
      printf("%-*s\t", strlen(titles[0]), _interfaces[i].name);
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

#define printStat1( var1 ) {						\
    printf("%30.30s = %15lld\n", # var1, (unsigned long long) c.var1);	\
  }

#define printStat( var1, var2 ) {					\
    printf("%30.30s = %15lld"  , # var1, (unsigned long long) c.var1);	\
    printf("%30.30s = %15lld\n", # var2, (unsigned long long) c.var2);	\
  }

#define printStats( var1, var2, len ) {					\
    for(unsigned j=0; j<len; j++) {					\
      sprintf(_buff,"%s[%d]",# var1,j);					\
      printf("%30.30s = %15lld  ", _buff, (unsigned long long)c.var1[j]); \
      sprintf(_buff,"%s[%d]",# var2,j);					\
      printf("%30.30s = %15lld\n", _buff, (unsigned long long)c.var2[j]); \
    }									\
  }

static void _show_stat(int argc, char** argv)
{
  if (argc>1)
    printf("show stat <port> : too many arguments (%d)\n",argc);
  else {
    for(int i=0; _interfaces[i].name!=NULL; i++) {
      if (strcasecmp(argv[0],_interfaces[i].name)==0) {
	printf("interface %s\n", argv[0]);

	char* reply = new char[0x80000];
	unsigned reply_len;
	SwmPortMibRequest* req = new SwmPortMibRequest( &_mgr->deviceMgr(0).dev().port_mib[_interfaces[i].port] );
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

    for(int i=0; _interfaces[i].name!=NULL; i++) {
      if (strcasecmp(argv[0],_interfaces[i].name)==0) {
	//  bring up port & wait
	_mgr->deviceMgr(0).configurePort(FmPort(_interfaces[i].port), speed);
	//  clear counters
	char* reply = new char[0x80000];
	unsigned reply_len;
	SwmEplReadRequest* req = new SwmEplReadRequest( &_mgr->deviceMgr(0).dev().epl_phy[_interfaces[i].port-1] );
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
    for(int i=0; _interfaces[i].name!=NULL; i++) {
      if (strcasecmp(argv[0],_interfaces[i].name)==0) {
	//  bring down port
	_mgr->deviceMgr(0).configurePort(FmPort(_interfaces[i].port), cm::fm::None);
	return;
      }
    }
    printf("disable %s : interface %s not found\n",argv[0],argv[0]);
  }
}

static DtmCommand_t dtm_command[] = {
				      { _show_int_brief ,  show_int_brief_args },
				      { _show_int       ,  show_int_args },
				      { _show_stat_brief,  show_stat_brief_args },
				      { _show_stat      ,  show_stat_args },
				      { _enable_port    ,  enable_args },
				      { _disable_port   ,  disable_args },
				      { NULL, NULL } };
/* { _show_int_dpm  ,  {"show","int" ,"dpm"  ,NULL} },
   { _show_stat_brief, {"show","stat","brief",NULL} },
   { _show_stat_dpm  , {"show","stat","dpm"  ,NULL} }, */

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

int main_vsn(int argc, char** argv)
{
  argc--;
  argv++;

  printf("HW Version: %s\n",cm::svc::CmHw::get_version_tag());
  time_t vsntim = cm::svc::CmHw::get_version_time();
  printf("HW Build time: %s\n",ctime(&vsntim));
  printf("SW Version: %s\n",cm::control::bldRevision());
  printf("SW Build time: %s\n",cm::control::bldDate());

  return 0;
}

int main_fm(int argc, char** argv)
{
  argc--;
  argv++;

  char* last = argv[argc - 1];

  if (*(last + strlen(last) - 1)=='?') {
    *(last + strlen(last) -1) = 0;  // strip off the '?'
    if (strlen(last)==0)            // remove last word if empty
      argc--;
    parse_help(argc, argv);
  }
  else
    parse_command(argc, argv);

  return 0;
}


static const unsigned IIC_REG0     = 0x218; // ram address
static const unsigned IIC_REG1     = 0x219; // ram data
static const unsigned IIC_REG2     = 0x21A; // last address of i2c write
static const unsigned IIC_REG3     = 0x21B; // response word

#if 0
int main_iicrr(int argc, char** argv)
{
  argc--;
  argv++;

  unsigned addr = strtoul(*argv,NULL,0);
  unsigned v;

  asm volatile("mtdcr %0, %1" : : "i"(IIC_REG0), "r"(addr));
  asm volatile("eieio");
  asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(IIC_REG1));

  printf("IIC_REG[0x%x] = 0x%x\n", addr,v);
  return 0;
}

int main_iicrw(int argc, char** argv)
{
  argc--;
  argv++;

  if (argc < 2) {
    printf("requires two arguments {address and value}\n");
    return -1;
  }

  unsigned addr = strtoul(*argv,NULL,0);
  argv++;
  unsigned v    = strtoul(*argv,NULL,0);;
  argv++;

  asm volatile("mtdcr %0, %1" : : "i"(IIC_REG0), "r"(addr));
  asm volatile("eieio");
  asm volatile("mtdcr %0, %1" : : "i"(IIC_REG1), "r"(v));
  asm volatile("eieio");

  printf("IIC_REG[0x%x] = 0x%x\n",addr,v);
  return 0;
}

int main_iic_raddr(int argc, char** argv)
{
  argc--;
  argv++;

  unsigned v;
  asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(IIC_REG2));

  printf("IIC_ADDR = 0x%x\n",v);
  return 0;
}

int main_iic_rresponse(int argc, char** argv)
{
  argc--;
  argv++;

  unsigned v;

  asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(IIC_REG3));

  printf("IIC_RESPONSE = 0x%x\n",v);
  return 0;
}

int main_iic_wresponse(int argc, char** argv)
{
  argc--;
  argv++;

  unsigned v = strtoul(*argv,NULL,0);

  asm volatile("mtdcr %0, %1" : : "i"(IIC_REG3), "r"(v));
  asm volatile("eieio");

  return 0;
}

#else

int main_iicrr(int argc, char** argv)
{
  argc--;
  argv++;

  unsigned addr = strtoul(*argv,NULL,0);
  unsigned len = 1;
  if (argc) {
    argc--;
    argv++;
    len = strtoul(*argv,NULL,0);
  }

  unsigned v,w=0;

  for(unsigned i=0; i<len; i++) {
    asm volatile("udi0fcm %0,%1,%2" : "=r"(v) : "r"(addr), "r"(w));
    asm volatile("eieio");
    printf("IIC_REG[0x%x] = 0x%x\n", addr,v);
    addr++;
  }

  return 0;
}

int main_iicrw(int argc, char** argv)
{
  argc--;
  argv++;

  if (argc < 2) {
    printf("requires two arguments {address and value}\n");
    return -1;
  }

  unsigned addr = strtoul(*argv,NULL,0);
  argv++;
  unsigned v    = strtoul(*argv,NULL,0);
  argv++;
  unsigned w;

  asm volatile("udi1fcm %0,%1,%2" : "=r"(w) : "r"(addr), "r"(v));

  printf("IIC_REG[0x%x] = 0x%x\n",addr,v);
  return 0;
}

int main_iic_raddr(int argc, char** argv)
{
  argc--;
  argv++;

  unsigned v,w=0;
  unsigned addr(0x1fe);
  asm volatile("udi0fcm %0,%1,%2" : "=r"(v) : "r"(addr), "r"(w));

  printf("IIC_ADDR = 0x%x\n",v);
  return 0;
}

int main_iic_rresponse(int argc, char** argv)
{
  argc--;
  argv++;

  unsigned v,w=0;

  unsigned addr(0x1ff);
  asm volatile("udi0fcm %0,%1,%2" : "=r"(v) : "r"(addr), "r"(w));

  printf("IIC_RESPONSE = 0x%x\n",v);
  return 0;
}

int main_iic_wresponse(int argc, char** argv)
{
  argc--;
  argv++;

  unsigned addr(0x1ff);
  unsigned v = strtoul(*argv,NULL,0);
  unsigned w;

  asm volatile("udi1fcm %0,%1,%2" : "=r"(w) : "r"(addr), "r"(v));

  return 0;
}

#endif

static const unsigned IIC_CAPTURE = 0x21C;

class I2CBus {
public:
  I2CBus() : _buffer(new char[256]), _p(_buffer), _addr(0)
  {
    unsigned v0, v1;
    unsigned addr = 0;
    asm volatile("mtdcr %0, %1" : : "i"(IIC_CAPTURE), "r"(addr));
    asm volatile("eieio");
    asm volatile("mfdcr %0, %1" : "=r"(v0) : "i"(IIC_CAPTURE));
    addr = 1;
    asm volatile("mtdcr %0, %1" : : "i"(IIC_CAPTURE), "r"(addr));
    asm volatile("eieio");
    asm volatile("mfdcr %0, %1" : "=r"(v1) : "i"(IIC_CAPTURE));
    _data = v1;
    _data <<= 32;
    _data |= v0;
  }
private:
  void _load  ()
  {
    unsigned addr = (_addr>>5)+1;
    asm volatile("mtdcr %0, %1" : : "i"(IIC_CAPTURE), "r"(addr));
    asm volatile("eieio");
    unsigned v;
    asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(IIC_CAPTURE));

    uint64_t data = v;
    data <<= 32;
    _data = (data) | (_data>>32);
  }

  unsigned _read(unsigned nb)
  {
    if (!nb) return 0;

    // swap the bit order (I2C is MSb first)
    unsigned v = 0;
    for(unsigned i=0; i<nb; i++)
      v |= ((_data >> ((_addr&0x1f)+i)) & 1) << (nb-i-1);

    _addr += nb;
    if ((_addr&0x1f)<nb)
      _load();

    return v;
  }

  void _insert  (unsigned addr)
  {
    // try to move 8 bits followed by an AK/NAK
    while(addr > _addr+8) {
      _p += sprintf(_p," %02X",_read(8));
      if (addr > _addr)
        _p += sprintf(_p,"%s",_read(1) ? " NAK":" AK");
    }
    // move remainder
    if (addr > _addr) {
      unsigned v = addr-_addr;
      _p += sprintf(_p," %02X(%d)",_read(v),v);
    }
  }
  void _append(const char* s) { _p += sprintf(_p,"%s",s); }
  void _dump  () { printf("%s\n",_buffer); _p=_buffer; }
public:
  void start(unsigned addr) { _insert(addr); _append(" S"); }
  void stop (unsigned addr) { _insert(addr); _append(" P"); _dump(); }
private:
  char* _buffer;
  char* _p;
  unsigned _addr;
  uint64_t _data;
};

int main_iicdump(int, char**)
{
  static const unsigned START_BIT = 15;
  static const unsigned STOP_BIT  = 14;
  static const unsigned ADDR_MASK = (1<<STOP_BIT)-1;

  // retrieve list of starts and stops
  std::list<unsigned> cmds;
  unsigned addr=0x80000000;
  while(1) {
    asm volatile("mtdcr %0, %1" : : "i"(IIC_CAPTURE), "r"(addr));
    asm volatile("eieio");
    unsigned v;
    asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(IIC_CAPTURE));
    if (v & ((1<<START_BIT) | (1<<STOP_BIT)))
      cmds.push_back(v);
    else
      break;

    addr++;
  }

  I2CBus i2c_bus;

  for(std::list<unsigned>::const_iterator it=cmds.begin(); it!=cmds.end(); it++) {
    if (*it & (1<<START_BIT))
      i2c_bus.start( (*it)&ADDR_MASK );
    else if (*it & (1<<STOP_BIT)) {
      i2c_bus.stop ( (*it)&ADDR_MASK );
    }
  }

  return 0;
}


static const unsigned IIC_DEBUG = 0x21D;

class StateDebug {
public:
  const char* state(unsigned v) const { return _states[ (v>>_start)&((1<<(_stop-_start))-1) ]; }
public:
  const char*  _name;
  const char** _states;
  unsigned     _start;
  unsigned     _stop;
};

static const char* I2C_STATES [] = { "RESET", "ENABLED", "WAITCF", "READSTAT", "RWDATA", "WAITNCF" };
static const char* I2CB_STATES[] = { "INIT", "REGADDR1", "REGADDR2", "REGDATAR", "REGDATAW" };
static const char* UCB_STATES [] = { "IDLE", "WRITE_AST", "WRITE_ACK", "READ_AST", "READ_ACK" };
static const char* WR_STATES  [] = { "IDLE", "WRITING", "ADDR1", "ADDR2", "RESP", "AST" };

static StateDebug _states[] =
  { { "I2C",     I2C_STATES , 0, 3 },
    { "I2CBYTE", I2CB_STATES, 3, 6 },
    { "UCBUS",   UCB_STATES , 6, 9 },
    { "WRSTATE", WR_STATES  , 9, 12} };

int main_iicdebug(int argc, char** argv)
{
  argc--;
  argv++;

  printf("addr  ");
  for(int i=0; i<4; i++)
    printf("%12.12s ", _states[i]._name);
  printf("  time \n\n");

  unsigned v;
  asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(IIC_DEBUG));

  unsigned end_addr = v>>23;
  unsigned addr = end_addr - strtoul(*argv,NULL,0);
  addr &= (1<<9)-1;

  do {
    asm volatile("mtdcr %0, %1" : : "i"(IIC_DEBUG), "r"(addr));
    asm volatile("eieio");
    asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(IIC_DEBUG));

    printf("%04x  ",addr);
    for(int i=0; i<4; i++)
      printf("%12.12s ", _states[i].state(v));
    printf("  %05x  [%08x]\n", (v>>12) & ((1<<11)-1), v);

    addr = (addr+1)&((1<<9)-1);
  } while (addr != end_addr);

  return 0;
}

}
}
