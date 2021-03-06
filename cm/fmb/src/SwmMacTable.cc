
#include "fmb/SwmMacTable.hh"

#include "fmb/FmMacTable.hh"
#include <stdio.h>

using namespace Bali;

SwmMacTable::SwmMacTable(const SwmMacTable& c)
{
  for(unsigned j=0; j<Entries; j++)
    entry[j] = c.entry[j];
}

SwmMacTable::SwmMacTable(const FmMacTable& c)
{
  for(unsigned j=0; j<Entries; j++)
    entry[j] = c.entry(j);
}

void SwmMacTable::clear()
{
  SwmMacTableEntry invalid; invalid.invalidate();
  for(unsigned k=0; k<Entries; k++)
    entry[k] = invalid;
}
