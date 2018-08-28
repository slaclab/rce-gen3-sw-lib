#ifndef FmEgressVlanTableEntry_h
#define FmEgressVlanTableEntry_h

#include "FmConfig.h"
#include "FmMReg.h"
#include "plx.h"

typedef struct {
  FmMReg _data[2];
}  FmEgressVlanTableEntry;

void FmEgressVlanTableEntry_write(plx*, FmEgressVlanTableEntry*, SwmVlanTableEntry*);

#endif
