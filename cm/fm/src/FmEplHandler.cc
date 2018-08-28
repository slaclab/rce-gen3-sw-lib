
#include "fm/FmEplHandler.hh"

#include "fm/FmTahoe.hh"

#include "cm_svc/Print.hh"

#include <stdio.h>
#include <stdint.h>

#define MASK_INTERRUPTS

static unsigned _lanes(unsigned q)
{
  return
    ((q>>0)&1) |
    ((q>>2)&2) |
    ((q>>4)&4) |
    ((q>>6)&8) ;
}

FmEplHandler::FmEplHandler(FmTahoe& d) :
  _dev(d),
  _oneLanePorts(0)
{
}

FmEplHandler::~FmEplHandler()
{
}

void FmEplHandler::enableFourLanePort(FmPort fmPort)
{
  unsigned p = unsigned(fmPort)-1;
  unsigned pmask = 1<<(p+1);
  _oneLanePorts &= ~pmask;

#ifndef MASK_INTERRUPTS
  volatile unsigned v;
  v = _dev.epl_phy[p].serdes_ip;  // clear out latched status
  v = _dev.epl_phy[p].pcs_ip;     // clear out latched status
  _dev.epl_phy[p].serdes_im = 0;
  _dev.epl_phy[p].pcs_im = 3;     // sticky fault status
  _dev.epl_phy[p].mac_im = 0;
#else
  _dev.epl_phy[p].serdes_im = 0xfff;
  _dev.epl_phy[p].pcs_im = 0x7fff;
  _dev.epl_phy[p].mac_im = 0;
#endif
}

void FmEplHandler::enableOneLanePort(FmPort fmPort)
{
  unsigned p = unsigned(fmPort)-1;
  unsigned pmask = 1<<(p+1);
  _oneLanePorts |= pmask;

#ifndef MASK_INTERRUPTS
  volatile unsigned v;
  v = _dev.epl_phy[p].serdes_ip;  // clear out latched status
  v = _dev.epl_phy[p].pcs_ip;     // clear out latched status
  _dev.epl_phy[p].serdes_im = 0;
  _dev.epl_phy[p].pcs_im = 0x43;  // sticky fault status and mis-aligned lanes
  _dev.epl_phy[p].mac_im = 0;
#else
  _dev.epl_phy[p].serdes_im = 0xfff;
  _dev.epl_phy[p].pcs_im = 0x7fff;
  _dev.epl_phy[p].mac_im = 0;
#endif
}

void FmEplHandler::call(unsigned p)
{
  using namespace cm::service;

  for(unsigned k=1; k<=24; k++) {
    if (p & (1<<k)) {
      FmEplRegs& r = _dev.epl_phy[k-1];
      unsigned v = r.epl_int_detect;
      printf("FmEplHandler port %x/%x %x\n",_dev.id(),k-1,v);

      if (v & 1) {
	unsigned q = r.serdes_ip;
	printv("  serdes_ip");
	if (q & 0x924) printv("    disparity error lanes  %x",_lanes(q>>2));
	if (q & 0x492) printv("    out-of-band char lanes %x",_lanes(q>>1));
	if (q & 0x249) printv("    loss of signal lanes   %x",_lanes(q));
	r.serdes_im = q & 0xfff;
      }
      if (v & 2) {
	unsigned q = r.pcs_ip;
	printv("  pcs");
	if (q & 0x4000) printv("    fault status change");
	if (q & 0x2000) printv("    link up");
	if (q & 0x1000) printv("    link went up");
	if (q & 0x0800) printv("    link went down");
	if (q & 0x0780) printv("    fifo overflow lanes %x",(q>>7)&0xf);
	if (q & 0x0040) printv("    lanes misaligned");
	if (q & 0x0020) printv("    fsig sent");
	if (q & 0x0010) printv("    rf sent");
	if (q & 0x0008) printv("    lf sent");
	if (q & 0x0004) printv("    fsig detected");
	if (q & 0x0002) printv("    rf detected");
	if (q & 0x0001) printv("    lf detected");

	unsigned m = r.pcs_im;
	if (q == 0) {
	}

	if ((q&0x7ffc) == 0x2000) { // link is up, reinstate all other interrupts
	  //	  volatile unsigned pp = r.serdes_ip;
	  //	  r.serdes_im = 0;
	  m = 0x2003;
	}
	else if (q) {  // mask off discovered conditions until the link comes back up
	  //	  m |= q & 0x07C0;  // mask the fifo overflows and lanes misaligned
	  m = (m | q) & ~0x2000;
	}
	else {
	  printv("    spurious (mask=%x), masking all sources)",m);
	  m = 0x7FFF;
	}
	if (_oneLanePorts & (1<<k))
	  m |= 0x40;
	  //	  m |= 0x7FFF;

	r.pcs_im = m;
	if (!q) {
	  m = r.pcs_im;
	  printv("    mask is now %x",m);
	}
      }
      if (v & 4) {
	unsigned q = r.mac_ip;
	printv("  mac");
	if (q & 0x400) printv("    fabric error");
	if (q & 0x200) printv("    rx pause en deasserted");
	if (q & 0x100) printv("    tx uflow");
	if (q & 0x080) printv("    tx crc wo rx crc error");
	if (q & 0x040) printv("    tx crc error");
	if (q & 0x020) printv("    rx phy error");
	if (q & 0x010) printv("    rx pause oflow");
	if (q & 0x008) printv("    rx oversize error");
	if (q & 0x004) printv("    rx crc error");
	if (q & 0x002) printv("    overflow error");
	if (q & 0x001) printv("    rx runt error");
	r.mac_im = q & 0x4ff;
      }
    }
  }
}
