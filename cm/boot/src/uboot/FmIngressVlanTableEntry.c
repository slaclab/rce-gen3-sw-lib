#include "../FmIngressVlanTableEntry.h"

void FmIngressVlanTableEntry_write(plx* _plx, FmIngressVlanTableEntry* this, SwmVlanTableEntry* entry)
{
  PLX_WRITE(_data[0],entry->_data[0]);
  PLX_WRITE(_data[1],entry->_data[1]);
  PLX_WRITE(_data[2],0);
  PLX_WRITE(_data[3],0);
}
