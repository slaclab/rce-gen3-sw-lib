

#include "../FmEplRegsB.h"

/* u-boot specific routine */
extern void udelay(unsigned long);

void FmEplRegsB_control(plx*       _plx,
                       FmEplRegsB* this,
                       unsigned   lanes,
                       unsigned   drive)
{
  //  unsigned ts = 50000; // 50 ms
  unsigned ts = 50; // 50 us
  unsigned l_01 = lanes&0x3;
  unsigned l_23 = lanes&0xc;

  //  Power up A/B
  //  Wait 10us
  //  Take PLL_AB out of reset
  //  Wait 10us
  //  Take lane A/B out of reset
  //  Wait 10us
  //  Repeat for C/D

  PLX_WRITE(serdes_ctrl_2,0x3ff00); // initialize

  PLX_WRITE(syncbuf_cfg,0x3f00);

  //  power up lanes
  volatile unsigned v = PLX_READ(serdes_ctrl_2);
  if (drive&0x100)
    v |= lanes;
  if (drive&0x200)
    v |= lanes<<4;

  if (l_01) {
    v &= ~(l_01<<12);   // Power up lanes 0,1
    PLX_WRITE(serdes_ctrl_2,v);
    udelay(ts);

    v &= ~(1<<16);      // Take lanes 0,1 PLL out of reset
    PLX_WRITE(serdes_ctrl_2,v);
    udelay(ts);

    v &= ~(l_01<<8);    // Take lanes 0,1 out of reset
    PLX_WRITE(serdes_ctrl_2,v);
    udelay(ts);
  }

  if (l_23) {
    v &= ~(l_23<<12);   // Power up lanes 2,3
    PLX_WRITE(serdes_ctrl_2,v);
    udelay(ts);

    v &= ~(1<<17);      // Take lanes 2,3 PLL out of reset
    PLX_WRITE(serdes_ctrl_2,v);
    udelay(ts);

    v &= ~(l_23<<8);    // Take lanes 2,3 out of reset
    PLX_WRITE(serdes_ctrl_2,v);
    udelay(ts);
  }

  // change max frame size to 9kB
  v = PLX_READ(mac_cfg_1);
  v &= 0xff000fff;
  v |= 0x00900000;
  PLX_WRITE(mac_cfg_1,v);

  //  PLX_RNW  (mac_cfg_2,~0x4);  // Rx Pause frames
  PLX_RNW  (mac_cfg_2,~0x70450);  // Don't modify outgoing frames, + errata 53
  PLX_WRITE(mac_cfg_3,0x401003ff); // Pause for 64kB duration
  //  PLX_WRITE(mac_cfg_4,0x3f0); // Resend pause after 63kB duration
}

static unsigned _encode_pcs_cfg_1( unsigned v, int lb, int rx_flip, int tx_flip)
{
  if (lb)      v |= (1<<14);
  if (rx_flip) v |= (1<<20);
  if (tx_flip) v |= (1<<19);
  return v;
}

void FmEplRegsB_setup10Gb (plx* _plx, FmEplRegsB* this, int lb, int rx_flip, int tx_flip, unsigned drive)
{
  unsigned v = 0;
  v |= (drive&0xf)*0x1111;
  v |= ((drive&0xf0)<<12)*0x1111;
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1(0x10034c00, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,v);
  FmEplRegsB_control(_plx,this,0xf,drive);
  PLX_WRITE(serdes_test_mode,lb ? 0x68 : 0x60);
}

void FmEplRegsB_setup4Gb (plx* _plx, FmEplRegsB* this, int lb, int rx_flip, int tx_flip)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1(0x10034c00, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,0x88884444);
  FmEplRegsB_control(_plx,this,0xf,0);
  PLX_WRITE(serdes_test_mode,lb ? 0x68 : 0x60);
}

//
//  For one lane modes, need to ignore certain frame errors
//
void FmEplRegsB_setup2_5Gb(plx* _plx, FmEplRegsB* this, int lb, int rx_flip, int tx_flip)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1 (0x3003081a, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,0x88884444);
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  FmEplRegsB_control(_plx,this,v,0);
  PLX_WRITE(serdes_test_mode,lb ? 0x68 : 0x60);
}

void FmEplRegsB_setup1Gb  (plx* _plx, FmEplRegsB* this, int lb, int rx_flip, int tx_flip)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1 (0x2003081a, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,0x88884444);
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  FmEplRegsB_control(_plx,this,v,0);
  PLX_WRITE(serdes_test_mode,lb ? 0x68 : 0x60);
}

void FmEplRegsB_setup100Mb(plx* _plx, FmEplRegsB* this, int lb, int rx_flip, int tx_flip)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1 (0x4003081a, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,0x88884444);
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  FmEplRegsB_control(_plx,this,v,0);
  PLX_WRITE(serdes_test_mode,lb ? 0x68 : 0x60);
}

void FmEplRegsB_setup10Mb (plx* _plx, FmEplRegsB* this, int lb, int rx_flip, int tx_flip)
{
  PLX_WRITE(pcs_cfg_1,_encode_pcs_cfg_1 (0x6003081a, lb, rx_flip, tx_flip));
  PLX_WRITE(serdes_ctrl_1,0);
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  FmEplRegsB_control(_plx,this,v,0);
  PLX_WRITE(serdes_test_mode, lb ? 0x68 : 0x60);
}

void FmEplRegsB_disable(plx* _plx, FmEplRegsB* this)
{
  PLX_WRITE(serdes_ctrl_2,0x3ff00); // initialize
}
