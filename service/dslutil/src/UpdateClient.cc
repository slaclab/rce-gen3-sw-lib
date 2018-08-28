// -*-Mode: C++;-*-
//                               Copyright 2015
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#include <cstdio>

#include "kvt/Kvt.h"
#include "time/Time.hh"

#include "dslutil/UpdateClient.hh"
#include "dslutil/UpdateSpace.hh"
#include "dslutil/AddressStar.hh"
#include "dslutil/UpdateBase.hh"
#include "dslutil/UpdateListener.hh"


namespace {
  inline uint64_t client_hash(const char* string) {
    return KvtHash(string);
  }
}

namespace service {
  namespace dslutil {
    namespace updater {
    
      Client::Client(Listener* l) :
        service::dsl::Client(Space::name),
        _interface(),
        _udp(_interface.bcAddr(), (uint16_t)UPDATE_PORT, (uint16_t)UPDATE_PORT),
        _listener(l)       
      {
      }


      Client::Client(Listener* l, const char* ifname) :
        service::dsl::Client(Space::name, ifname),
        _interface(ifname),
        _udp(_interface.bcAddr(), (uint16_t)UPDATE_PORT, (uint16_t)UPDATE_PORT),
        _listener(l)        
      {
      }

      bool
      Client::sendUpdate(const char* address,
                         uint32_t id,
                         uint32_t ip,
                         const char* src,
                         const char* dst,
                         bool clear)
      {
        AddressStar addr(address);
        return sendUpdate(&addr, id, ip, src, dst, clear);
      }

      bool
      Client::sendUpdate(AddressStar* address,
                         uint32_t id,
                         uint32_t ip,
                         const char* src,
                         const char* dst,
                         bool clear)
      {
      
        _listener->setShelf(address->shelf());
        _listener->launch();  // Launch the listener if it is not already launched.
        
        Tag tag(address, id, ip, src, dst);

        if (clear) _listener->clear();

        bool result = (lookup(&tag) != 0);
        tool::concurrency::Thread::threadYield();
        
        return result;
      }
    }
  }
}
