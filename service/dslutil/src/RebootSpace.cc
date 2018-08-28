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
#include "dslutil/RebootBase.hh"
#include "dslutil/RebootSpace.hh"
#include "logger/Logger.hh"
#include "net/Interface.hh"


namespace service {
  namespace dslutil {
    namespace rebooter {
      using service::logger::Logger;
      using service::dsl::Location;
    
      const char* Space::name = "DSL_REBOOT";
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
      
        _reboot(tag->dest_os(), tag->load_bit());
      
        service::net::Interface _int;
        return new(_really_unused) Location(_int.ipAddr(), _int.hwAddr() << 16);
      }

    }
  }
}
