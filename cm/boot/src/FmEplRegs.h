#ifndef FmEplRegs_h
#define FmEplRegs_h



#include "FmMibCounter.h"
#include "FmMReg.h"
#include "plx.h"

typedef struct {
  FmMReg       serdes_ctrl_1;  // 0x8000
  FmMReg       serdes_ctrl_2;
  FmMReg       serdes_ctrl_3;
  FmMReg       serdes_test_mode; // 0x8003
  FmMReg       serdes_status;
  FmMReg       serdes_ip;
  FmMReg       serdes_im;
  FmMReg       reserved1;
  FmMReg       serdes_bist_err_cnt; // 0x8008
  FmMReg       pcs_cfg_1;
  FmMReg       pcs_cfg_2;
  FmMReg       pcs_cfg_3;
  FmMReg       pcs_cfg_4;
  FmMReg       pcs_cfg_5;
  FmMReg       pcs_ip;    // 0x800e
  FmMReg       pcs_im;
  FmMReg       pacing_pri_wm[8];
  FmMReg       pacing_rate;  // 0x8018
  FmMReg       pacing_status;
  FmMReg       mac_cfg_1;  // 0x801a
  FmMReg       mac_cfg_2;
  FmMReg       mac_cfg_3;
  FmMReg       mac_cfg_4;
  FmMReg       mac_cfg_5;
  FmMReg       mac_cfg_6;
  FmMReg       tx_pri_map_1;   // 0x20
  FmMReg       tx_pri_map_2;
  FmMReg       mac_status;
  FmMReg       mac_ip;
  FmMReg       mac_im;           // 0x24
  FmMReg       stat_epl_error1;
  FmMReg       stat_tx_pause;
  FmMReg       stat_tx_crc;
  FmMReg       stat_epl_error2;
  FmMReg       stat_rx_jabber;  // 0x29
  FmMReg       epl_led_status;
  FmMReg       epl_int_detect;
  FmMReg       stat_tx_bytecount; // 0x2c
  FmMReg       reserved2[0x3d3];
} FmEplRegs;

void FmEplRegs_setup10Gb (plx*, FmEplRegs*, int lb,int rx_flip,int tx_flip,unsigned drive);
void FmEplRegs_setup4Gb  (plx*, FmEplRegs*, int lb,int rx_flip,int tx_flip);
void FmEplRegs_setup2_5Gb(plx*, FmEplRegs*, int lb,int rx_flip,int tx_flip);
void FmEplRegs_setup1Gb  (plx*, FmEplRegs*, int lb,int rx_flip,int tx_flip);
void FmEplRegs_setup100Mb(plx*, FmEplRegs*, int lb,int rx_flip,int tx_flip);
void FmEplRegs_setup10Mb (plx*, FmEplRegs*, int lb,int rx_flip,int tx_flip);

void FmEplRegs_disable   (plx*, FmEplRegs*);

void FmEplRegs_control(plx*, FmEplRegs*, unsigned,unsigned);


#endif
