#ifndef FmEgressFidTableEntry_h
#define FmEgressFidTableEntry_h

#include "FmConfig.h"
#include "FmMReg.h"
#include "plx.h"

typedef struct {
  FmMReg _data;
} FmEgressFidTableEntry;

void FmEgressFidTableEntry_write(plx*, FmEgressFidTableEntry*, SwmFidTableEntry*);

#endif
