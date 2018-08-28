#ifndef FmMacTableEntry_hh
#define FmMacTableEntry_hh


#include "fm/FmMReg.hh"

class SwmMacTableEntry;

class FmMacTableEntry {
public:
  FmMacTableEntry() {}

  FmMacTableEntry& operator=(const SwmMacTableEntry&);

  operator SwmMacTableEntry() const;

  FmMReg _data[4];

private:
  ~FmMacTableEntry() {}
  friend class FmTahoe;
};

#endif
