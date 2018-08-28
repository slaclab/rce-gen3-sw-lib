//==============================================================//
//
//  This table is a mirror of the hardware table within the switch
//  It learns of updates to the hardware table via an interrupt,
//    and updates its copy via changes stored in the hardware FIFO.
//
//  The interface allows entries to be added, removed, or updated.
//  Changes are reflected in the hardware table.
//
//  The only guarantee I see of getting exclusive access to the
//  hardware table is by disabling learning on all the ports.
//  Also need a mutex among software threads.
//
//==============================================================//

#ifndef SwmMacTable_hh
#define SwmMacTable_hh


#include "fm/SwmMacTableEntry.hh"

class FmMacTable;

class SwmMacTable {
public:
  enum { Entries=0x4000 };

  SwmMacTable() {}
  SwmMacTable(const SwmMacTable&);
  SwmMacTable(const FmMacTable&);
  ~SwmMacTable() {}

  void clear();

  SwmMacTableEntry entry[Entries];
};

#endif
