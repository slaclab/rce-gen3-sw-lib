
#include "fmb/SwmMacTableD.hh"
#include "fmb/FmMacTable.hh"

using namespace Bali;

SwmMacTableD::SwmMacTableD(const FmMacTable& c)
{
  for(unsigned j=0; j<Entries; j++)
    entry[j] = c.direct_entry(j);
}
