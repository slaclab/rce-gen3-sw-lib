#ifndef SwmPortFwdTable_hh
#define SwmPortFwdTable_hh


#include "fm/FmPort.hh"

class SwmPortFwdTable {
public:
  SwmPortFwdTable() {}
  SwmPortFwdTable(const SwmPortFwdTable&);
  ~SwmPortFwdTable() {}

  FmPortMask forward[FmPort::MAX_PORT];
};

#endif
