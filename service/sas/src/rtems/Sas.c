/*
** ++
**  Package: SAS
**	
**
**  Abstract: System level SAS RTEMS routines
**
**  Implementation of RTEMS SAS facility. See the corresponding header (".h") file for a
**  more complete description of the functions implemented below.      
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - June 30, 2014
**
**  Revision History:
**	None.
**
** --
*/

#include <stdint.h>

#include "rtems.h"

#include "map/Lookup.h"
#include "svt/Svt.h"
#include "elf/linker.h"
#include "debug/print.h"

#include "sas/Sas.h"

#include "../Map.h"
#include "../Interrupt.h"
#include "../Transaction.h"

Axi     SasAxi;                    
Ocm     SasOcm;                    
OcmMap *SasOcmMap = 0;             

/*
**
** These constants specify the SAS SVT table and symbol names. 
** The values represented here must be maintained with
** the corresponding SVT implementation.
**
*/

const char SVT_NAME[]              = "config:sas.svt";
const char INTR_MAP[]              = "SAS_INTR_MAP";     
const char PLUGIN_PREFS[]          = "SAS_PLUGIN_PREFS"; 
const char APP_PREFS[]             = "SAS_APP_PREFS";    

const char _svt_install_error[]    = "Cannot install svt: %s error 0x%x\n";
const char _svt_translate_error[]  = "No svt symbol found: %s\n";

/*
** ++
**
** This function installs the SAS symbol value table.
**
** --
*/

InterruptMap *InstallSvt(void)
  {
  InterruptMap *intrMap;
  int error = Svt_Install(SVT_SAS_TABLE_NUM, SVT_NAME);  
  if(error)
    {
    dbg_printv(_svt_install_error, SVT_NAME, error);
    return NULL;
    }
  
  /* lookup interrupt map in the SAS SVT */
  intrMap = (InterruptMap*)Svt_Translate(INTR_MAP, SVT_SAS_TABLE);  
  if(!intrMap)
    {
    dbg_printv(_svt_translate_error, INTR_MAP);
    return NULL;
    }
  return intrMap;
  }

/*
** ++
**
** This function bind plugins to sockets using the svt.
**
** --
*/

SAS_Status BindPlugins(void)
  {
  SAS_Preferences **prefs;
  SAS_Status       status = SAS_SUCCESS;
    
  /* find the list of plugins to bind */
  SAS_Preferences **list = (SAS_Preferences**)Svt_Translate(PLUGIN_PREFS, SVT_SAS_TABLE);
  if(!list) return status;
  
  /* bind plugins to sockets */
  for(prefs=list;*prefs;prefs++)
    {
    status = SAS_Plugin(*prefs);
    if(status != SAS_SUCCESS) break;
    }
  return status;
  }

/*
** ++
**
** This function registers application fifo configurations using the svt.
**
** --
*/

SAS_Status BindAppFifos(void)
  {
  SAS_AppPreferences **attrs;
  SAS_Status        status = SAS_SUCCESS;
    
  /* find the list of fifos to bind */
  SAS_AppPreferences **list = (SAS_AppPreferences**)Svt_Translate(APP_PREFS, SVT_SAS_TABLE);
  if(!list) return status;
  
  /* bind fifos to slots */
  for(attrs=list;*attrs;attrs++)
    {
    status = SAS_AppRegister(*attrs);
    if(status != SAS_SUCCESS) break;
    }
  return status;
  }

/*
** ++
**
* This routine executes the constructor 
* for the RTEMS SAS shareable library.
*
** --
*/

int lnk_options = LNK_INSTALL;

int lnk_prelude(void *prefs, void *elfHdr) 
  {
  SAS_Status status = SAS_SUCCESS;
  
  /* map the ocm */
  SasOcm = LookupOcm();
  if(!SasOcm) return SAS_NO_OCM;
  SasOcmMap = (OcmMap*)SasOcm;
  
  /* map the axi */
  SasAxi = LookupAxiSocket();
  if(!SasAxi) return SAS_NO_AXI;
  
  /* initialize the facility */
  status = SAS_Construct();
  if(status != SAS_SUCCESS) return status;
  
  /* register OS interrupt handlers */
  status = IntrRegister();
  if(status != SAS_SUCCESS) return status;

  /* bind plugin sockets */
  status = BindPlugins();
  if(status != SAS_SUCCESS) return status;

  /* bind application fifos */
  status = BindAppFifos();
  if(status != SAS_SUCCESS) return status;
  
  /* register shell commands */
  SAS_AddCommands();
  
  return status;
  }
