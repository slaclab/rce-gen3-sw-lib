// -*-Mode: C++;-*-
//
//                           Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include "atca/Attributes.hh"
#include "dsl/Location.hh"
#include "dsl/Tag.hh"
#include "dslutil/AddressStar.hh"
#include "dslutil/IdSpace.hh"
#include "dslutil/IdBase.hh"

#include "logger/Logger.hh"
#include "net/Interface.hh"
#include "udp/Link.hh"

#include "debug/print.h"

namespace service {
  namespace dslutil {
    namespace identifier {
      using service::logger::Logger;
      using service::dsl::Location;
    
      const char* Space::name = "DSL_IDENTIFIER";


      Space::Space(const service::atca::Attributes* attr) :
        service::dsl::Space(name),
        _addr(attr->group(),
              attr->cluster(),
              attr->bay(),
              attr->element()),
        _loc(attr->ip(), attr->mac())
      {
      }

      const Location*
      Space::lookup(const service::dsl::Tag* _tag) {

        uint8_t buf[sizeof(AddressStar)];
        Tag* tag = (Tag*)_tag;
        
        AddressStar* received = tag->addr(buf);
      
        if (  received->match(&_addr) ) {
          identify();          
        }
      
        return &_loc;
      }

      void
      Space::identify() {
        service::net::Interface interface;
        service::udp::Link  link(interface.bcAddr(),
                                 (uint16_t)IDENTIFIER_PORT);
                
        Message* msg = new(_msg) Message(_addr.shelf(),
                                         _addr.slot(),
                                         _addr.cmb(),
                                         _addr.element(),
                                         _os(),
                                         interface.ipAddr(),
                                         _id());

        link.post((void*)msg, sizeof(Message));
      }
    }
  }
}
