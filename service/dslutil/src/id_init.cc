// -*-Mode: C++;-*-
//
//                            Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include "dslutil/IdSpace.hh"
#include "bsi/Bsi.h"
#include "logger/Logger.hh"
#include "atca/Attributes.hh"
#include "dsl/Server.hh"
#include "dslutil/IdBase.hh"

namespace service {
  namespace dslutil {
    namespace identifier {
      using service::logger::Logger;
      using service::atca::Attributes;

    
      /** @brief Initialize the identifier DSL service
          Looks up the BSI information
          Constructs and binds the Space to DSL
          Sends out one identifier packet.
      */
      void initialize() {
        Bsi bsi = LookupBsi();
        if (!bsi) {
          Logger().error("BSI lookup failed in init_identify.");
          return;
        }

        const char *group;
        uint32_t addr;
        uint32_t cluster;
        uint32_t bay;
        uint32_t element;

        char buffer[BSI_GROUP_NAME_SIZE * sizeof(unsigned)];
        group = BsiReadGroup(bsi,buffer);
        addr = BsiRead32(bsi,BSI_CLUSTER_ADDR_OFFSET);
        cluster = BSI_CLUSTER_FROM_CLUSTER_ADDR(addr);
        bay = BSI_BAY_FROM_CLUSTER_ADDR(addr);
        element = BSI_ELEMENT_FROM_CLUSTER_ADDR(addr);

        // Ownership of Attributes is transferred to RebootSpace
        Attributes *attr = new Attributes(group,cluster,bay,element);

        Space* space = new Space(attr);
        service::dsl::Server::bind(space);

        space->identify();        
      }
    }
  }
}

