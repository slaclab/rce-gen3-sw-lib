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

#include <string.h>
#include <stdio.h>

typedef struct {
  const char** args;
  const char** lines;
} DtmHelp_t;

static unsigned _enabled_ports = 0x5a;
static unsigned _up_ports = 0x18;

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

static DtmHelp_t dtm_help[] = { { show_args, show_lines },
				{ show_int_args, show_int_lines },
				{ show_int_brief_args, show_int_brief_lines },
				{ show_int_dpm_args, show_int_dpm_lines },
				{ show_stat_args, show_stat_lines },
				{ show_stat_brief_args, show_stat_brief_lines },
				{ show_stat_dpm_args, show_stat_dpm_lines },
				{ NULL, NULL } };

typedef struct {
  void (*cmd)(int, char**);
  const char** args;
} DtmCommand_t;

typedef struct {
  const char* name;
  unsigned    port;
} DtmInterface_t;

static DtmInterface_t _interfaces[] = { { "dpm0.0", 3 },
					{ "dpm0.1", 5 },
					{ "dpm1.0", 11 },
					{ "dpm1.1", 15 },
					{ "dpm2.0", 2 },
					{ "dpm2.1", 1 },
					{ "dpm3.0", 7 },
					{ "dpm3.1", 9 },
					{ NULL, 0 } };

static const char* line_sep = "---------------------";

static void _show_int_brief(int argc, char**)
{
  static const char* titles[] = { "interface",
				  "enabled",
				  "status",
				  NULL };
  if (argc>0)
    printf("show int brief : too many arguments (%d)\n",argc);
  else {

    unsigned up = _up_ports;

    for(unsigned i=0; titles[i]!=NULL; i++)
      printf("%s%c",titles[i],titles[i+1] ? '\t':'\n');
    for(unsigned i=0; titles[i]!=NULL; i++)
      printf("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
    for(unsigned i=0; _interfaces[i].name!=NULL; i++) {
      printf("%-*s\t", strlen(titles[0]), _interfaces[i].name);
      printf("%*s\t" , strlen(titles[1]), (_enabled_ports & 1<<(_interfaces[i].port)) ? "T":"F");
      printf("%*s\n" , strlen(titles[2]), (_up_ports      & 1<<(_interfaces[i].port)) ? "UP":"DOWN");
    }
  }
}

static void show_stat_brief(int argc, char**)
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
    int reply_len;
    SwmAllPortMibRequest* req = new SwmAllPortMibRequest( t );
    _mgr->process(*req, (void*)reply, reply_len);
    delete req;
    if (!reply_len) {
      printf("Error fetching port statistics\n");
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
      printf("%*%lld\t" , strlen(titles[1]), rxPkts);
      unsigned long long txPkts = c.txUnicast + c.txBroadcast + c.txMulticast;
      printf("%*lld\t" , strlen(titles[2]), txPkts);
      unsigned long long rxErrs = c.rxFcsErrors + c.rxSymbolErrors;
      printf("%*lld\t" , strlen(titles[2]), rxErrs);
      unsigned long long txErrs = c.txTimeoutDrops + c.txErrorDrops;
      printf("%*lld\n" , strlen(titles[2]), txErrs);
    }
  }
}

static const char* _show_int_brief_args[] = {"show","int" ,"brief",NULL};
static DtmCommand_t dtm_command[] = { { _show_int_brief,  _show_int_brief_args },
				      { NULL, NULL } };
/* { _show_int_dpm  ,  {"show","int" ,"dpm"  ,NULL} },
   { _show_stat_brief, {"show","stat","brief",NULL} },
   { _show_stat_dpm  , {"show","stat","dpm"  ,NULL} }, */

static void parse_help(int argc, char** argv)
{
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
  printf("parse error : ");
  for(int iarg=0; iarg<argc; iarg++)
    printf("%s ",argv[iarg]);
  printf("\n");
}

int main(int argc, char** argv)
{
  argc--;
  argv++;

  char* last = argv[argc - 1];

  if (*(last + strlen(last) - 1)=='?') {
    *(last + strlen(last) -1) = 0;
    parse_help(argc, argv);
  }
  else
    parse_command(argc, argv);

  return 0;
}
