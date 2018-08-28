#ifndef Bali_FmMacTcnFifoEntry_hh
#define Bali_FmMacTcnFifoEntry_hh


#include "fm/FmMReg.hh"

namespace Bali {
  class SwmMacTcnFifoEntry;

  class FmMacTcnFifoEntry {
  public:
    FmMacTcnFifoEntry() {}
    FmMacTcnFifoEntry(const SwmMacTcnFifoEntry&);
    ~FmMacTcnFifoEntry() {}

    FmMacTcnFifoEntry& operator=(const SwmMacTcnFifoEntry&);

    operator SwmMacTcnFifoEntry() const;

  private:
    friend class SwmMacTableEntry;
    FmMReg _data[4];
  };
};

#endif
