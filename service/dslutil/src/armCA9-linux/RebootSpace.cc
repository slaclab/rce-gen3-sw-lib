// -*-Mode: C++;-*-
//
//                           Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>

#include "dslutil/Base.h"
#include "dslutil/RebootSpace.hh"
#include "logger/Logger.hh"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"

namespace service {
  namespace dslutil {
    namespace rebooter {
      using service::logger::Logger;
  
      const char REBOOT[]   = "/usr/bin/reboot_rce";
      const char RTEMS[]    = " -t rtems";
      const char LINUX[]    = " -t linux";
      const char RAMDISK[]  = " -t ramdisk";
      const char LOADBIT0[] = " -b 0";
      const char LOADBIT1[] = " -b 1";
    
      void Space::_reboot(uint32_t dest_os, uint32_t load_bit)
      {
        char buffer[64];
        
        strcpy(buffer,REBOOT);
    
        if(dest_os == OS_RTEMS)          
          strcat(buffer,RTEMS);
        else if(dest_os == OS_LINUX)
          strcat(buffer,LINUX);
        else if(dest_os == OS_RESCUE)
          strcat(buffer,RAMDISK);
        
        if(load_bit == 0)
          strcat(buffer,LOADBIT0);
        else if(load_bit == 1)
          strcat(buffer,LOADBIT1);
      
        Bsi bsi = LookupBsi();    
        if (!bsi)
          {
            printf("failure mapping bsi\n");
            return;
          }

        BsiWrite32(bsi,BSI_BOOT_RESPONSE_OFFSET,BSI_BOOT_RESPONSE_REBOOTING);        
      
        /* Execute the reboot_rce *program* using a system call. This shuts
           down all linux services cleanly prior to reboot.
        */
        pid_t reboot_pid;
        if( 0 == (reboot_pid = fork()) ) {
          system((const char*)buffer);
          exit(1); /* never reached if command succeeds. */
        }
      }            
    }
  }
}
