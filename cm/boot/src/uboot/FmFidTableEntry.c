

#include "../FmFidTableEntry.h"

void FmFidTableEntry_write(plx* _plx, FmFidTableEntry* this, SwmFidTableEntry* entry)
{
  unsigned d0,d1,parity,j;

  unsigned listening  = entry->listening;
  unsigned learning   = entry->learning;
  unsigned forwarding = entry->forwarding;
  listening  &= ~forwarding;
  learning   &= ~forwarding;
  listening  &= ~learning;
  listening  &= ((1<<24)-1)<<1;
  learning   &= ((1<<24)-1)<<1;
  forwarding &= ((1<<24)-1)<<1;

  d0 = 0;
  for(j=0; j<16; j++) {
    d0 |= (1*(listening&1) |
           2*(learning &1) |
           3*(forwarding&1)) << (2*j);
    listening >>= 1;
    learning  >>= 1;
    forwarding>>= 1;
  }

  d1 = 0;
  for(j=0; j<16; j++) {
    d1 |= (1*(listening&1) |
           2*(learning &1) |
           3*(forwarding&1)) << (j*2);
    listening >>= 1;
    learning  >>= 1;
    forwarding>>= 1;
  }
  
  parity = 0;
  j = d0 ^ d1;
  while ( j ) {
    if (j&1) parity++;
    j >>= 1;
  }
  d0 |= parity&1;

  PLX_WRITE(_data[0],d0);
  PLX_WRITE(_data[1],d1);
}
