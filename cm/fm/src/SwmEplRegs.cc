
#include "fm/SwmEplRegs.hh"

#include "fm/FmEplRegs.hh"

SwmEplRegs::SwmEplRegs()
{
}

#if 0
SwmEplRegs::SwmEplRegs( const FmEplRegs& c )
{
  //
  //  Read the appropriate registers from this block
  //
  serdes_ctrl_1 = c.serdes_ctrl_1;
  serdes_ctrl_2 = c.serdes_ctrl_2;
  serdes_ctrl_3 = c.serdes_ctrl_3;
  serdes_test_mode = c.serdes_test_mode;
  serdes_status = c.serdes_status;
  serdes_ip = c.serdes_ip;
  serdes_im = c.serdes_im;
  serdes_bist_err_cnt = c.serdes_bist_err_cnt;
  pcs_cfg_1 = c.pcs_cfg_1;
  pcs_cfg_2 = c.pcs_cfg_2;
  pcs_cfg_3 = c.pcs_cfg_3;
  pcs_cfg_4 = c.pcs_cfg_4;
  pcs_cfg_5 = c.pcs_cfg_5;
  pcs_ip = c.pcs_ip;
  pcs_im = c.pcs_im;
  for(unsigned j=0; j<8; j++)
    pacing_pri_wm[j] = c.pacing_pri_wm[j];
  pacing_rate = c.pacing_rate;
  pacing_status = c.pacing_status;
  mac_cfg_1 = c.mac_cfg_1;
  mac_cfg_2 = c.mac_cfg_2;
  mac_cfg_3 = c.mac_cfg_3;
  mac_cfg_4 = c.mac_cfg_4;
  mac_cfg_5 = c.mac_cfg_5;
  mac_cfg_6 = c.mac_cfg_6;
  tx_pri_map_1 = c.tx_pri_map_1;
  tx_pri_map_2 = c.tx_pri_map_2;
  mac_status = c.mac_status;
  mac_ip = c.mac_ip;
  mac_im = c.mac_im;
  stat_epl_error1 = c.stat_epl_error1;
  stat_tx_pause = c.stat_tx_pause;
  stat_tx_crc = c.stat_tx_crc;
  stat_epl_error2 = c.stat_epl_error2;
  stat_rx_jabber = c.stat_rx_jabber;
  epl_led_status = c.epl_led_status;
  epl_int_detect = c.epl_int_detect;
  stat_tx_bytecount = c.stat_tx_bytecount;
}
#endif

SwmEplRegs::~SwmEplRegs()
{
}

SwmEplRegs& SwmEplRegs::operator=(const SwmEplRegs& c)
{
  serdes_ctrl_1 = c.serdes_ctrl_1;
  serdes_ctrl_2 = c.serdes_ctrl_2;
  serdes_ctrl_3 = c.serdes_ctrl_3;
  serdes_test_mode = c.serdes_test_mode;
  serdes_status = c.serdes_status;
  serdes_ip = c.serdes_ip;
  serdes_im = c.serdes_im;
  serdes_bist_err_cnt = c.serdes_bist_err_cnt;
  pcs_cfg_1 = c.pcs_cfg_1;
  pcs_cfg_2 = c.pcs_cfg_2;
  pcs_cfg_3 = c.pcs_cfg_3;
  pcs_cfg_4 = c.pcs_cfg_4;
  pcs_cfg_5 = c.pcs_cfg_5;
  pcs_ip = c.pcs_ip;
  pcs_im = c.pcs_im;
  for(unsigned j=0; j<8; j++)
    pacing_pri_wm[j] = c.pacing_pri_wm[j];
  pacing_rate = c.pacing_rate;
  pacing_status = c.pacing_status;
  mac_cfg_1 = c.mac_cfg_1;
  mac_cfg_2 = c.mac_cfg_2;
  mac_cfg_3 = c.mac_cfg_3;
  mac_cfg_4 = c.mac_cfg_4;
  mac_cfg_5 = c.mac_cfg_5;
  mac_cfg_6 = c.mac_cfg_6;
  tx_pri_map_1 = c.tx_pri_map_1;
  tx_pri_map_2 = c.tx_pri_map_2;
  mac_status = c.mac_status;
  mac_ip = c.mac_ip;
  mac_im = c.mac_im;
  stat_epl_error1 = c.stat_epl_error1;
  stat_tx_pause = c.stat_tx_pause;
  stat_tx_crc = c.stat_tx_crc;
  stat_epl_error2 = c.stat_epl_error2;
  stat_rx_jabber = c.stat_rx_jabber;
  epl_led_status = c.epl_led_status;
  epl_int_detect = c.epl_int_detect;
  stat_tx_bytecount = c.stat_tx_bytecount;

  return *this;
}
