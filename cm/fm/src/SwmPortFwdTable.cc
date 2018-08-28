
#include "fm/SwmPortFwdTable.hh"


SwmPortFwdTable::SwmPortFwdTable(const SwmPortFwdTable& c)
{
  for(unsigned i=0; i<FmPort::MAX_PORT; i++)
    forward[i] = c.forward[i];
}
