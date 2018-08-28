#ifndef SwmMacTableD_hh
#define SwmMacTableD_hh


#include "fm/SwmMacTable.hh"

class SwmMacTableD : public SwmMacTable {
public:
  SwmMacTableD() {}
  SwmMacTableD(const SwmMacTableD& c) : SwmMacTable(c) {}
  SwmMacTableD(const FmMacTable&);
};

#endif
