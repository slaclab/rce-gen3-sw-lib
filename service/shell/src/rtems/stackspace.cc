
// #include <inttypes.h>

// #include <rtems.h>
// #include <rtems/malloc.h>
// #include <rtems/score/protectedheap.h>
// #include <rtems/score/wkspace.h>
// #include "internal.h"

#include <stdio.h>
#include <string.h>

#include <rtems.h>

#include "memory/mem.h"

#include "shell/Command.h"

#define CMD_NAME  "stackspace"
#define CMD_TOPIC "rtems"

namespace {
  void printInfo(const char *c, Heap_Information *h) {
    printf(
           "Number of %s blocks: %lu\n"
           "Largest %s block:    %lu\n"
           "Total bytes %s:      %lu\n",
           c, (unsigned long)h->number,
           c, (unsigned long)h->largest,
           c, (unsigned long)h->total
           );
  }
}

namespace service {
  namespace shell {
    namespace stackspace {

      const char* const usage = "Report the space usage in the Heap used for task stacks";

      int main(__attribute__((unused)) int argc,
               __attribute__((unused)) const char* argv[])
      {
        Heap_Information_block info;
        mem_getStackHeapInfo(&info);
        printInfo("free", &info.Free);
        printInfo("used", &info.Used);
        return 0;
      }
    }
  }
}

SHELL_COMMAND(STACKSPACE_Cmd, CMD_NAME, service::shell::stackspace::usage, CMD_TOPIC, service::shell::stackspace::main);
