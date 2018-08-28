#ifndef FmBootStatus_hh
#define FmBootStatus_hh


#include "fm/FmSReg.hh"

class FmBootStatus : public FmSReg {
public:
  enum { Boot_Running          =1<<0,
	 EEPROM_Running        =1<<2,
	 Memory_Initialization =1<<3,
	 Fusebox_Processing    =1<<4 };
};

#endif
