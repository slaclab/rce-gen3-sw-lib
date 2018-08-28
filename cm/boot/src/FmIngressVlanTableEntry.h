#ifndef FmIngressVlanTableEntry_h
#define FmIngressVlanTableEntry_h

#include "FmConfig.h"
#include "FmMReg.h"
#include "plx.h"

typedef struct {
  FmMReg _data[4];
}  FmIngressVlanTableEntry;

void FmIngressVlanTableEntry_write(plx*, FmIngressVlanTableEntry*, SwmVlanTableEntry*);

#endif
