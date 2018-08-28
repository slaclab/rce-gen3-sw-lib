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

#ifndef FmMacTable_hh
#define FmMacTable_hh


#include "fm/FmHandler.hh"

#include "fm/SwmMacTable.hh"
#include "fm/SwmMacTableEntry.hh"

//
//  I need to make sure I handle the case of trying to add an entry for
//  addresses that are already subscribed (static versus dynamic) or
//  trying to add an entry for addresses that don't resolve to an
//  empty entry.  Do static entries override dynamic entries?
//

class FmTahoe;

class FmMacTable : public FmHandler {
public:
  enum { Entries=0x4000 };

  FmMacTable(FmTahoe& d);
  virtual ~FmMacTable() {}

  int findMatch(const SwmMacTableEntry&) const;

  const SwmMacTableEntry& entry(unsigned index) const { return _entry[index]; }
  SwmMacTableEntry direct_entry(unsigned index) const;

  //  Change particular entries in the table
  //  (is index complete or only the hash function result?)
  void updateEntry(unsigned index,const SwmMacTableEntry&);
  void deleteEntry(unsigned index,const SwmMacTableEntry&);

  //  Change entries in the table wherever they are found
  //  First look for matches of MacAddr and Vlan
  int  addEntry   (const SwmMacTableEntry&);
  void deleteEntry(const SwmMacTableEntry&);

  virtual void call(unsigned);

  int updated(unsigned i) const { return _updated[i]; }

private:
  SwmMacTableEntry _entry[Entries];
  FmTahoe&         _dev;
  int short        _updated[64];
};

#endif
