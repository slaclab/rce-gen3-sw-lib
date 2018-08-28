#include "../FmEgressVlanTableEntry.h"

void FmEgressVlanTableEntry_write(plx* _plx, FmEgressVlanTableEntry* this, SwmVlanTableEntry* entry)
{
  unsigned v = SwmVlanTableEntry_members(entry);
  PLX_WRITE(_data[0],v<<4);
  PLX_WRITE(_data[1],0);
}
