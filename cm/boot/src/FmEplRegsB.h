#ifndef FmEplRegsB_h
#define FmEplRegsB_h



#include "FmMibCounter.h"
#include "FmMReg.h"
#include "plx.h"

typedef struct {
  FmMReg       serdes_ctrl_1;
  FmMReg       serdes_ctrl_2;
  FmMReg       serdes_ctrl_3;
  FmMReg       serdes_test_mode;
  FmMReg       serdes_status;
  FmMReg       serdes_ip;
  FmMReg       serdes_im;
  FmMReg       serdes_bist_err_cnt;
  FmMReg       reserved1;
  FmMReg       pcs_cfg_1;
  FmMReg       pcs_cfg_2;
  FmMReg       pcs_cfg_3;
  FmMReg       pcs_cfg_4;
  FmMReg       pcs_cfg_5;
  FmMReg       pcs_ip;
  FmMReg       pcs_im;
  FmMReg       reserved2[8];
  FmMReg       syncbuf_cfg;
  FmMReg       reserved3;
  FmMReg       mac_cfg_1;
  FmMReg       mac_cfg_2;
  FmMReg       mac_cfg_3;
  FmMReg       reserved4;
  FmMReg       mac_cfg_5;
  FmMReg       mac_cfg_6;
  FmMReg       tx_vpri_map_1;   // 0x20
  FmMReg       tx_vpri_map_2;
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
  FmMReg       reserved_2d[0x3d-0x2d];
  FmMReg       jitter_timer; // 0x3d
  FmMReg       parse_cfg   ; // 0x3e
  FmSReg       reserved_3e[0x80-0x3f];
  FmMReg       vlantag_table[0x80];
  FmSReg       reserved_100[0x400-0x100];
} FmEplRegsB;

void FmEplRegsB_setup10Gb (plx*, FmEplRegsB*, int lb,int rx_flip,int tx_flip,unsigned drive);
void FmEplRegsB_setup4Gb  (plx*, FmEplRegsB*, int lb,int rx_flip,int tx_flip);
void FmEplRegsB_setup2_5Gb(plx*, FmEplRegsB*, int lb,int rx_flip,int tx_flip);
void FmEplRegsB_setup1Gb  (plx*, FmEplRegsB*, int lb,int rx_flip,int tx_flip);
void FmEplRegsB_setup100Mb(plx*, FmEplRegsB*, int lb,int rx_flip,int tx_flip);
void FmEplRegsB_setup10Mb (plx*, FmEplRegsB*, int lb,int rx_flip,int tx_flip);

void FmEplRegsB_disable   (plx*, FmEplRegsB*);

void FmEplRegsB_control(plx*, FmEplRegsB*, unsigned,unsigned);


#endif
