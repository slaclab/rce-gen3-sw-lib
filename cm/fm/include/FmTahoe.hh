#ifndef FmTahoe_hh
#define FmTahoe_hh


#include "fm/FmBootStatus.hh"
#include "fm/FmPortMibCounters.hh"
#include "fm/FmEplRegs.hh"
#include "fm/FmPortMscCounters.hh"
#include "fm/FmMacTableEntry.hh"
#include "fm/FmVlanTableEntry.hh"
#include "fm/FmFidTableEntry.hh"
#include "fm/FmCpuPortStatus.hh"
#include "fm/FmCReg.hh"
#include "fm/FmMReg.hh"
#include "fm/FmPort.hh"

class FmTahoe {
public:
  FmTahoe();

  void* operator new(unsigned, int id);
  void  operator delete(void*);

  static FmCpuPortStatus cpuPortStatus();

  unsigned id() const;

  void destroy();

  void resetChip();
  void releaseChip();

  int txPacket(const unsigned* p,unsigned len,FmPortMask dmask);
  int rxPacket(unsigned* p);

  static void enableInterrupts(unsigned);
  static void disableInterrupts(unsigned);

  static void enable_tx_fifo_interrupt(unsigned);
  static void enable_rx_fifo_interrupt(unsigned);
  static void disable_tx_fifo_interrupt(unsigned);
  static void disable_rx_fifo_interrupt(unsigned);

  unsigned run_bist(FmPortMask pmask,unsigned itest,unsigned len,
		    unsigned* result);

  //
  //  Device Registers (Map)
  //
  FmBootStatus boot_status;       // 0
  FmMReg reserved1[0xff];
  FmMReg scan_freq_mult;    // 0x100
  FmMReg scan_ctrl;
  FmMReg scan_sel;
  FmMReg scan_data_int;
  FmMReg scan_data_out;     // 0x104
  FmMReg reserved2[0x1fb];

  class SoftReset { public: enum { Core_Reset          =1<<0,
				   Frame_Handler_Reset =1<<1 }; };
  FmMReg soft_reset;        // 0x300
  FmCReg chip_mode;
  FmCReg clk_mult_1;
  FmCReg frame_time_out;
  FmSReg vpd;               // 0x304
  FmMReg reserved3[4];    // fusebox_1..4

  class Interrupt_Detect { public: enum { LCI_Int = 0x2,
					  Trigger_Int = 0x4,
					  Frame_Cntl_Int = 0x10,
					  Mgr_Int = 0x20,
					  EPL_Int = 0x100,
					  Port_Security_Int = 0x200,
					  Port_VLAN_Int2 = 0x400,
					  Port_VLAN_Int1 = 0x800,
					  Parity_Error_Int = 0x1000 }; };
  FmSReg interrupt_detect;  // 0x309

  FmMReg global_epl_int_detect;

  class Mgr_Ip { public: enum { SPI_Error = 0x8,
				Boot_Done = 0x10 }; };
  FmMReg mgr_ip;    // 0x30B
  FmMReg mgr_im;

  class Frame_Ctrl_Ip { public: enum { MAC_Parity_Error=0x1,
				       Entry_Aged=0x2,
				       Entry_Learned=0x4,
				       Bin_Full=0x8,
				       FIFO_Full=0x10,
				       VLAN_Parity_Error=0x20,
				       CM_Priv_Drop=0x40,
				       Frame_Parity_Error=0x80,
				       Frame_TimeOut=0x100,
				       MAC_Lookup_Skipped=0x200,
				       MAC_Learning_Skipped=0x400 }; };
  FmMReg frame_ctrl_ip;
  FmMReg frame_ctrl_im;

  class Sys_Cfg_7 { public: enum { DisableAging = 0x80000000 }; };
  FmMReg sys_cfg_7;         // 0x30f

  FmMReg reserved4[2];    // FM_MA_TABLE_STATUS_3 = 0x310
                            // FM_CONTEPL_CTRLSTAT = 0x311

  FmMReg perr_ip;           // 0x312
  FmMReg perr_im;

  class Perr_Debug { public: enum { s_Transient_Perr = 0, m_Transient_Perr = 0xff };
                             enum { s_Cumulative_Perr = 8, m_Cumulative_Perr = 0xff };
                             enum { s_Fatal_Perr = 16, m_Fatal_Perr = 0xff }; };

  FmMReg perr_debug;

  FmCReg pll_fh_ctrl;
  FmSReg pll_fh_stat;       // 0x316
  FmCReg port_clk_sel;
  FmCReg port_reset;        // 0x318
  FmMReg shadow_fusebox[4]; // 0x319-31c
  FmMReg reserved5[0x1d23];
  FmMReg egress_sched_1;   // 0x2040
  FmMReg egress_sched_2;
  FmMReg reserved6[0xba];
  FmMReg jitter_cfg;       // 0x20fc
  FmMReg reserved7[0xf13];
  FmMReg ma_table_status_3;// 0x3010
  FmMReg reserved8[0xfef];
  FmMReg lci_rx_fifo;      // 0x4000
  FmMReg lci_tx_fifo;

  class Lci_Ip { public: enum { TxRdy = 0x1,
				RxRdy = 0x2,
				RxEOT = 0x4,
				RxErr = 0x8,
				RxErrInt = 0x10,
				RxErrTail = 0x20,
				RxUnderflow = 0x40,
				TxOverrun = 0x80 }; };
  FmMReg lci_ip;          // 0x4002
  FmMReg lci_im;

  class Lci_Status { public: enum { TxRdy = 0x1,
				    RxRdy = 0x2,
				    RxEOT = 0x4 }; };
  FmMReg lci_status;      // 0x4004

  class Lci_Cfg { public: enum { RxEnable = 0x1,
				 TxCompCRC = 0x4,
				 BigEndian = 0x8,
				 HostPadding = 0x10 }; };
  FmMReg lci_cfg;          // 0x4005

  FmMReg reserved9[0x3ffa];
  FmEplRegs  epl_phy [24];  // 0x8000 - 0xdfff
  FmMReg reserved10[0x2000];
  FmMacTableEntry ma_table_entry[0x4000]; // 0x10000
  FmMReg reserved11[0x30000];

  // Some incomplete reference to
  // congestion mgmt 0x30e00

  FmVlanTableEntry vid_table[0x1000];// 0x50000
  FmFidTableEntry  fid_table[0x1000];// 0x52000

  class Port_Cfg_1 { public: enum { DropTaggedFrames = 0x10000,
				    DropUntaggedFrames = 0x20000,
				    FilterIngVlanViolations = 0x40000,
				    VlanIngPortPrecedence = 0x80000,
				    EnableLearning = 0x100000,
				    SecureUnkSourceMAC = 0x200000,
				    SecureSrcMACUnknown = 0x200000,
				    SecureSrcMACUnknownOrMany = 0x300000,
				    TrapSecurityViolations = 0x800000,
				    RemapSecurityViolations = 0x1000000,
				    MultiVlanTagging = 0x2000000 }; };
  FmMReg port_cfg_1[25];   // 0x54000

  FmMReg reserved12[0x3fe7];
  FmMReg ma_table_status_1;// 0x58000
  FmMReg ma_table_status_2;
  FmMReg reserved13[0x10e];
  FmMReg header_mask[4];   // 0x58110
  FmMReg reserved14[0xc];
  FmMReg ma_table_cfg;     // 0x58120

  class Sys_Cfg_2 { public: enum { VlanUcastTunnel = 0x1,
				   VlanMcastTunnel = 0x2,
				   VlanTagEnable   = 0x4,
				   MultiSpanTree   = 0x8 }; };
  FmMReg sys_cfg_2;        // 0x58121

  FmMReg reserved15[0x7ede];
  FmMReg sched_pri_map;    // 0x60000

  class Sys_Cfg_1 { public: enum { TrapOther = 0x1,
				   TrapLACP  = 0x2,
				   TrapBPDU  = 0x4,
				   TrapGARP  = 0x8,
				   TrapIGMP  = 0x10,
				   Trap802_1 = 0x20,
				   TrapBcast = 0x40,
				   RemapIEEE = 0x80,
				   RemapCPU  = 0x100,
				   RemapEthType = 0x200,
				   DropPause = 0x400,
				   UcastFloodCntl = 0x2000,
				   McastFloodCntl = 0x4000,
				   BcastDiscard = 0x8000 }; };
  FmMReg sys_cfg_1;        // 0x60001

  FmMReg sys_cfg_3;
  FmMReg sys_cfg_4;

  class Sys_Cfg_6 { public: enum { TrapEthType = 0x10000 }; };
  FmMReg sys_cfg_6;        // 0x60004

  FmMReg reserved16[0x1b];
  FmMReg trunk_canonical[24];// 0x60020
  FmMReg reserved17[8];
  FmMReg rx_pri_map[25];   // 0x60040
  FmMReg reserved18[7];
  FmMReg port_cfg_2[25];   // 0x60060
  FmMReg reserved19[0xf87];
  FmMReg trunk_hash_mask;    // 0x61000
  FmMReg reserved20[0x101f];
  FmMReg trigger_cfg[16];    // 0x62020
  FmMReg reserved41[0x10];
  FmMReg trigger_pri[16];    // 0x62040
  FmMReg reserved42[0x10];
  FmMReg trigger_rx [16];    // 0x62060
  FmMReg reserved43[0x10];
  FmMReg trigger_tx [16];    // 0x62080
  FmMReg reserved21[0xf70];
  FmMReg trunk_port_map[25]; // 0x63000
  FmMReg reserved22[7];
  FmMReg trunk_group_1[12];  // 0x63020
  FmMReg reserved23[20];
  FmMReg trunk_group_2[12];  // 0x63040
  FmMReg reserved24[20];
  FmMReg trunk_group_3[12];  // 0x63060
  FmMReg reserved25[0xf94];
  FmMReg cm_pri_map_1;     // 0x64000
  FmMReg cm_pri_map_2;
  FmMReg lfsr_cfg;
  FmMReg queue_cfg_4;
  FmMReg queue_cfg_5;
  FmMReg reserved26[3];
  FmMReg stream_status_2;  // 0x64008
  FmMReg reserved27[0x17];
  FmMReg queue_cfg_1[25];  // 0x64020
  FmMReg reserved28[7];
  FmMReg queue_cfg_2[25];  // 0x64040
  FmMReg reserved29[7];
  FmMReg stream_status_1[25]; // 0x64060
  FmMReg reserved30[7];
  FmMReg global_pause_wm[25];// 0x64080
  FmMReg reserved31[7];
  FmMReg rx_pause_wm[25];  // 0x640a0
  FmMReg reserved32[7];
  FmMReg port_vlan_ip_1;   // 0x640c0  // The order of _1 and _2 is reverse that in the datasheet
  FmMReg port_vlan_im_1;               // but agrees with Registers.pm.  Empirically, this order
  FmMReg port_vlan_ip_2;               // is correct (a read clears the corresponding interrupt).
  FmMReg port_vlan_im_2;               //
  FmMReg port_mac_sec_ip;
  FmMReg port_mac_sec_im;
  FmMReg trigger_ip;
  FmMReg trigger_im;        // 0x640c7
  FmMReg reserved33[0xf38];
  FmMReg queue_cfg_3;      // 0x65000
  FmMReg reserved34[0xbf];
  FmMReg saf_matrix[25];   // 0x650c0
  FmMReg reserved35[0xf27];
  FmMibCounter cmGlobalLowDrops; // 0x66000
  FmMibCounter cmGlobalHighDrops;
  FmMibCounter cmGlobalPrivilegeDrops;
  FmMReg reserved36[0x7a];
  FmMibCounter cmTxDrops[25]; // 0x66080
  FmMReg reserved37[0xe];
  FmMibCounter trigCount[17]; // 0x660c0
  FmMReg reserved38[0x1e];
  FmMibCounter vlanUnicast      [32]; // 0x66100
  FmMibCounter vlanXcast        [32];
  FmMibCounter vlanUnicastOctets[32];
  FmMibCounter vlanXcastOctets  [32]; // -0x661ff
  FmMReg reserved39[0x9e00];

  FmPortMibCounters port_mib[25];   // 0x70000-0x731ff
  FmMReg reserved40[0xce00];

  FmPortMscCounters port_msc[25];   // 0x80000-0x883ff

  int _poll(FmMReg& r,unsigned mask,unsigned val,unsigned timeout);

private:

  ~FmTahoe(); // cannot be made on the stack
  friend class no_friend;
};


#endif
