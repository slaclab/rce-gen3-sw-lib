// -*-Mode: C++;-*-
//
//                           Copyright 2015
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>

#include "dsl/Location.hh"
#include "dslutil/Base.h"
#include "dslutil/UpdateSpace.hh"
#include "logger/Logger.hh"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"

#define DTM_BAY 4

namespace service {
  namespace dslutil {
    namespace updater {
      using service::logger::Logger;
      
      const char _update_mount[] = "/mnt/dslupdate";

      uint32_t
      Space::_os() { return OS_LINUX; }
      
      void Space::_update(uint32_t id, uint32_t ip, const char* src, const char* dst)
      {
        char *argv[2];
        char ip_buf[64];
        char cmd_buf[64];
        int ret;
        uint32_t status = 0;
        uint32_t isDtm = _addr.cmb() == DTM_BAY ? 1 : 0;
        
        dsl::Location loc(BSWP__swap32b(ip));
                
        /* Execute directory update using system calls */
        pid_t update_pid;
        if( 0 == (update_pid = fork()) ) {
          argv[0] = cmd_buf;

          sprintf(cmd_buf,"mkdir -p %s",_update_mount);
          ret = system(argv[0]);

          sprintf(cmd_buf,"umount -f %s",_update_mount);
          ret = system(argv[0]);
                    
          sprintf(cmd_buf,"mount -t nfs %s:%s %s -o ro,bg,retry=1",loc.layer3.addr_p(ip_buf),src,_update_mount);
          ret = system(argv[0]);
          if(ret)
            {
            _post(id,UPDATE_ERR_MOUNT);
            exit(0);
            }
                    
          sprintf(cmd_buf,"/bin/sh %s/%s 0x%08x %d&",_update_mount,dst,id,isDtm);
          ret = system(argv[0]);
          if(ret)
            {
            status |= UPDATE_ERR_SHELL;
            }

          Bsi bsi = LookupBsi();    
          if (!bsi)
            {
              printf("failure mapping bsi\n");
              return;
            }

          BsiWrite32(bsi,BSI_BOOT_RESPONSE_OFFSET,BSI_BOOT_RESPONSE_UPDATING);
          
          _post(id,status);
                    
          exit(0);
        }
      }            
    }
  }
}
