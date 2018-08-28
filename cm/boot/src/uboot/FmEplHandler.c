

#include "../FmEplHandler.h"
#include "../plx.h"

#define MASK_INTERRUPTS

void FmEplHandler_enableFourLanePort(plx* _plx, FmEplHandler* epl, FmPort fmPort)
{
  unsigned p = fmPort-1;
  unsigned pmask = 1<<(p+1);
  FmTahoe* this  = epl->_dev;

#ifndef MASK_INTERRUPTS
  volatile unsigned v;
  v = PLX_READ(epl_phy[p].serdes_ip);  // clear out latched status
  v = PLX_READ(epl_phy[p].pcs_ip);     // clear out latched status
  PLX_WRITE(epl_phy[p].serdes_im,0);
  PLX_WRITE(epl_phy[p].pcs_im,3);     // sticky fault status
  PLX_WRITE(epl_phy[p].mac_im,0);
#else
  PLX_WRITE(epl_phy[p].serdes_im,0xfff);
  PLX_WRITE(epl_phy[p].pcs_im  ,0x7fff);
  PLX_WRITE(epl_phy[p].mac_im,0);
#endif

  epl->_oneLanePorts &= ~pmask;
}

void FmEplHandler_enableOneLanePort(plx* _plx, FmEplHandler* epl, FmPort fmPort)
{
  unsigned p = fmPort-1;
  unsigned pmask = 1<<(p+1);
  FmTahoe* this  = epl->_dev;

#ifndef MASK_INTERRUPTS
  volatile unsigned v;
  v = PLX_READ(epl_phy[p].serdes_ip);  // clear out latched status
  v = PLX_READ(epl_phy[p].pcs_ip);     // clear out latched status
  PLX_WRITE(epl_phy[p].serdes_im,0);
  PLX_WRITE(phy[p].pcs_im,0x43);  // sticky fault status and mis-aligned lanes
  PLX_WRITE(epl_phy[p].mac_im,0);
#else
  PLX_WRITE(epl_phy[p].serdes_im,0xfff);
  PLX_WRITE(epl_phy[p].pcs_im,0x7fff);
  PLX_WRITE(epl_phy[p].mac_im,0);
#endif

  epl->_oneLanePorts |= pmask;
}
