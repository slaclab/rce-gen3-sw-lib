#ifndef SwmEplRegsEplRegs_hh
#define SwmEplRegsEplRegs_hh


#include "fm/SwmMibCounter.hh"

class FmEplRegs;

class SwmEplRegs {
public:
  SwmEplRegs();
  //  SwmEplRegs( const FmEplRegs& );
  ~SwmEplRegs();

  SwmEplRegs& operator=(const SwmEplRegs&);

  unsigned       serdes_ctrl_1;
  unsigned       serdes_ctrl_2;
  unsigned       serdes_ctrl_3;
  unsigned       serdes_test_mode;
  unsigned       serdes_status;
  unsigned       serdes_ip;
  unsigned       serdes_im;
  unsigned       reserved1;
  unsigned       serdes_bist_err_cnt;
  unsigned       pcs_cfg_1;
  unsigned       pcs_cfg_2;
  unsigned       pcs_cfg_3;
  unsigned       pcs_cfg_4;
  unsigned       pcs_cfg_5;
  unsigned       pcs_ip;
  unsigned       pcs_im;
  unsigned       pacing_pri_wm[8];
  unsigned       pacing_rate;
  unsigned       pacing_status;
  unsigned       mac_cfg_1;
  unsigned       mac_cfg_2;
  unsigned       mac_cfg_3;
  unsigned       mac_cfg_4;
  unsigned       mac_cfg_5;
  unsigned       mac_cfg_6;
  unsigned       tx_pri_map_1;
  unsigned       tx_pri_map_2;
  unsigned       mac_status;
  unsigned       mac_ip;
  unsigned       mac_im;
  unsigned       stat_epl_error1;
  unsigned       stat_tx_pause;
  unsigned       stat_tx_crc;
  unsigned       stat_epl_error2;
  unsigned       stat_rx_jabber;
  unsigned       epl_led_status;
  unsigned       epl_int_detect;
  unsigned       stat_tx_bytecount;
  unsigned       reserved3[0x3d3];
};

#endif
