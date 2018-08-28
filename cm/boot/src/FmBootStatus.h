#ifndef FmBootStatus_h
#define FmBootStatus_h



#include "FmSReg.h"

enum { Boot_Running          =1<<0,
       EEPROM_Running        =1<<2,
       Memory_Initialization =1<<3,
       Fusebox_Processing    =1<<4 };

typedef FmSReg FmBootStatus;

#endif
