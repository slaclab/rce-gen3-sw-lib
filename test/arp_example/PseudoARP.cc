// -*-Mode: C++;-*-
/**
@file
@brief Brief description of file contents and purpose here.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include "bsi/Bsi.h"
#include "debug/print.h"
#include "hash/Hash.h"
#include "elf/linker.h"

#include "PseudoARP.hh"

#define PRINT dbg_printv


#define NSLOTS       2   // Assume a two slot crate for the exercise
#define NBAY         4   // There are 4 Bays on a V8 COB  
#define NRCE         2   // There are 2 RCEs on a DPM

#define LOG2_ENTRIES  8    // log_2(entries in table)

#define MAC_HEAD_MSB      0x08
#define MAC_HEAD_LSB      0x00


// Blank namespace for local/private functions
namespace {
  static int      _populate_loop(const char*, KvTable);
  static uint32_t _reduce(uint64_t);
  static uint64_t _unreduce(uint32_t);
}

namespace examples {
   
  PseudoARP::PseudoARP()
  {
    char* buffer = new char[KvtSizeof(LOG2_ENTRIES)];
    
    _table = KvtConstruct(LOG2_ENTRIES, buffer);
    
    if (0 == _table) {
      // allocation error, deal with it
      PRINT("Allocation error in PseudoARP.  Aborting constructor.\n");
      return;
    }
    int rc = refresh();
    if (0 != rc) {
      // error in refreshing
      PRINT("Refresh error in PseudoARP.  Aborting constructor.\n");
      return;
    }
  }

  PseudoARP::~PseudoARP()
  {
  }

  uint64_t
  PseudoARP::lookup(uint32_t ip)
  {
    uint32_t reduced;
    KvtKey key = Hash64_32(0, ip);
    if ((reduced = (uint32_t)KvtLookup(key, _table)) == 0) {
      // not found
      return 0;
    }

    return _unreduce(reduced);
  }

  int
  PseudoARP::refresh()
  {
    const char* shelf;
    int rc = -1;
    
    { // Fetch the shelf name from the BSI
      Bsi bsi = LookupBsi();
      if (!bsi) {        // error state, deal with it.
        return rc;
      }

      uint32_t addr;
      char buffer[BSI_GROUP_NAME_SIZE * sizeof(unsigned)];
      shelf = BsiReadGroup(bsi, buffer);
    }

    rc = _populate_loop(shelf, _table);
    return rc;
  }
}


namespace {

  static uint32_t _reduce(uint64_t mac)
  {
    // Note: MAC comes in NW byte order
    return (uint32_t)((mac >> 32) & 0xffffffff );
  }

  static uint64_t _unreduce(uint32_t reduced)
  {
    uint64_t mac = (uint64_t)reduced << 32;
    mac |= (uint64_t)MAC_HEAD_LSB   << (32-8);
    mac |= (uint64_t)MAC_HEAD_MSB   << (32-16);
    
    return (mac);
  }

}

#include "atca/Client.hh"
#include "atca/Address.hh"
#include "net/MacAddress.hh"
namespace {
  static int _populate_loop(const char* shelf, KvTable table)
  {
    int count = 0;

    for (unsigned slot=0; slot<NSLOTS; ++slot) {
      for (unsigned bay=0; bay<NBAY; ++bay) {
        for (unsigned rce=0; rce < 2*NRCE; rce += 2) {
          service::atca::Client client;
          
          service::atca::Address addr(shelf, slot+1, bay, rce);
          
          service::dsl::Location* loc = client.lookup(addr);
          if (!loc) continue;

          uint32_t ipaddr  = loc->layer3.addr();
          uint64_t mac     = loc->layer2;

          KvtKey key = Hash64_32(0, ipaddr);
          KvtValue value = (KvtValue)_reduce(mac);

          // If the insert goes bad, return error
          if (0 == KvtInsert(key, value, table))       return -1;
          ++count;
        }
      }      
    }
    // Nothing in the Table?  Error
    if (0 == count) return -1;
    
    return 0;
  }
}

// Global instance of the DB.
examples::PseudoARP* common_pseudoarp_instance = 0;

// "Install" this object
extern "C" const int lnk_options = LNK_INSTALL;

extern "C" int lnk_prelude(void *arg, void *elf)
{
  PRINT("Constructing PseudoARP DB.\n");
  PRINT("This may take some time if the shelf is not completely populated...\n");
  common_pseudoarp_instance = new examples::PseudoARP();
  PRINT("Done.\n");
  return 0;
}

  
