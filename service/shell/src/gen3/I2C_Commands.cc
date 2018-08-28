
#include "shell/os/BaseCommands.hh"

#include <rtems/shell.h>

#include <string>
using std::string;

#define __need_getopt_newlib
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

namespace service {
namespace shell {

static char qwbuf[32];
static unsigned* _qwbuf = (unsigned*)((unsigned(qwbuf)+0xf)&~0xf);

static string i2cUsageLong = "Usage:\n"					\
  "  i2c [-r]|[-w <string>] [-h]\n"					\
  "  I2C Master Commands.\n"						\
  "  Arguments: \n"							\
  "  -r read an I2C transaction result\n"				\
  "  -w <string> generate an i2C transaction with a string of the format\n" \
  "     \"<N> <byte0> ... <byteN-1>\"\n";

static string i2cUsage = "i2c [-r]|[-w <string>] [-h]";

static int i2c_main(int argc, char **argv) {
  int i;
  for (i=1; i<argc; i++) {
    if (strcmp(argv[i],"-r")==0) {
      int ncmds = atoi(argv[++i]);
      char buff[256];
      char* p = buff;
      p += sprintf(p,"Read I2C Cmds[%d]:",ncmds);
      for(int i=0; i<ncmds; i++) {
	uint8_t* wbuf = reinterpret_cast<uint8_t*>(_qwbuf);
	asm volatile("stqfcmx 0,%0,%1" : : "r"(0), "r"(wbuf));

	unsigned nbyt = wbuf[15^3];
	p += sprintf(p,"(%02x)",nbyt);

	for(unsigned i=0; i<nbyt; i++)
	  p += sprintf(p," %02x",wbuf[i^3]);

	p += strlen(p);
      }
      printf("%s\n",buff);
    }
    else if (strcmp(argv[i],"-w")==0) {
      uint8_t* cmd = reinterpret_cast<uint8_t*>(_qwbuf);

      uint8_t nbyt;
      char* args = argv[++i];
      printf("arg0 %s\n",args);

      if (!sscanf(args,"%hhx",&nbyt))
	return 2;

      for(int j=0; j<nbyt; j++) {
	args = argv[++i];
	printf("arg%d %s\n",j+1,args);
	if (!sscanf(args,"%hhx",&cmd[j^3]))
	  continue;
      }

      cmd[15^3] = nbyt;

      printf("%08x %08x %08x %08x\n",
	     _qwbuf[0],_qwbuf[1],_qwbuf[2],_qwbuf[3]);

      //  I suspect that the PPC405 errata describes a problem I've
      //  seen where the APU can generate neither a WRITEBACKOK nor
      //  FLUSH signal (hanging the FCM). Described software fixes
      //  use a "nop" instruction before/after the "lqfcmx".
      asm volatile("nop");
      asm volatile("lqfcmx 0,%0,%1" : : "r"(0), "r"(cmd));
      asm volatile("nop");
    }
    else {
      printf(i2cUsageLong.c_str());
      return 2;
    }
  }
  return 0;
}

std::string I2C::usage() {
  return i2cUsage;
}

service::shell::Command::MainFunc I2C::main() {
  return i2c_main;
}

}
}
