// -*-Mode: C++;-*-
//
//                           Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include <rtems.h>
#include <unistd.h>
#include <stdio.h>

#include "dslutil/RebootSpace.hh"
#include "dslutil/RebootBase.hh"
#include "dslutil/Base.h"
#include "cpu/cpu.h"
#include "shell/bootmode.h"

#include <rtems/libio.h>
#include  <errno.h>
#include "system/fs.h"
#include "debug/print.h"
#include "logger/Logger.hh"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"

#define DEFAULT_RESET_LEVEL 0x2

extern "C" {
  int rtems_shell_main_cp(int argc, char *argv[]);
}

namespace service {
  namespace dslutil {
    namespace rebooter {

      using service::logger::Logger;
      
      void Space::_reboot(uint32_t dest_os, uint32_t load_bit)
      {
        const char *argv[2] = {NULL,NULL};
        int result;
        unsigned level __attribute__((unused));

        if(dest_os == OS_RTEMS)
          argv[0] = RTEMS;
        else if(dest_os == OS_LINUX)
          argv[0] = LINUX;
        else if(dest_os == OS_RESCUE)
          argv[0] = RAMDISK;
      
        if(!load_bit)
          argv[1] = LOADBIT0;
        else if(load_bit == 1)
          argv[1] = LOADBIT1;
        
        if(argv[0] || argv[1])
          {
          int status = set_bootmode(argv[0], argv[1]);
          if (status) {
            Logger().error("Got a bad status from set_bootmode: %d", status);
            return;
            }
          }
        
        sync();
        
        /* unmount the rtems application partition */
        unmount(SD_MOUNT_POINT_RTEMSAPP);
       
        /* unmount the rtems partition */
        unmount(SD_MOUNT_POINT_RTEMS);

        /* unmount the scratch partition */
        unmount(SD_MOUNT_POINT_SCRATCH);
           
        Bsi bsi = LookupBsi();    
        if (!bsi)
          {
            printf("failure mapping bsi\n");
            return;
          }

        BsiWrite32(bsi,BSI_BOOT_RESPONSE_OFFSET,BSI_BOOT_RESPONSE_REBOOTING);        
           
        rtems_interrupt_disable(level);
        
        TOOL_CPU_reset(DEFAULT_RESET_LEVEL);
      }
    }
  }
}
