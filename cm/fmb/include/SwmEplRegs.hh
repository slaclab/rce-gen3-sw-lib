#ifndef Bali_SwmEplRegsEplRegs_hh
#define Bali_SwmEplRegsEplRegs_hh


namespace Bali {
  class FmEplRegs;

  class SwmEplRegs {
  public:
    SwmEplRegs();
    //  SwmEplRegs( const FmEplRegs& );
    ~SwmEplRegs();

    SwmEplRegs& operator=(const SwmEplRegs&);

#if 0
    unsigned       serdes_ctrl_1;
    unsigned       serdes_ctrl_2;
    unsigned       serdes_ctrl_3;
    unsigned       serdes_test_mode;
    unsigned       serdes_status;
    unsigned       serdes_ip;
    unsigned       serdes_im;
    unsigned       serdes_bist_err_cnt;
    unsigned       reserved1;
    unsigned       pcs_cfg_1;
    unsigned       pcs_cfg_2;
    unsigned       pcs_cfg_3;
    unsigned       pcs_cfg_4;
    unsigned       pcs_cfg_5;
    unsigned       pcs_ip;
    unsigned       pcs_im;
    unsigned       reserved2[8];
    unsigned       syncbuf_cfg;
    unsigned       reserved3;
    unsigned       mac_cfg_1;  // 0x801a
    unsigned       mac_cfg_2;
    unsigned       mac_cfg_3;
    unsigned       reserved4;
    unsigned       mac_cfg_5;
    unsigned       mac_cfg_6;
    unsigned       tx_vpri_map_1;   // 0x20
    unsigned       tx_vpri_map_2;
    unsigned       mac_status;
    unsigned       mac_ip;
    unsigned       mac_im;           // 0x24
    unsigned       stat_epl_error1;
    unsigned       stat_tx_pause;
    unsigned       stat_tx_crc;
    unsigned       stat_epl_error2;
    unsigned       stat_rx_jabber;  // 0x29
    unsigned       epl_led_status;
    unsigned       epl_int_detect;
    unsigned       stat_tx_bytecount; // 0x2c
    unsigned       reserved5[6];
    unsigned       src_mac_lo;   // 0x33
    unsigned       src_mac_hi;
    unsigned       src_mac_virtual_lo;
    unsigned       src_mac_virtual_hi;
    unsigned       reserved6[6];
    unsigned       jitter_timer;   // 0x3d
    unsigned       parse_cfg;
    unsigned       mac_vlan_etype_2;
    unsigned       parse_rlt_1;
    unsigned       parse_rlt_2;
    unsigned       tx_trunc;
    unsigned       cpid[16];
    unsigned       reserved7[6];
    unsigned       mac_vlan_type_1;
    unsigned       di_cfg;
    unsigned       tcp_wd_mask_lo;
    unsigned       tcp_wd_mask_hi;
    unsigned       udp_wd_mask_lo;
    unsigned       udp_wd_mask_hi;
    unsigned       l4prot1_wd_mask_lo;
    unsigned       l4prot1_wd_mask_hi;
    unsigned       l4prot2_wd_mask_lo;
    unsigned       l4prot2_wd_mask_hi;
    unsigned       an_tx_msg[4];
    unsigned       an_rx_msg[4];
    unsigned       an_ctrl;
    unsigned       an_status;
    unsigned       an_timeout;
    unsigned       an_tx_timer;
    unsigned       reserved8[4];
    unsigned       vlantag_table[128];
    unsigned       reserved9[0x300];
#else
    unsigned       serdes_ctrl_1;
    unsigned       serdes_ctrl_2;
    unsigned       serdes_ctrl_3;
    unsigned       serdes_test_mode;
    unsigned       serdes_status;
    unsigned       serdes_ip;
    unsigned       serdes_im;
    unsigned       serdes_bist_err_cnt;
//  unsigned       reserved1;
    unsigned       pcs_cfg_1;
    unsigned       pcs_cfg_2;
    unsigned       pcs_cfg_3;
    unsigned       pcs_cfg_4;
    unsigned       pcs_cfg_5;
    unsigned       pcs_ip;
    unsigned       pcs_im;
//  unsigned       reserved2[8];
    unsigned       syncbuf_cfg;
//  unsigned       reserved3;
    unsigned       mac_cfg_1;
    unsigned       mac_cfg_2;
    unsigned       mac_cfg_3;
//  unsigned       reserved4;
    unsigned       mac_cfg_5;
    unsigned       mac_cfg_6;
    unsigned       tx_vpri_map_1;
    unsigned       tx_vpri_map_2;
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
//  unsigned       reserved5[0x3d3];
#endif /* 0 */
  };
};

#endif
