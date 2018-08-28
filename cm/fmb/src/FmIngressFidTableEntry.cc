
#include "fmb/FmIngressFidTableEntry.hh"

#include "fm/SwmFidTableEntry.hh"

using namespace Bali;

FmIngressFidTableEntry::FmIngressFidTableEntry(const SwmFidTableEntry& e)
{
  *this = e;
}


FmIngressFidTableEntry::operator SwmFidTableEntry() const
{
  FmPortMask listening(0);
  FmPortMask learning(0);
  FmPortMask forwarding(0);

  unsigned d0 = _data[0] & ~3;
  unsigned d1 = _data[1];
  unsigned k=0;
  for(unsigned j=0; j<32; j+=2, k++) {
    unsigned b = (d0>>j)&3;
    if      (b==1) listening  |= (1<<k);
    else if (b==2) learning   |= (1<<k);
    else if (b==3) forwarding |= (1<<k);
  }
  for(unsigned j=0; j<32; j+=2, k++) {
    unsigned b = (d1>>j)&3;
    if      (b==1) listening  |= (1<<k);
    else if (b==2) learning   |= (1<<k);
    else if (b==3) forwarding |= (1<<k);
  }

  return SwmFidTableEntry( listening, learning, forwarding );
}

FmIngressFidTableEntry& FmIngressFidTableEntry::operator=(const SwmFidTableEntry& e)
{
  unsigned listening = e.listening();
  unsigned learning = e.learning();
  unsigned forwarding = e.forwarding();

  // The states are mutually exclusive
  // Give priority to the later states
  listening &= ~forwarding;
  learning  &= ~forwarding;
  listening &= ~learning;

  // The cpu port does not participate
  listening  &= 0x1fffffe;
  learning   &= 0x1fffffe;
  forwarding &= 0x1fffffe;

  unsigned d0 = 0;
  for(unsigned j=0; j<16; j++) {
    d0 |= (1*(listening&1) |
	   2*(learning &1) |
	   3*(forwarding&1)) << (2*j);
    listening >>= 1;
    learning  >>= 1;
    forwarding>>= 1;
  }

  unsigned d1 = 0;
  for(unsigned j=0; j<16; j++) {
    d1 |= (1*(listening&1) |
	   2*(learning &1) |
	   3*(forwarding&1)) << (j*2);
    listening >>= 1;
    learning  >>= 1;
    forwarding>>= 1;
  }

  _data[0] = d0;
  _data[1] = d1;
  return *this;
}

