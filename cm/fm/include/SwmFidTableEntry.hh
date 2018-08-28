#ifndef SwmFidTableEntry_hh
#define SwmFidTableEntry_hh


#include "fm/FmPort.hh"

class SwmFidTableEntry {
public:
  SwmFidTableEntry();
  SwmFidTableEntry(FmPortMask listening,
		   FmPortMask learning,
		   FmPortMask forwarding);

  SwmFidTableEntry& operator=(const SwmFidTableEntry&);

  FmPortMask       listening () const { return _listening; }
  FmPortMask       learning  () const { return _learning; }
  FmPortMask       forwarding() const { return _forwarding; }

private:
  FmPortMask _listening, _learning, _forwarding;
};

#endif
