// -*-Mode: C++;-*-
//                               Copyright 2015
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#include <cstdio>

#include "kvt/Kvt.h"
#include "time/Time.hh"

#include "dslutil/RebootClient.hh"
#include "dslutil/RebootSpace.hh"
#include "dslutil/AddressStar.hh"
#include "dslutil/RebootBase.hh"



namespace {
  inline uint64_t client_hash(const char* string) {
    return KvtHash(string);
  }
}

namespace service {
  namespace dslutil {
    namespace rebooter {
    
      Client::Client() :
        service::dsl::Client(Space::name),
        _interface(),
        _udp(_interface.bcAddr(), (uint16_t)REBOOT_PORT, (uint16_t)REBOOT_PORT)
      {
      }


      Client::Client(const char* ifname) :
        service::dsl::Client(Space::name, ifname),
        _interface(ifname),
        _udp(_interface.bcAddr(), (uint16_t)REBOOT_PORT, (uint16_t)REBOOT_PORT)
      {
      }

      bool
      Client::sendReboot(const char* address,
                         uint32_t dest_os,
                         uint32_t load_bit)
      {
        AddressStar addr(address);
        return sendReboot(&addr, dest_os, load_bit);
      }

      bool
      Client::sendReboot(AddressStar* address,
                         uint32_t dest_os,
                         uint32_t load_bit)
      {
        // It is defined as an error if a reboot is sent to a specific
        // OS when the CMB is wildcarded as CMB_ALL_WC
        if ( (address->cmb() == CMB_ALL_WC) &&
             (dest_os != OS_NOTSET) ) {
          printf("ERROR: sendReboot of CMB_ALL_WC to a specific OS is not allowed.");
          return false;
        }
        
        Tag tag(address, dest_os, load_bit);

        lookup(&tag);

        return true;
      }
    }
  }
}
