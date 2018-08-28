#include "../FmEgressFidTableEntry.h"

void FmEgressFidTableEntry_write(plx* _plx, FmEgressFidTableEntry* this, SwmFidTableEntry* entry)
{
  unsigned forwarding = entry->forwarding;
  PLX_WRITE(_data,forwarding & ~1);
}
