
#include "fm/SwmMacTableD.hh"
#include "fm/FmMacTable.hh"

SwmMacTableD::SwmMacTableD(const FmMacTable& c)
{
  for(unsigned j=0; j<Entries; j++)
    entry[j] = c.direct_entry(j);
}
