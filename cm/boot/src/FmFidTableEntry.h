#ifndef FmFidTableEntry_h
#define FmFidTableEntry_h



#include "FmConfig.h"
#include "FmMReg.h"
#include "plx.h"

typedef struct {
  FmMReg _data[2];
} FmFidTableEntry;

void FmFidTableEntry_write(plx*, FmFidTableEntry*, SwmFidTableEntry*);

#endif
