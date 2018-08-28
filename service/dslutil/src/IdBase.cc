// -*-Mode: C++;-*-
//                               Copyright 2014
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.


#include <inttypes.h>
#include <string.h>

#include "conversion/impl/BSWP.ih"
#include "dslutil/IdBase.hh"

namespace service {
  namespace dslutil {
    namespace identifier {
      /**
       */


      Message::Message(const char* shelf_in,
                       uint8_t     slot_in,
                       uint8_t     cmb_in,
                       uint8_t     element_in,
                       uint8_t     os_in,
                       uint32_t    ip_in,
                       const char* id_in) :
        slot(slot_in), cmb(cmb_in), element(element_in), os(os_in), ip(ip_in)
      {
        strncpy(shelf, shelf_in, service::atca::Address::MAX_SHELF_STRING);
        strncpy(id, id_in, service::atca::Address::MAX_SHELF_STRING);        
      }

      Tag::Tag(uint64_t shelf_hash,
               uint32_t slot,
               uint32_t cmb,
               uint32_t element) :
          service::dsl::Tag()
      {
        value.w64[0] = BSWP__swap64b(shelf_hash);
        value.w32[2] = BSWP__swap32b(slot);
        value.w32[3] = BSWP__swap32b(cmb);
        value.w32[4] = BSWP__swap32b(element);
      }
      
      Tag::Tag(AddressStar* addr) :
        service::dsl::Tag()
      {
        value.w64[0] = BSWP__swap64b(addr->shelf_hash());
        value.w32[2] = BSWP__swap32b(addr->slot());
        value.w32[3] = BSWP__swap32b(addr->cmb());
        value.w32[4] = BSWP__swap32b(addr->element());
      }

      AddressStar* Tag::addr(uint8_t* buffer)
      {
        AddressStar* result = new(buffer) AddressStar(BSWP__swap64b(value.w64[0]),
                                                      BSWP__swap32b(value.w32[2]),
                                                      BSWP__swap32b(value.w32[3]),
                                                      BSWP__swap32b(value.w32[4]));
        return result;
      }
    }
  }
}
