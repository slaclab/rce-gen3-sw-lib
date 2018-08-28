

#include "../FmEplRegs.h"

/* u-boot specific routine */
extern void udelay(unsigned long);

void FmEplRegs_control(plx*       _plx,
                       FmEplRegs* this,
                       unsigned   lanes,
                       unsigned   drive)
{
  //  unsigned ts = 50000; // 50 ms
  unsigned ts = 500; // 500 us

  PLX_WRITE(serdes_ctrl_2,0x3ff00); // initialize

  //  power up lanes
  volatile unsigned v = PLX_READ(serdes_ctrl_2);
  if (drive&0x100)
    v |= lanes;
  if (drive&0x200)
    v |= lanes<<4;
  v &= ~0xf000;
  v |= (0xf & ~lanes)<<12;
  PLX_WRITE(serdes_ctrl_2,v);
  if (lanes)
    udelay(ts);

  //  PLL reset
  v &= ~0x30000;
  if (!(lanes&0x3))
    v |= 0x10000;
  if (!(lanes&0xc))
    v |= 0x20000;
  PLX_WRITE(serdes_ctrl_2,v);
  if (lanes)
    udelay(ts);

  //  lane reset
  v &= ~0xf00;
  v |= (0xf & ~lanes)<<8;
  PLX_WRITE(serdes_ctrl_2,v);
  if (lanes)
    udelay(ts);

  // change max frame size to 9kB
  v = PLX_READ(mac_cfg_1);
  v &= 0xff000fff;
  v |= 0x00900000;
  PLX_WRITE(mac_cfg_1,v);

  PLX_RNW  (mac_cfg_2,~0x4);  // Rx Pause frames
  PLX_WRITE(mac_cfg_3,0x3ff); // Pause for 64kB duration
  PLX_WRITE(mac_cfg_4,0x3f0); // Resend pause after 63kB duration
}

static unsigned _encode_pcs_cfg_1( unsigned v, int lb, int rx_flip, int tx_flip)
{
  if (lb)      v |= (1<<14);
  if (rx_flip) v |= (1<<20);
  if (tx_flip) v |= (1<<19);
  return v;
}

void FmEplRegs_setup10Gb (plx* _plx, FmEplRegs* this, int lb, int rx_flip, int tx_flip, unsigned drive)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1(0x10034c00, lb, rx_flip, tx_flip));
  unsigned v = 0;
  v |= (drive&0xf)*0x1111;
  v |= ((drive&0xf0)<<12)*0x1111;
  PLX_WRITE(serdes_ctrl_1,v);
  FmEplRegs_control(_plx,this,0xf,drive);
  PLX_WRITE(serdes_test_mode,lb ? 0x68 : 0x60);
}

void FmEplRegs_setup4Gb (plx* _plx, FmEplRegs* this, int lb, int rx_flip, int tx_flip)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1(0x10034c00, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,0x88884444);
  FmEplRegs_control(_plx,this,0xf,0);
  PLX_WRITE(serdes_test_mode,lb ? 0x68 : 0x60);
}

//
//  For one lane modes, need to ignore certain frame errors
//
void FmEplRegs_setup2_5Gb(plx* _plx, FmEplRegs* this, int lb, int rx_flip, int tx_flip)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1 (0x3003081a, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,0x88884444);
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  FmEplRegs_control(_plx,this,v,0);
  PLX_WRITE(serdes_test_mode,lb ? 0x68 : 0x60);
}

void FmEplRegs_setup1Gb  (plx* _plx, FmEplRegs* this, int lb, int rx_flip, int tx_flip)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1 (0x2003081a, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,0x88884444);
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  FmEplRegs_control(_plx,this,v,0);
  PLX_WRITE(serdes_test_mode,lb ? 0x68 : 0x60);
}

void FmEplRegs_setup100Mb(plx* _plx, FmEplRegs* this, int lb, int rx_flip, int tx_flip)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1 (0x4003081a, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,0x88884444);
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  FmEplRegs_control(_plx,this,v,0);
  PLX_WRITE(serdes_test_mode,lb ? 0x68 : 0x60);
}

void FmEplRegs_setup10Mb (plx* _plx, FmEplRegs* this, int lb, int rx_flip, int tx_flip)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1 (0x6003081a, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,0);
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  FmEplRegs_control(_plx,this,v,0);
  PLX_WRITE(serdes_test_mode, lb ? 0x68 : 0x60);
}

void FmEplRegs_disable(plx* _plx, FmEplRegs* this)
{
  PLX_WRITE(serdes_ctrl_2,0x3ff00); // initialize
}
