// -*-Mode: C++;-*-
//                               Copyright 2014
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#if !defined(SERVICE_DSLUTIL_REBOOTER_CLIENT_HH)
#define      SERVICE_DSLUTIL_REBOOTER_CLIENT_HH

#include <inttypes.h>

#include "dsl/Client.hh"
#include "net/Interface.hh"
#include "udp/Link.hh"
#include "dslutil/Base.h"

namespace service {
  namespace dslutil {
    class AddressStar;
    
    namespace rebooter {
      class Client : public service::dsl::Client {
      public:
        Client();
        Client(const char*);
        virtual ~Client() {}
      
      public:
        bool sendReboot(const char*,  uint32_t dest_os = OS_NOTSET, uint32_t load_bit = 0);
        bool sendReboot(AddressStar*, uint32_t dest_os = OS_NOTSET, uint32_t load_bit = 0);

      private:
        service::net::Interface      _interface;
        service::udp::Link           _udp;
      };
    }
  }
}
#endif
