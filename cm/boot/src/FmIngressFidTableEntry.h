#ifndef FmIngressFidTableEntry_h
#define FmIngressFidTableEntry_h

#include "FmConfig.h"
#include "FmMReg.h"
#include "plx.h"

typedef struct {
  FmMReg _data[2];
}  FmIngressFidTableEntry;

void FmIngressFidTableEntry_write(plx*, FmIngressFidTableEntry*, SwmFidTableEntry*);

#endif
