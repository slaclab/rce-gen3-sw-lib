
#include "fmb/SwmEplRegs.hh"

#include "fmb/FmEplRegs.hh"

#include <string.h>

using namespace Bali;

SwmEplRegs::SwmEplRegs()
{
}


SwmEplRegs::~SwmEplRegs()
{
}

SwmEplRegs& SwmEplRegs::operator=(const SwmEplRegs& c)
{
  memcpy(this, &c, sizeof(c));
  return *this;
}
