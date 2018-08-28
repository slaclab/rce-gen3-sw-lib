// -*-Mode: C++;-*-
//
//                           Copyright 2015
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//

#include <stdio.h>
#include "atca/Attributes.hh"
#include "dsl/Location.hh"
#include "dsl/Tag.hh"
#include "dslutil/AddressStar.hh"
#include "dslutil/UpdateBase.hh"
#include "dslutil/UpdateSpace.hh"
#include "logger/Logger.hh"
#include "net/Interface.hh"
#include "udp/Link.hh"
#include "dslutil/UpdateSpace.hh"
#include "dslutil/UpdateBase.hh"


namespace service {
  namespace dslutil {
    namespace updater {
      using service::logger::Logger;
      using service::dsl::Location;
    
      const char* Space::name = "DSL_UPDATE";
      uint8_t     _really_unused[sizeof(service::dsl::Location)];   

      Space::Space(const service::atca::Attributes* attr) :
        service::dsl::Space(name),
        _addr(attr->group(),
              attr->cluster(),
              attr->bay(),
              attr->element())
      {}


      const Location*
      Space::lookup(const service::dsl::Tag* _tag) {

        uint8_t buf[sizeof(AddressStar)];
        Tag* tag = (Tag*)_tag;
        
        AddressStar* received = tag->addr(buf);
      
        if ( ! received->match(&_addr) ) {
          return (Location*)0;
        }
      
        _update(tag->id(), tag->ip(), tag->src(), tag->dst());
      
        service::net::Interface _int;
        return new(_really_unused) Location(_int.ipAddr(), _int.hwAddr() << 16);
      }
      
      void Space::_post(uint32_t id, uint32_t status)
      {
        service::net::Interface interface;
        service::udp::Link  link(interface.bcAddr(),
                                 (uint16_t)UPDATE_PORT);
        
        Message* msg = new(_msg) Message(_addr.shelf(),
                                         _addr.slot(),
                                         _addr.cmb(),
                                         _addr.element(),
                                         _os(),
                                         id,
                                         status);

        link.post((void*)msg, sizeof(Message));
      }      
    }
  }
}
