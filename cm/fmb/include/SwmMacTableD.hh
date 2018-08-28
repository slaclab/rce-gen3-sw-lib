#ifndef Bali_SwmMacTableD_hh
#define Bali_SwmMacTableD_hh


#include "fmb/SwmMacTable.hh"

namespace Bali {
class SwmMacTableD : public SwmMacTable {
public:
  SwmMacTableD() {}
  SwmMacTableD(const SwmMacTableD& c) : SwmMacTable(c) {}
  SwmMacTableD(const FmMacTable&);
};
};

#endif
