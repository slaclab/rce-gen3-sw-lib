// -*-Mode: C++;-*-
//                               Copyright 2014
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#include <stdio.h>

#include "time/Time.hh"

#include "dslutil/AddressStar.hh"
#include "dslutil/IdBase.hh"
#include "dslutil/IdClient.hh"
#include "dslutil/IdSpace.hh"

namespace service {
  namespace dslutil {
    namespace identifier {
    
      Client::Client(Listener* l) :
        service::dsl::Client(Space::name),
        _listener(l)
      {
      }


      Client::Client(Listener* l, const char* ifname) :
        service::dsl::Client(Space::name, ifname),
        _listener(l)
      {
      }


      bool
      Client::identify(const char* address,
                       bool clear)
      {
        AddressStar addr(address);
        return identify(&addr, clear);
      }
        
      bool
      Client::identify(AddressStar* address,
                       bool clear)
      {
        
        _listener->setShelf(address->shelf());
        _listener->launch();  // Launch the listener if it is not already launched.

        Tag tag(address);

        if (clear) _listener->clear();

        bool result = (lookup(&tag) != 0);
        tool::concurrency::Thread::threadYield();
        
        return result;
      }
    }    
  }
}
