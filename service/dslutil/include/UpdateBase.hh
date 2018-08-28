// -*-Mode: C++;-*-
//
//                            Copyright 2015
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.
//


#if !defined(SERVICE_DSLUTIL_UPDATER_BASE_HH)
#define      SERVICE_DSLUTIL_UPDATER_BASE_HH

#include <inttypes.h>

#include "dsl/Tag.hh"
#include "dslutil/AddressStar.hh"
#include "net/Socket.hh"

#define MAX_PATH_LEN 32 // bytes

namespace service {
  namespace dslutil {
    namespace updater {

      enum {UPDATE_PORT = service::net::Socket::WKS_BASE + 509};
      enum {
           UPDATE_SUCCESS         = 0,
           UPDATE_ERR_MKDIR       = 1<<1,
           UPDATE_ERR_MOUNT       = 1<<2,
           UPDATE_ERR_SHELL       = 1<<3,
           UPDATE_ERR_UMOUNT      = 1<<4,
           UPDATE_ERR_RMDIR       = 1<<5
           };

      class Message {
      public:
        Message(const char* shelf_in,
                uint8_t     slot_in,
                uint8_t     cmb_in,
                uint8_t     element_in,
                uint8_t     os_in,
                uint32_t    id_in,
                uint64_t    status_in);
        ~Message() {};

      public:
        uint8_t   slot;
        uint8_t   cmb;
        uint8_t   element;
        uint8_t   os;
        uint32_t  id;
        uint32_t  status;
        char      shelf[service::atca::Address::MAX_SHELF_STRING];
      };

      class Tag : public service::dsl::Tag {
      public:
                
        Tag(AddressStar*, uint32_t id, uint32_t ip, const char* src, const char* dst);

        virtual ~Tag() {}

      public:

        uint32_t     id();
        uint32_t     ip();
        const char*  src();
        const char*  dst();
        AddressStar* addr(uint8_t*);
      };
    }
  }
}
#endif
