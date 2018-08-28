#ifndef FmMacHandler_hh
#define FmMacHandler_hh


#include "fm/FmHandler.hh"

class FmTahoe;

class FmMacHandler : public FmHandler {
public:
  FmMacHandler(FmTahoe*);

  void call(unsigned);

private:
  FmTahoe* _dev;
};

#endif
