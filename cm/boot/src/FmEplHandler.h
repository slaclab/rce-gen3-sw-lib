#ifndef FmEplHandler_h
#define FmEplHandler_h



#include "FmTahoe.h"
#include "FmPort.h"
#include "plx.h"

typedef struct {
  FmTahoe*   _dev;
  FmPortMask _oneLanePorts;
} FmEplHandler;

void FmEplHandler_enableOneLanePort (plx*,FmEplHandler*,FmPort);
void FmEplHandler_enableFourLanePort(plx*,FmEplHandler*,FmPort);

#endif
