// -*-Mode: C++;-*-
//                               Copyright 2015
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#if !defined(SERVICE_DSLUTIL_UPDATER_CLIENT_HH)
#define      SERVICE_DSLUTIL_UPDATER_CLIENT_HH

#include <inttypes.h>

#include "dsl/Client.hh"
#include "net/Interface.hh"
#include "udp/Link.hh"
#include "dslutil/Base.h"
#include "dslutil/UpdateListener.hh"

namespace service {
  namespace dslutil {
    class AddressStar;
    
    namespace updater {
      class Client : public service::dsl::Client {
      public:
        enum {MAX_ID_TIMEOUT = 3};

      public:
        Client(Listener*);
        Client(Listener*, const char*);
        virtual ~Client() {}
      
      public:
        bool sendUpdate(const char*,  uint32_t id, uint32_t ip, const char *src, const char *dst, bool clear=false);
        bool sendUpdate(AddressStar*, uint32_t id, uint32_t ip, const char *src, const char *dst, bool clear=false);

      private:
        service::net::Interface      _interface;
        service::udp::Link           _udp;

      private:
        Listener*                    _listener;        
      };
    }
  }
}
#endif
