// -*-Mode: C++;-*-
//                               Copyright 2014
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#if !defined(SERVICE_DSLUTIL_IDENTIFIER_CLIENT_HH)
#define      SERVICE_DSLUTIL_IDENTIFIER_CLIENT_HH

#include <inttypes.h>

#include "dsl/Client.hh"
//#include "dslutil/Base.h"
#include "dslutil/IdListener.hh"

namespace service {
  namespace dslutil {
    class AddressStar;
    
    namespace identifier {
       
      class Client : public service::dsl::Client {
      public:
        enum {MAX_ID_TIMEOUT = 3};
        
      public:
        Client(Listener*);
        Client(Listener*, const char*);
        virtual ~Client() {}
      
      public:
        bool identify(AddressStar*, bool clear=false);
        bool identify(const char*, bool clear=false);

      private:
        Listener*            _listener;
      };
    }
  }
}
#endif
