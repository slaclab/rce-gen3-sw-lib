// -*-Mode: C++;-*-
/*!@file     init.cc
*
* @brief     Initialization code for launching the ATCA server.
*
* @author    R. Claus     -- REG/DRD - (claus@slac.stanford.edu)
*            S. Maldonado -- REG/DRD - (smaldona@slac.stanford.edu)
*
* @date      July 3, 2013 -- Created
*
* $Revision: 4078 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/


#include "atca/init.hh"
#include "atca/Space.hh"
#include "atca/Attributes.hh"

#include "dsl/Server.hh"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"
#include "elf/linker.h"

using service::dsl::Server;
using service::atca::Space;
using service::atca::Attributes;

#if defined(__cplusplus)
extern "C" {
#endif

int lnk_options __attribute__((visibility("default"))) =  LNK_INSTALL;

namespace service {

  namespace atca {

    /** @brief Initialize the DSL ATCA subsystem on RTEMS.

    Looks up the Boostrap Info (BSI) and binds to the master DSL
    server.
     */    
    void initialize()
    {
      Bsi bsi = LookupBsi();
      const char *group;
      uint32_t addr;
      uint32_t cluster;
      uint32_t bay;
      uint32_t element;
      
      if(!bsi) return;
      
      char buffer[BSI_GROUP_NAME_SIZE * sizeof(unsigned)];
      group = BsiReadGroup(bsi,buffer);
      addr = BsiRead32(bsi,BSI_CLUSTER_ADDR_OFFSET);
      cluster = BSI_CLUSTER_FROM_CLUSTER_ADDR(addr);
      bay = BSI_BAY_FROM_CLUSTER_ADDR(addr);
      element = BSI_ELEMENT_FROM_CLUSTER_ADDR(addr);
      
      Attributes *attr = new Attributes(group,cluster,bay,element);
      // Bind the ATCA space to the DSL server
      Space *space = new Space(attr);
      Server::bind(space);
    }
    
  }

}

/** @cond development */
/** @brief Execute the ATCA service initialization code on library load.
 */
int lnk_prelude(void *prefs, void *elfHdr) 
 { 
 service::atca::initialize();
 return 0;
 }

/** @endcond */  

#if defined(__cplusplus)
} // extern "C"
#endif
