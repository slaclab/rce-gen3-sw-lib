

#include "../FmIngressFidTableEntry.h"

void FmIngressFidTableEntry_write(plx* _plx, FmIngressFidTableEntry* this, SwmFidTableEntry* entry)
{
  unsigned d0,d1,j;

  unsigned listening  = entry->listening;
  unsigned learning   = entry->learning;
  unsigned forwarding = entry->forwarding;
  listening  &= ~forwarding;
  learning   &= ~forwarding;
  listening  &= ~learning;
  listening  &= PHY_PORTS;
  learning   &= PHY_PORTS;
  forwarding &= PHY_PORTS;

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
  
  PLX_WRITE(_data[0],d0);
  PLX_WRITE(_data[1],d1);
}
