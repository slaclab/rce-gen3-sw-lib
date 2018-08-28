#ifndef FmFidTableEntry_hh
#define FmFidTableEntry_hh


#include "fm/FmMReg.hh"

class SwmFidTableEntry;

class FmFidTableEntry {
public:
  FmFidTableEntry() {}
  FmFidTableEntry(const SwmFidTableEntry&);
  ~FmFidTableEntry() {}

  FmFidTableEntry& operator=(const SwmFidTableEntry&);

  operator SwmFidTableEntry() const;

private:
  FmMReg _data[2];
};

#endif
