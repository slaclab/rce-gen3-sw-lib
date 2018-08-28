#ifndef FmBali_h
#define FmBali_h

#include <inttypes.h>

#include "FmPortMibCounters.h"
#include "FmEplRegsB.h"
#include "FmMacTableEntry.h"
#include "FmIngressVlanTableEntry.h"
#include "FmEgressVlanTableEntry.h"
#include "FmIngressFidTableEntry.h"
#include "FmEgressFidTableEntry.h"
#include "FmStatsCfg.h"
#include "FmMacTcnFifoEntry.h"

#include "FmCReg.h"
#include "FmMReg.h"
#include "FmPort.h"

//
//  Device Register Map
//
typedef struct {
  //
  //  HSM Registers
  //
  FmSReg reserved;         // 0

  FmMReg lci_cfg;          // 1
  FmMReg lci_rx_fifo;     
  FmMReg lci_tx_fifo;

  FmMReg lci_ip;          // 4
  FmMReg lci_im;

  FmMReg lci_status;
  FmSReg reserved_7[0x200-0x7];

  FmSReg last_fatal;         // 0x200

  FmSReg interrupt_detect;
  FmSReg fatal_count;

  FmMReg soft_reset;

  FmMReg watchdog_cfg;
  FmSReg pll_fh_stat;
  FmMReg pll_fh_ctrl; // 0x206
  FmSReg reserved207[0x304-0x207];

  FmSReg vpd;               // 0x304
  FmSReg reserved305[0x40000-0x305];

  //
  //  LSM Registers
  //
  FmMReg lsm_int_detect;    // 0x40000
  FmMReg global_epl_int_detect;
  FmMReg perr_ip;
  FmMReg perr_im;
  FmMReg sw_ip;
  FmMReg sw_im;
  FmSReg reserved_40006[0x40101-0x40006];
  FmCReg frame_time_out;    // 0x40101
  FmCReg boot_ctrl;
  FmSReg boot_status;
  FmCReg clk_mult_1;
  FmSReg reserved_40105[0x40108-0x40105];
  FmSReg vpd_info_1[2];     // 0x40108
  FmSReg vpd_info_2[2];
  FmSReg reserved_4010C[0x40134-0x4010c];
  FmMReg gpio_cfg;          // 0x40134
  FmMReg gpio_data;
  FmMReg gpio_ip;
  FmMReg gpio_im;
  FmMReg i2c_cfg;
  FmMReg i2c_data[2];
  FmMReg i2c_ctrl;
  FmMReg mdio_cfg;
  FmMReg mdio_data;
  FmMReg mdio_ctrl;
  FmMReg led_cfg;
  FmSReg reserved_40140[0x40160-0x40140];
  FmMReg epl_port_ctrl[25]; // 0x40160
  FmSReg reserved_40179[0x41200-0x40179];
  FmMReg crm_cfg_counter[256]; // 0x41200
  FmMReg crm_cfg_window[256];
  FmMReg crm_cfg_limit[256];
  FmMReg crm_last_count[256];
  FmMReg crm_exceed_count[256];
  FmMReg crm_cfg;
  FmMReg crm_int_detect;
  FmSReg reserved_41702[0x41710-0x41702];
  FmMReg crm_ip[8];
  FmSReg reserved_41718[0x41720-0x41718];
  FmMReg crm_im[8];
  FmSReg reserved_41728[0x50000-0x41728];

  //
  //  Ethernet Port Logic Registers
  //
  FmEplRegsB epl_phy[25]; // 0x50000
  FmSReg reserved_56400[0x60000-0x56400];

  //
  //  Scheduler Registers
  //
  FmSReg reserved_60000[0x60040-0x60000];
  struct {
    FmMReg cfg;
    FmSReg reserved;
  } sched_group[25];
  FmSReg reserved_60072[0x80000-0x60072];

  //
  //  MSB Registers
  //
  FmSReg reserved_80000[0x90000-0x80000];

  //
  //  Stats Registers
  //
#define ADD_STAT(group,member0,member1)         \
  struct {                                      \
    FmMibCounter member0;                       \
    FmMibCounter member1;                       \
  } Stat##group[25];                            \
    FmSReg reserved_Stat##group[12]

  ADD_STAT(0, RxUcstPktsNonIp, RxBcstPktsNonIp);
  ADD_STAT(1, RxMcstPktsNonIp, RxUcstPktsIPv4);
  ADD_STAT(2, RxBcstPktsIPv4,  RxMcstPktsIPv4);
  ADD_STAT(3, RxUcstPktsIPv6,  RxBcstPktsIPv6);
  ADD_STAT(4, RxMcstPktsIPv6,  RxPausePkts);
  ADD_STAT(5, RxCBPausePkts,   RxSymbolErrors);
  ADD_STAT(6, RxFcsErrors,     RxFrameSizeErrors);
  ADD_STAT(7, RxP0, RxP1);
  ADD_STAT(8, RxP2, RxP3);
  ADD_STAT(9, RxP4, RxP5);
  ADD_STAT(A, RxP6, RxP7);
  ADD_STAT(B, TxUcstPkts, TxBcstPkts);
  ADD_STAT(C, TxMcstPkts, TxTimeoutDrop);
  ADD_STAT(D, TxErrorDrop, TxLoopbackDrop);
  FmSReg reserved_90620[0x91000-0x90620];
  
  ADD_STAT(E, RxMinto63,     Rx64Pkts);
  ADD_STAT(F, Rx65to127,     Rx128to255);
  ADD_STAT(G, Rx256to511,    Rx512to1023);
  ADD_STAT(H, Rx1024to1522,  Rx1523to2047);
  ADD_STAT(I, Rx2048to4095,  Rx4096to8191);
  ADD_STAT(J, Rx8192to10239, Rx10240toMax);
  ADD_STAT(K, RxFragments,   RxUndersized);
  ADD_STAT(L, RxOversized,   ReservedL);
  ADD_STAT(M, RxOctetsNonIP, RxOctetsIPv4);
  ADD_STAT(N, RxOctetsIPv6,  RxOctetsError);
  ADD_STAT(O, TxMinto63,     Tx64Pkts);
  ADD_STAT(P, Tx65to127,     Tx128to255);
  ADD_STAT(Q, Tx256to511,    Tx512to1023);
  ADD_STAT(R, Tx1024to1522,  Tx1523to2047);
  ADD_STAT(S, Tx2048to4095,  Tx4096to8191);
  ADD_STAT(T, Tx8192to10239, Tx10240toMax);
  ADD_STAT(U, TxErrors,      ReservedU);
  FmSReg reserved_91770[0x92000-0x91770];

  ADD_STAT(V, FIDForwarded, FloodForwarded);
  ADD_STAT(W, SpeciallyHandled, ParseErrDrops);
  ADD_STAT(X, ParityError, Trapped);
  ADD_STAT(Y, PauseDrops, STPDrops);
  ADD_STAT(Z, SecurityViolations, VlanTagDrops);
  ADD_STAT(a, VlanIngressDrops, VlanEgressDrops);
  ADD_STAT(b, GlortMissDrops, FFUDrops);
  ADD_STAT(c, TriggerDrops, PolicerDrops);
  ADD_STAT(d, TTLDrops, CMPrivDrops);
  ADD_STAT(e, SMP0Drops, SMP1Drops);
  ADD_STAT(f, RxHog0Drops, RxHog1Drops);
  ADD_STAT(g, TxHog0Drops, TxHog1Drops);
  ADD_STAT(h, RateLimit0Drops, RateLimit1Drops);
  ADD_STAT(i, BadSMPDrops, TriggerRedirects);
  ADD_STAT(j, Others, Reservedj);
  FmSReg reserved_92690[0x92700-0x92690];

  ADD_STAT(k, RxOctetsP0, RxOctetsP1);
  ADD_STAT(l, RxOctetsP2, RxOctetsP3);
  ADD_STAT(m, RxOctetsP4, RxOctetsP5);
  ADD_STAT(n, RxOctetsP6, RxOctetsP7);
  ADD_STAT(o, TxOctets, TxErrorOctets);
  FmSReg reserved_92930[0xc0000-0x92930];
#undef ADD_STAT

  //
  //  MTABLE Registers
  //
  FmSReg reserved_C0000[0xC8000-0xC0000];
  FmMReg ip_multicast_table[16384];
  FmSReg reserved_CC000[0x100000-0xCC000];

  //
  //  Frame Handler Registers
  //
  FmMReg sys_cfg_1;
  FmSReg reserved_100001;
  FmMReg sys_cfg_3;
  FmMReg sys_cfg_4;
  FmSReg reserved_100004[2];
  FmMReg sys_cfg_7;
  FmMReg sys_cfg_8;
  FmSReg reserved_100008[0x100012-0x100008];
  FmMReg port_vlan_ip_1;
  FmMReg port_vlan_im_1;
  FmMReg port_vlan_ip_2;
  FmMReg port_vlan_im_2;
  FmMReg port_mac_sec_ip;
  FmMReg port_mac_sec_im;
  FmMReg fh_int_detect;
  FmMReg sys_cfg_router;
  FmSReg reserved_10001A[0x100020-0x10001A];
  FmMReg l34_hash_cfg;
  FmMReg l34_flow_hash_cfg_1;
  FmMReg l34_flow_hash_cfg_2;
  FmMReg l234_hash_cfg;
  //  FmSReg reserved_100024[0x10002E-0x100024];
  FmSReg reserved_100024[10];
  FmMReg tx_mirror_fh;
  FmMReg cpu_trap_mask_fh;
  FmMReg cpu_log_mask_fh;
  FmMReg rx_mirror_cfg;
  FmMReg parity_ip;
  FmMReg parity_im;
  FmSReg reserved_100034;
  FmMReg trap_glort;
  FmSReg reserved_100036[0x100040-0x100036];
  FmMReg saf_matrix[25];
  FmSReg reserved_100059[0x100060-0x100059];
  FmMReg fh_loopback_suppress[25];
  FmSReg reserved_100079[0x100080-0x100079];
  FmMReg internal_port_mask;
  FmSReg reserved_100081;
  FmMReg mgmt_clk_counter;
  FmMReg mgmt_ffu_clk_counter;
  FmSReg reserved_100084[0x101000-0x100084];

  FmMReg port_cfg_1[25];
  FmSReg reserved_101019[0x101020-0x101019];
  FmMReg port_cfg_2[25];
  FmSReg reserved_101039[0x101040-0x101039];
  FmMReg port_cfg_3[25];
  FmSReg reserved_101059[0x101060-0x101059];
  FmMReg port_cfg_isl[25];
  FmSReg reserved_101079[0x101080-0x101079];
  FmMReg rx_vpri_map[25];
  FmSReg reserved_101099[0x1010C0-0x101099];
  FmMReg dscp_pri_map[64];
  FmMReg vpri_pri_map[16];
  FmSReg reserved_101110[0x103000-0x101110];

  FmSReg reserved_103000[0x104000-0x103000];  // congestion management

  FmMReg lag_cfg[25];
  FmSReg reserved_104019[0x104020-0x104019];
  FmMReg canonical_glort_cam[16];
  FmSReg reserved_104030[0x106000-0x104030];

  FmSReg reserved_106000[0x106100-0x106000];
  FmMReg trigger_config_cfg[64];
  FmMReg trigger_config_param[64];
  FmMReg trigger_config_ffu[64];
  FmMReg trigger_config_type[64];
  FmMReg trigger_config_glort[64];
  FmMReg trigger_config_rx[64];
  FmMReg trigger_config_tx[64];
  FmMReg trigger_config_amask_1[64];
  FmMReg trigger_config_amask_2[64];
  FmMReg trigger_action_cfg_1[64];
  FmMReg trigger_action_cfg_2[64];
  FmMReg trigger_action_glort[64];
  FmMReg trigger_action_dmask[64];
  FmMReg trigger_action_mirror[64];
  FmMReg trigger_action_drop[64];
  FmMReg trigger_rate_lim_cfg_1[16];
  FmMReg trigger_rate_lim_cfg_2[16];
  FmMReg trigger_ip[2];
  FmSReg reserved_1064E2[0x1064E4-0x1064E2];
  FmMReg trigger_im[2];
  FmSReg reserved_1064E6[0x106580-0x1064E6];
  FmMReg trigger_rate_lim_usage[64];
  FmSReg reserved_1065C0[0x108000-0x1065C0];

  struct {
    FmMReg a;
    FmMReg b;
  } glort_dest_table[4096];

  FmMReg glort_cam[256];

  FmSReg reserved_10A100[0x10A200-0x10A100];

  struct {
    FmMReg a;
    FmMReg b;
  } glort_ram[256];
  
  FmSReg reserved_10A400[0x10C000-0x10A400];

  FmSReg reserved_10C000[0x120000-0x10C000];

  FmSReg reserved_120000[0x140000-0x120000];

  FmSReg reserved_140000[0x17B800-0x140000];
  FmMReg ffu_init_slice;

  FmSReg reserved_17B801[0x180000-0x17B801];

  FmMacTableEntry         ma_table_entry   [0x4000]; // 0x180000
  FmIngressVlanTableEntry ingress_vid_table[0x1000]; // 0x190000
  FmEgressVlanTableEntry  egress_vid_table [0x1000]; // 0x194000
  FmIngressFidTableEntry  ingress_fid_table[0x1000]; // 0x196000
  FmEgressFidTableEntry   egress_fid_table [0x1000]; // 0x198000
  FmSReg                  reserved_199000  [0x19E000-0x199000];

  FmMacTcnFifoEntry       mac_tcn_fifo     [0x200];  // 0x19E000
  FmMReg                  mac_tcn_ptr;               // 0x19E800
  FmMReg                  mac_tcn_ip;
  FmMReg                  mac_tcn_im;
  FmMReg                  mac_table_status_3;
  FmMReg                  mac_table_cfg_1;
  FmMReg                  mac_table_cfg_2;
  FmMReg                  mac_table_cfg_3;
  FmMReg                  mac_tcn_cfg_1;
  FmMReg                  mac_tcn_cfg_2;
  FmMReg                  mac_purge;
  FmMReg                  mtu_table[8];
  FmSReg                  reserved_19E818[0x1A0100-0x19E812];

  //
  //  Stats VLAN
  //
  FmMibCounter stat_VlanUcstPkts  [64];  // 0x1A0100
  FmMibCounter stat_VlanXcstPkts  [64];  // 0x1A0180
  FmMibCounter stat_VlanUcstOctets[64];  // 0x1A0200
  FmMibCounter stat_VlanXcstOctets[64];  // 0x1A0280
  FmMibCounter stat_Trigger       [64];  // 0x1A0300
  FmSReg       reserved_1A0380[0x1A0400-0x1A0380];
  FmMibCounter stat_egressAclPkts [25];
  FmSReg       reserved_1A0432[0x1A0480-0x1A0432];
  FmMibCounter stat_egressAclOctets[25];
  FmSReg       reserved_1A04B2[0x1A0500-0x1A04B2];
  FmStatsCfg   stats_cfg[25];
  FmSReg       reserved_1A0532[0x1A0540-0x1A0532];

#define ADD_STAT(group)                         \
  struct {                                      \
    FmSReg value;                               \
    FmSReg reserved;                            \
  } group[25];                                  \
    FmSReg reserved_##group[14]

  ADD_STAT(stats_drop_count_rx);
  ADD_STAT(stats_drop_count_tx);

#undef ADD_STAT
} FmBali;

#endif


