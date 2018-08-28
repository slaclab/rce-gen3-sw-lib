// -*-Mode: C++;-*-
//
//                           Copyright 2015
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include <rtems.h>
#include <unistd.h>
#include <stdio.h>

#include <rtems/shell.h>

#include "dslutil/UpdateSpace.hh"
#include "dslutil/UpdateBase.hh"
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

#define DSL_UPDATE_PRIORITY 160

namespace service {
  namespace dslutil {
    namespace updater {

      uint32_t
      Space::_os() { return OS_RTEMS; }

      using service::logger::Logger;

      const char _update_mount[] = "/mnt/dslupdate";
      const char _last_tag   []  = "/mnt/dslupdate/TAG";
      const char _last_update[]  = "/mnt/rtemsapp/config/.updating";
      
      const char _mount_error1[] = "SD mount point create failure 0x%x errno 0x%x: %s\n";
      const char _mount_error2[] = "SD mount failure 0x%x: %s to %s\n";
                        
      void Space::_update(uint32_t id, uint32_t ip, const char* src, const char* dst)
      {
        const char *argv[4];
        char ip_buf[64];
        char src_buf[64];
        char cmd_buf[64];
        uint32_t status = 0;

        dsl::Location loc(BSWP__swap32b(ip));
               
        sprintf(src_buf,"%s:%s",loc.layer3.addr_p(ip_buf),src);        
       
        int error = rtems_mkdir(_update_mount, S_IRWXU|S_IRWXG|S_IRWXO);
        if(error)
          {
          printf(_mount_error1, error, strerror(errno), _update_mount);         
          _post(id,UPDATE_ERR_MKDIR);
          return;
          }
         
        if (mount (src_buf, _update_mount, "nfs", RTEMS_FILESYSTEM_READ_WRITE, NULL) < 0)
          {
          printf(_mount_error2, error, src_buf, _update_mount);
          _post(id,UPDATE_ERR_MOUNT);
          return;
          }
          
        FILE *fd;
        bzero(cmd_buf,DSLUTIL_MAX_ID_STR);
        fd = fopen(_last_tag,"rb");
        if(fd)                                     
          {          
          fread(cmd_buf,DSLUTIL_MAX_ID_STR,1,fd);
          fclose(fd);
          }
        
        sprintf(src_buf,"%s/%s",_update_mount,dst);
        
        /*
         * Run the script
         */
        rtems_status_code result = rtems_shell_script(
          "DSLU",        /* the name of the task */
          RTEMS_MINIMUM_STACK_SIZE * 10,       /* stack size */
          DSL_UPDATE_PRIORITY,   /* task priority */
          src_buf,      /* the script file */
          "stdout",     /* where to redirect the script */
          0,            /* run once and exit */
          0,            /* we will wait */
          0             /* do we echo */
        );
        if (result) status = UPDATE_ERR_SHELL;
                  
        fd = fopen(_last_update,"w+b");
        if(!fd)
          {
          status |= UPDATE_ERR_SHELL;
          }
        else
          {          
          rewind(fd);
          sprintf(src_buf,"0x%08x %s",id,cmd_buf);
          fwrite(src_buf,1,strlen(src_buf),fd);
          fclose(fd);
          }
          
        Bsi bsi = LookupBsi();    
        if (!bsi)
          {
            printf("failure mapping bsi\n");
            return;
          }

        BsiWrite32(bsi,BSI_BOOT_RESPONSE_OFFSET,BSI_BOOT_RESPONSE_UPDATING);

        _post(id,status);

      }      
    }
  }
}
