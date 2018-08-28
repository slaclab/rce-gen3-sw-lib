// -*-Mode: C++;-*-
//
//                            Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include "atca/Attributes.hh"
#include "bsi/Bsi.h"
#include "dsl/Server.hh"
#include "dslutil/RebootSpace.hh"
#include "logger/Logger.hh"

namespace service {
  namespace dslutil {
    namespace rebooter {
      using service::logger::Logger;
      using service::atca::Attributes;


      /** @brief Initialize the rebooter DSL service
          Looks up the BSI information to pass to register_reboot
          Registers the rebooter service with DSL.
          Ownership of rebooter::Space is transferred to DSL.
      */
      void initialize() {
        Bsi bsi = LookupBsi();
        if (!bsi) {
          Logger().error("BSI lookup failed in service::dslutil::rebooter::init().");
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

        // Ownership of Attributes is transferred to Space
        Attributes *attr = new Attributes(group,cluster,bay,element);

        // Create and register Space with DSL
        Space* space = new Space(attr);
        service::dsl::Server::bind(space);
      }
    }
  }
}
