#ifndef FmVlanTableEntry_hh
#define FmVlanTableEntry_hh


#include "fm/FmMReg.hh"

class SwmVlanTableEntry;

class FmVlanTableEntry {
public:
  FmVlanTableEntry() {}
  FmVlanTableEntry(const SwmVlanTableEntry&);
  ~FmVlanTableEntry() {}

  FmVlanTableEntry& operator=(const SwmVlanTableEntry&);

  operator SwmVlanTableEntry() const;

private:
  FmMReg _data[2];
};

#endif
