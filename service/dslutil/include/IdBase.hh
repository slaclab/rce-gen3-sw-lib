// -*-Mode: C++;-*-
//
//                            Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//


#if !defined(SERVICE_DSLUTIL_IDENTIFIER_BASE_HH)
#define      SERVICE_DSLUTIL_IDENTIFIER_BASE_HH

#include <inttypes.h>

#include "net/Socket.hh"
#include "dslutil/AddressStar.hh"
#include "atca/Address.hh"
#include "dsl/Tag.hh"

namespace service {
  namespace dslutil {
    namespace identifier {

      enum {IDENTIFIER_PORT = service::net::Socket::WKS_BASE + 510};

      class Message {
      public:
        Message(const char* shelf_in,
                uint8_t     slot_in,
                uint8_t     cmb_in,
                uint8_t     element_in,
                uint8_t     os_in,
                uint32_t    ip_in,
                const char* id_in);
        ~Message() {};

      public:
        uint8_t   slot;
        uint8_t   cmb;
        uint8_t   element;
        uint8_t   os;
        uint32_t  ip;
        char      shelf[service::atca::Address::MAX_SHELF_STRING];
        char      id[32];
      };

      class Tag : public service::dsl::Tag {
      public:

        Tag(uint64_t, uint32_t, uint32_t, uint32_t);

        Tag(AddressStar*);

        virtual ~Tag() {}

      public:

        AddressStar* addr(uint8_t*);
      
      };
    }
  }
}
#endif
