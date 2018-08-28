// -*-Mode: C++;-*-
//                               Copyright 2014
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.


#include <inttypes.h>
#include "conversion/impl/BSWP.ih"
#include "dslutil/RebootBase.hh"

namespace service {
  namespace dslutil {
    namespace rebooter {
      /**
       */

      Tag::Tag(uint64_t shelf_hash,
               uint32_t slot,
               uint32_t cmb,
               uint32_t element,
               uint32_t dest_os,
               uint32_t load_bit) :
          service::dsl::Tag()
      {
        value.w64[0] = BSWP__swap64b(shelf_hash);
        value.w32[2] = BSWP__swap32b(slot);
        value.w32[3] = BSWP__swap32b(cmb);
        value.w32[4] = BSWP__swap32b(element);
        value.w32[5] = BSWP__swap32b(dest_os);
        value.w32[6] = BSWP__swap32b(load_bit);
      }

      Tag::Tag(AddressStar* addr,
               uint32_t dest_os,
               uint32_t load_bit) :
        service::dsl::Tag()
      {
        value.w64[0] = BSWP__swap64b(addr->shelf_hash());
        value.w32[2] = BSWP__swap32b(addr->slot());
        value.w32[3] = BSWP__swap32b(addr->cmb());
        value.w32[4] = BSWP__swap32b(addr->element());
        value.w32[5] = BSWP__swap32b(dest_os);
        value.w32[6] = BSWP__swap32b(load_bit);
      }

      uint32_t Tag::dest_os()
      {
        return BSWP__swap32b(value.w32[5]);
      }

      uint32_t Tag::load_bit()
      {
        return BSWP__swap32b(value.w32[6]);
      }

      AddressStar* Tag::addr(uint8_t* buffer)
      {
        AddressStar* result = new(buffer) AddressStar(BSWP__swap64b(value.w64[0]),
                                                      BSWP__swap32b(value.w32[2]),
                                                      BSWP__swap32b(value.w32[3]),
                                                      BSWP__swap32b(value.w32[4]));
        return result;
      }


      /** @cond development

      @var REBOOT_PORT
      @brief UDP port where the Re




       @endcond
       */

    }
  }
}
