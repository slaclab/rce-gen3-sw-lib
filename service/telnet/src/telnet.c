/*
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	    000 - December 2, 2013
**
**  Revision History:
**	    None.
**
** --
*/



#include <rtems/telnetd.h>

#include "svt/Svt.h"    
#include "shell/shell.h"
#include "telnet/telnet_prefs.h"

static const char TELNET_PREFS[]  = "TELNET_PREFERENCES";
static const char _prefs_error[]  = "Telnet task prefs lookup failed for symbol %s\n";

rtems_telnetd_config_table rtems_telnetd_config;

// The telnet shell callback called by every telnet session started
void telnetShell(char *pty_name, void *cmd_arg)
  {
  Shell_Run(pty_name);  
  }

int lnk_prelude(Telnet_Prefs *p, void *elfHdr) 
  {
  /* Starting the telnetd server. Note that login_check only controls
     the behavior of the telnet daemon itself, not that of the shell
     it starts (some application shells might not have this ability so
     the telnetd can do it for them). The RTEMS shell login is
     controlled using the shell environment. See Shell_Run().
  */
  
  if(!p)
    {
    p = (Telnet_Prefs*)Svt_Translate(TELNET_PREFS, SVT_SYS_TABLE);
    int error = p ? SVT_SUCCESS: SVT_NOSUCH_SYMBOL;
    if(error) dbg_bugcheck(_prefs_error, TELNET_PREFS);
    }

  rtems_telnetd_config.command     = telnetShell;  /* "shell" function */
  rtems_telnetd_config.arg         = NULL; /* no context necessary for echoShell */
  rtems_telnetd_config.keep_stdio  = false;/* spawn a new thread */

  rtems_telnetd_config.priority    = p->priority;    /* priority .. we feel important today */
  rtems_telnetd_config.stack_size  = p->stack_size;  /* shell needs a large stack */
  rtems_telnetd_config.login_check = p->login_check; /* do not ask for password */

  return rtems_telnetd_initialize();
  }
