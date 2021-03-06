
#include "fmb/FmEplRegs.hh"

#include "fmb/SwmEplRegs.hh"

#include <time.h>

using namespace Bali;

FmEplRegs& FmEplRegs::operator=( const SwmEplRegs& c )
{
  //
  //  Write the appropriate registers from this block
  //
  serdes_ctrl_1 = c.serdes_ctrl_1;
  serdes_ctrl_2 = c.serdes_ctrl_2;
  serdes_ctrl_3 = c.serdes_ctrl_3;
  serdes_test_mode = c.serdes_test_mode;
  //  serdes_status = c.serdes_status;
  serdes_ip = c.serdes_ip;
  serdes_im = c.serdes_im;
  //  serdes_bist_err_cnt = c.serdes_bist_err_cnt;
  pcs_cfg_1 = c.pcs_cfg_1;
  pcs_cfg_2 = c.pcs_cfg_2;
  pcs_cfg_3 = c.pcs_cfg_3;
  pcs_cfg_4 = c.pcs_cfg_4;
  pcs_cfg_5 = c.pcs_cfg_5;
  pcs_ip = c.pcs_ip;
  pcs_im = c.pcs_im;
  syncbuf_cfg = c.syncbuf_cfg;
  mac_cfg_1 = c.mac_cfg_1;
  mac_cfg_2 = c.mac_cfg_2;
  mac_cfg_3 = c.mac_cfg_3;
  mac_cfg_5 = c.mac_cfg_5;
  mac_cfg_6 = c.mac_cfg_6;
  tx_vpri_map_1 = c.tx_vpri_map_1;
  tx_vpri_map_2 = c.tx_vpri_map_2;
  //  mac_status = c.mac_status;
  mac_ip = c.mac_ip;
  mac_im = c.mac_im;
  //  stat_epl_error1 = c.stat_epl_error1;
  stat_tx_pause = c.stat_tx_pause;
  stat_tx_crc = c.stat_tx_crc;
  //  stat_epl_error2 = c.stat_epl_error2;
  stat_rx_jabber = c.stat_rx_jabber;
  epl_led_status = c.epl_led_status;
  //  epl_int_detect = c.epl_int_detect;
  //  stat_tx_bytecount = c.stat_tx_bytecount;

  return* this;
}

FmEplRegs::operator SwmEplRegs() const
{
  timespec ts;
  ts.tv_sec = 0;
  //  ts.tv_nsec = 20000; // Try a 20 usec delay between register reads
  ts.tv_nsec = 0;

  const FmEplRegs& c = *this;
  SwmEplRegs v;
  //
  //  Read the appropriate registers from this block
  //
  v.serdes_ctrl_1 = c.serdes_ctrl_1;  nanosleep(&ts, 0);
  v.serdes_ctrl_2 = c.serdes_ctrl_2;  nanosleep(&ts, 0);
  v.serdes_ctrl_3 = c.serdes_ctrl_3;  nanosleep(&ts, 0);
  v.serdes_test_mode = c.serdes_test_mode;  nanosleep(&ts, 0);
  v.serdes_status = c.serdes_status;  nanosleep(&ts, 0);
  v.serdes_ip = c.serdes_ip;  nanosleep(&ts, 0);
  v.serdes_im = c.serdes_im;  nanosleep(&ts, 0);
  v.serdes_bist_err_cnt = c.serdes_bist_err_cnt;  nanosleep(&ts, 0);
  v.pcs_cfg_1 = c.pcs_cfg_1;  nanosleep(&ts, 0);
  v.pcs_cfg_2 = c.pcs_cfg_2;  nanosleep(&ts, 0);
  v.pcs_cfg_3 = c.pcs_cfg_3;  nanosleep(&ts, 0);
  v.pcs_cfg_4 = c.pcs_cfg_4;  nanosleep(&ts, 0);
  v.pcs_cfg_5 = c.pcs_cfg_5;  nanosleep(&ts, 0);
  v.pcs_ip = c.pcs_ip;  nanosleep(&ts, 0);
  v.pcs_im = c.pcs_im;  nanosleep(&ts, 0);
  v.syncbuf_cfg = c.syncbuf_cfg;  nanosleep(&ts, 0);
  v.mac_cfg_1 = c.mac_cfg_1;  nanosleep(&ts, 0);
  v.mac_cfg_2 = c.mac_cfg_2;  nanosleep(&ts, 0);
  v.mac_cfg_3 = c.mac_cfg_3;  nanosleep(&ts, 0);
  //  v.mac_cfg_4 = c.mac_cfg_4;  nanosleep(&ts, 0);
  v.mac_cfg_5 = c.mac_cfg_5;  nanosleep(&ts, 0);
  v.mac_cfg_6 = c.mac_cfg_6;  nanosleep(&ts, 0);
  v.tx_vpri_map_1 = c.tx_vpri_map_1;  nanosleep(&ts, 0);
  v.tx_vpri_map_2 = c.tx_vpri_map_2;  nanosleep(&ts, 0);
  v.mac_status = c.mac_status;  nanosleep(&ts, 0);
  v.mac_ip = c.mac_ip;  nanosleep(&ts, 0);
  v.mac_im = c.mac_im;  nanosleep(&ts, 0);
  v.stat_epl_error1 = c.stat_epl_error1;  nanosleep(&ts, 0);
  v.stat_tx_pause = c.stat_tx_pause;  nanosleep(&ts, 0);
  v.stat_tx_crc = c.stat_tx_crc;  nanosleep(&ts, 0);
  v.stat_epl_error2 = c.stat_epl_error2;  nanosleep(&ts, 0);
  v.stat_rx_jabber = c.stat_rx_jabber;  nanosleep(&ts, 0);
  v.epl_led_status = c.epl_led_status;  nanosleep(&ts, 0);
  v.epl_int_detect = c.epl_int_detect;  nanosleep(&ts, 0);
  v.stat_tx_bytecount = c.stat_tx_bytecount;  nanosleep(&ts, 0);

  //
  //  Clear the registers with a write (to mimic Tahoe registers that cleared on read)
  //
  FmEplRegs& s = *const_cast<FmEplRegs*>(this);
  s.serdes_ip=0xfff;
  s.pcs_ip   =0xffffffff;
  s.mac_ip   =0xfff;
  s.stat_epl_error1=0xffff;
  s.stat_epl_error2=0xffff;

  return v;
}

unsigned FmEplRegs::ds() const
{
  unsigned v = pcs_cfg_1;
  return (v>>29)&3;
}

void FmEplRegs::_control(unsigned lanes,
			 unsigned drive)
{
  timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 50000; // comments say only 500 ns, but I don't trust

  serdes_ctrl_2 = 0x3ff00; // initialize

  syncbuf_cfg = 0x3f00;

  //  power up lanes
  volatile unsigned v = serdes_ctrl_2;
  if (drive&0x100)
    v |= lanes;
  if (drive&0x200)
    v |= lanes<<4;

  unsigned l_01 = lanes&3;
  if (l_01) {
    v &= ~(l_01<<12);   // Power up lanes 0,1
    serdes_ctrl_2 = v;
    nanosleep(&ts,0);

    v &= ~(1<<16);      // Take lanes 0,1 PLL out of reset
    serdes_ctrl_2 = v;
    nanosleep(&ts,0);

    v &= ~(l_01<<8);    // Take lanes 0,1 out of reset
    serdes_ctrl_2 = v;
    nanosleep(&ts,0);
  }

  unsigned l_23 = lanes&0xc;
  if (l_23) {
    v &= ~(l_23<<12);   // Power up lanes 2,3
    serdes_ctrl_2 = v;
    nanosleep(&ts,0);

    v &= ~(1<<17);      // Take lanes 2,3 PLL out of reset
    serdes_ctrl_2 = v;
    nanosleep(&ts,0);

    v &= ~(l_23<<8);    // Take lanes 2,3 out of reset
    serdes_ctrl_2 = v;
    nanosleep(&ts,0);
  }

  // change max frame size to 9kB
  v = mac_cfg_1;
  v &= 0xff000fff;
  v |= 0x00900000;
  mac_cfg_1 = v;

  //mac_cfg_2 &= ~0x4; // Parse Rx Pause frames
  mac_cfg_2 &= ~0x70450; // Don't modify outgoing frames, disable error discards (errata 53)
  mac_cfg_3 = 0x401003ff; // Pause for 64kB duration
  //  mac_cfg_4 = 0x3f0; // Resend pause after 63kB duration
}

static unsigned _encode_pcs_cfg_1( unsigned v, int lb, int rx_flip, int tx_flip)
{
  if (lb)      v |= (1<<14);
  if (rx_flip) v |= (1<<20);
  if (tx_flip) v |= (1<<19);
  return v;
}

void FmEplRegs::setup10Gb (int lb, int rx_flip, int tx_flip, unsigned drive)
{
  pcs_cfg_1 = _encode_pcs_cfg_1(0x10034c00, lb, rx_flip, tx_flip);
  unsigned v = 0;
  v |= (drive&0xf)*0x1111;
  v |= ((drive&0xf0)<<12)*0x1111;
  serdes_ctrl_1 = v;
  _control(0xf,drive);
  serdes_test_mode = lb ? 0x68 : 0x60;
}

void FmEplRegs::setup4Gb (int lb, int rx_flip, int tx_flip)
{
  pcs_cfg_1 = _encode_pcs_cfg_1(0x10034c00, lb, rx_flip, tx_flip);
  serdes_ctrl_1 = 0x88884444;
  _control(0xf);
  serdes_test_mode = lb ? 0x68 : 0x60;
}

//
//  For one lane modes, need to ignore certain frame errors
//
void FmEplRegs::setup2_5Gb(int lb, int rx_flip, int tx_flip)
{
  pcs_cfg_1 = _encode_pcs_cfg_1 (0x3003081a, lb, rx_flip, tx_flip);
  serdes_ctrl_1 = 0x88884444;
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  _control(v);
  serdes_test_mode = lb ? 0x68 : 0x60;
}

void FmEplRegs::setup1Gb  (int lb, int rx_flip, int tx_flip)
{
  pcs_cfg_1 = _encode_pcs_cfg_1 (0x2003081a, lb, rx_flip, tx_flip);
  serdes_ctrl_1 = 0x88884444;
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  _control(v);
  serdes_test_mode = lb ? 0x68 : 0x60;
}

void FmEplRegs::setup100Mb(int lb, int rx_flip, int tx_flip)
{
  pcs_cfg_1 = _encode_pcs_cfg_1 (0x4003081a, lb, rx_flip, tx_flip);
  serdes_ctrl_1 = 0x88884444;
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  _control(v);
  serdes_test_mode = lb ? 0x68 : 0x60;
}

void FmEplRegs::setup10Mb (int lb, int rx_flip, int tx_flip)
{
  pcs_cfg_1 = _encode_pcs_cfg_1 (0x6003081a, lb, rx_flip, tx_flip);
  serdes_ctrl_1 = 0;
  unsigned v = ( rx_flip ? 8 : 1 ) | ( tx_flip ? 8 : 1 );
  _control(v);
  serdes_test_mode = lb ? 0x68 : 0x60;
}

void FmEplRegs::disable()
{
  serdes_ctrl_2 = 0x3ff00; // initialize
}

FmEplRegs::Speed FmEplRegs::speed() const
{
  unsigned v = serdes_ctrl_2;
  if (v != 0x3ff00) {
    v = pcs_cfg_1;
    switch((v>>28)&0x7) {
    case 1: return X4Lanes;
    case 2: return X1Gb;
    case 3: return X2_5Gb;
    case 4: return X100Mb;
    case 6: return X10Mb;
    default: break;
    }
  }
  return Disabled;
}

bool FmEplRegs::rx_flip() const
{
  unsigned v = pcs_cfg_1;
  return v&(1<<20);
}

bool FmEplRegs::tx_flip() const
{
  unsigned v = pcs_cfg_1;
  return v&(1<<19);
}
