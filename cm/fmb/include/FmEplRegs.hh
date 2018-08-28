#ifndef Bali_FmEplRegs_hh
#define Bali_FmEplRegs_hh


#include "fm/FmMibCounter.hh"
#include "fm/FmMReg.hh"

namespace Bali {
  class FmBali;
  class SwmEplRegs;

  class FmEplRegs {
  public:
    FmEplRegs() {}

    FmEplRegs& operator=(const SwmEplRegs&);
    operator SwmEplRegs() const;

    unsigned ds() const;

    void setup10Gb (int lb=0,int rx_flip=0,int tx_flip=0,unsigned drive=0);
    void setup4Gb  (int lb=0,int rx_flip=0,int tx_flip=0);
    void setup2_5Gb(int lb=0,int rx_flip=0,int tx_flip=0);
    void setup1Gb  (int lb=0,int rx_flip=0,int tx_flip=0);
    void setup100Mb(int lb=0,int rx_flip=0,int tx_flip=0);
    void setup10Mb (int lb=0,int rx_flip=0,int tx_flip=0);

    void disable   ();

  public:
    enum Speed { Disabled, X10Mb, X100Mb, X1Gb, X2_5Gb, X4Lanes };
    Speed speed() const;
    bool rx_flip() const;
    bool tx_flip() const;

  private:
    void _control(unsigned,unsigned=0);

  public:
    FmMReg       serdes_ctrl_1;
    FmMReg       serdes_ctrl_2;
    FmMReg       serdes_ctrl_3;
    FmMReg       serdes_test_mode;
    FmMReg       serdes_status;
    FmMReg       serdes_ip;
    FmMReg       serdes_im;
    FmMReg       serdes_bist_err_cnt;
  private:
    FmMReg       reserved1;
  public:
    FmMReg       pcs_cfg_1;
    FmMReg       pcs_cfg_2;
    FmMReg       pcs_cfg_3;
    FmMReg       pcs_cfg_4;
    FmMReg       pcs_cfg_5;
    FmMReg       pcs_ip;
    FmMReg       pcs_im;
  private:
    FmMReg       reserved2[8];
  public:
    FmMReg       syncbuf_cfg;
  private:
    FmMReg       reserved3;
  public:
    FmMReg       mac_cfg_1;
    FmMReg       mac_cfg_2;
    FmMReg       mac_cfg_3;
  private:
    FmMReg       reserved4;
  public:
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
  private:
    FmMReg       reserved_2d[0x3d-0x2d];
  public:
    FmMReg       jitter_timer; // 0x3d
    FmMReg       parse_cfg   ; // 0x3e
  private:
    FmSReg       reserved_3e[0x80-0x3f];
  public:
    FmMReg       vlantag_table[0x80];
  private:
    FmSReg       reserved_100[0x400-0x100];
  private:
    ~FmEplRegs() {}
    friend class FmBali;
  };
};

#endif
