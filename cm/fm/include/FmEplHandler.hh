#ifndef FmEplHandler_hh
#define FmEplHandler_hh


#include "fm/FmHandler.hh"
#include "fm/FmPort.hh"

class FmTahoe;

class FmEplHandler : public FmHandler {
public:
  FmEplHandler(FmTahoe&);
  ~FmEplHandler();

  void enableOneLanePort (FmPort);
  void enableFourLanePort(FmPort);

  void call(unsigned);

private:
  FmTahoe& _dev;
  unsigned _oneLanePorts;
};

#endif
