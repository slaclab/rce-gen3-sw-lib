#ifndef FmMReg_h
#define FmMReg_h



#include "FmReg.h"

typedef FmReg FmMReg;

#define PLX_RNW(reg,mask) PLX_WRITE(reg,(PLX_READ(reg)&mask))
#define PLX_ROW(reg,mask) PLX_WRITE(reg,(PLX_READ(reg)|mask))
#define PLX_SETB(reg,mask,b) PLX_WRITE(reg,((PLX_READ(reg)&~mask)|(b&mask)))

#endif
