// -*-Mode: C++;-*-
//
//                            Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//


#if !defined(SERVICE_DSLUTIL_REBOOTER_BASE_HH)
#define      SERVICE_DSLUTIL_REBOOTER_BASE_HH

#include <inttypes.h>

#include "dsl/Tag.hh"
#include "dslutil/AddressStar.hh"
#include "net/Socket.hh"

namespace service {
  namespace dslutil {
    namespace rebooter {

      enum {REBOOT_PORT = service::net::Socket::WKS_BASE + 511};

      class Tag : public service::dsl::Tag {
      public:
        
        Tag(uint64_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
        
        Tag(AddressStar*, uint32_t os, uint32_t load_bit);

        virtual ~Tag() {}

      public:

        uint32_t     dest_os();
        uint32_t     load_bit();
        AddressStar* addr(uint8_t*);
      };
     
    }
  }
}
#endif
