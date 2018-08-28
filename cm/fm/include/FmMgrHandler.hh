#ifndef FmMgrHandler_hh
#define FmMgrHandler_hh


#include "fm/FmHandler.hh"

class FmTahoe;

class FmMgrHandler : public FmHandler {
public:
  FmMgrHandler(FmTahoe*);
  void call(unsigned);

private:
  FmTahoe* _dev;
};

#endif
