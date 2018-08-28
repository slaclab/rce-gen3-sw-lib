// -*-Mode: C++;-*-
//                               Copyright 2015
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#if !defined(SERVICE_DSLUTIL_UPDATER_LISTENER_HH)
#define      SERVICE_DSLUTIL_UPDATER_LISTENER_HH

#include <inttypes.h>
#include <map>

#include "concurrency/Thread.hh"
#include "concurrency/Semaphore.hh"
#include "net/Interface.hh"
#include "udp/Link.hh"
#include "dslutil/Base.h"
#include "cob_ipmc/COB_ipmc.h"

namespace service {
  namespace dslutil {
    namespace updater {

      class Listener : public tool::concurrency::Thread {
      public:
        enum {MAX_SHELF_STR  = 72};
        
      public:
        Listener();
        Listener(const char*);
        virtual ~Listener();

      public:
        virtual void body();

      public:
        void launch() { start(); yield(); }
        void terminate() { _terminate = true; }
        
      public:
        void clear();
        DSL_slot_update_status* status(void*);
        void report(COB_slot_rce_status *exp, bool showMissing=false);

      public:
        void setShelf(const char*);

      private:
        tool::concurrency::Semaphore _lock[2];
        service::net::Interface      _interface;
        service::udp::Link           _udp;
        DSL_slot_update_status       _status;
        char                         _shelf[MAX_SHELF_STR];
        volatile bool                _terminate;
      };
    }
  }
}
#endif
