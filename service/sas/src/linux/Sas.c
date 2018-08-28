/*
** ++
**  Package: SAS
**	
**
**  Abstract: System level SAS LINUX routines
**
**  Implementation of LINUX SAS facility. See the corresponding header (".h") file for a
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

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <dlfcn.h>

#include "memory/mem.h"
#include "map/Lookup.h"

#include "sas/Sas.h"

#include "../Map.h"
#include "../Interrupt.h"
#include "../Transaction.h"

//#define DEBUG

#ifdef DEBUG
#define debug_printf printf
#else
#define debug_printf(...)
#endif

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

const char SVT_NAME[]              = "libsassvt.so";     
const char INTR_MAP[]              = "SAS_INTR_MAP";     
const char PLUGIN_PREFS[]          = "SAS_PLUGIN_PREFS"; 
const char APP_PREFS[]             = "SAS_APP_PREFS";    

const char _svt_install_error[]    = "Cannot dlopen svt: %s\n";
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
  void *handle = dlopen(SVT_NAME,RTLD_LAZY);
  if(!handle)
    {
    debug_printf(_svt_install_error, SVT_NAME);
    return NULL;
    }  
  
  /* lookup interrupt map in the SAS SVT */  
  InterruptMap *intrMap = (InterruptMap*)dlsym(handle,INTR_MAP);
  if(!intrMap)
    {
    debug_printf(_svt_translate_error, INTR_MAP);
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

  void *handle = dlopen(SVT_NAME,RTLD_LAZY);
  if(!handle)
    {
    debug_printf(_svt_install_error, SVT_NAME);
    return SAS_NO_SVT;
    }  

  /* find the list of plugins to bind */
  SAS_Preferences **list = (SAS_Preferences**)dlsym(handle,PLUGIN_PREFS);
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

  void *handle = dlopen(SVT_NAME,RTLD_LAZY);
  if(!handle)
    {
    debug_printf(_svt_install_error, SVT_NAME);
    return SAS_NO_SVT;
    }  
  
  /* find the list of fifos to bind */
  SAS_AppPreferences **list = (SAS_AppPreferences**)dlsym(handle,APP_PREFS);
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
* for the LINUX SAS shareable library.
*
** --
*/

__attribute__((constructor))
static void init(void) 
  {  
  /* map the ocm */
  SasOcm = LookupOcm();
  if(!SasOcm)
    {
    printf("Error mapping ocm\n");
    return;
    }
  SasOcmMap = (OcmMap*)SasOcm;

  /* map the axi */
  SasAxi = LookupAxiSocket();
  if(!SasAxi)
    {
    printf("Error mapping axi\n");
    return;
    }
  
  /* initialize the facility */
  int error = SAS_Construct();
  if(error != SAS_SUCCESS)
    {
    printf("Error constructing SAS 0x%x\n",(int)error);
    return;
    }

  /* bind plugin sockets */
  error = BindPlugins();
  if(error != SAS_SUCCESS)
    {
    printf("Error binding SAS plugin sockets 0x%x\n",(int)error);
    return;
    }
    
  /* bind application fifos */
  error = BindAppFifos();
  if(error != SAS_SUCCESS)
    {
    printf("Error binding SAS application fifos 0x%x\n",(int)error);
    return;
    }
  }
