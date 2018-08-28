
#include "fm/FmTahoeMgr.hh"

#include "cm_svc/Print.hh"

#include "fm/FmMacTable.hh"
#include "fm/FmTahoe.hh"
#include "fm/FmMgrHandler.hh"
#include "fm/FmEplHandler.hh"
#include "fm/FmHandler.hh"
#include "fm/FmConfig.hh"

#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>

//#define DBUG

static const unsigned _testPort = 5;

using namespace cm::service;

class FmDefaultHandler : public FmHandler {
 public:
   FmDefaultHandler(const char* title) : _title(title) {}
   void call(unsigned p) {
#ifdef DBUG
     printf("FmDefaultHandler[%s]::call %x\n",_title,p);
#endif
   }
 private:
   const char* _title;
 };

FmTahoeMgr::FmTahoeMgr( FmTahoe& d, const FmConfig& cfg ) :
  _dev(d),
  _macTable(0),
  _eplHandler(0),
  _config(cfg),
  _unusedTriggers(0)
{
  // Compare cfg against current register values?
//  configure(cfg);
}

//  Initialize PCIE link
//    Memory map access to interface
//    Reset bridge (and switch)
//    Setup bridge (and switch)
void FmTahoeMgr::reset(const FmConfig& cfg)
{
  timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 20000000; // (20 msec should be sufficient)

  volatile unsigned v;

   FmReg::init();

   //  Hardware initialization goes here
   _dev.resetChip();
   nanosleep(&ts, 0);

   _dev.releaseChip();
   nanosleep(&ts, 0);

   //  Poll boot status until zero or timeout
   { unsigned k=1024;
     do {
       v = _dev.boot_status;
     } while( v && --k );
     if (!k) {
       std::ostringstream o;
       o << "FmTahoeMgr boot_status poll1 timeout 0x"
         << std::hex << v;
       throw o.str();
     }
   }

   //  Begin boot state machine
   //  ( Enable DFT/JTAG with 0xd05 )
   //  Errata item #10 "Contention between 1Gb Mode and LED Interface
   //    Port Status
   //   _dev.chip_mode = 0xd04;

   printv("chip mode = %x", unsigned(_dev.chip_mode));
   _dev.chip_mode = 0xd00;
   //   printv("chip mode = %x", unsigned(_dev.chip_mode));

   printv("FmTahoeMgr(%d) waiting for boot complete",_dev.id());

   //  Poll until boot is complete
   { unsigned k=20480;
     do {
       v = _dev.boot_status;
     } while( v && --k );
     if (!k) {
       std::ostringstream o;
       o << "FmTahoeMgr boot status didn't clear in 20480 reads 0x"
         << std::hex << v;
       throw o.str();
     }
     else
       printv("FmTahoeMgr boot status cleared in %d reads",20480-k);
   }

   //  Read vital product data register
   {
     v = _dev.vpd;
     if ( (v & 0x0fffffff) != 0xae1842b ) {
       std::ostringstream o;
       o << "Read incorrect product code 0x"
         << std::hex << v << " (0x0ae1842b)";
       throw o.str();
     }
   }

   //  Ref design writes and reads from shadow fusebox here as a test

   //  Program frame handler clock for 300MHz
   //  (some contradiction in the description of bit 0 for this register)
   //  pllout<<15 | m << 4 | n << 11 | p << 2
   //  p=0; m=1b; n=3  (297 MHz)

   while(1) {
     printv("pll fh ctrl = %x", unsigned(_dev.pll_fh_ctrl));
     _dev.pll_fh_ctrl = 0x19b0;
     printv("pll fh ctrl = %x", unsigned(_dev.pll_fh_ctrl));

     printv("waiting");
     //   ts.tv_sec = 3;
     ts.tv_sec = 1;
     ts.tv_nsec = 0;
     nanosleep(&ts, 0);
     printv("polling");

     //  Poll until frame handler pll locks
     { unsigned k = 10240;
       do {
         v = _dev.pll_fh_stat;
       } while ( v==0 && --k );
       if (!k) {
         printv("FmTahoeMgr timeout waiting for FH_PLL to lock");
       }
       else {
         printv("FmTahoeMgr fh pll locked");
         break;
       }
     }
     printv("pll fh ctrl = %x", unsigned(_dev.pll_fh_ctrl));
   }

   //  Release pll_fh
   //   printv("FmTahoeMgr releasing FH reset");
   _dev.soft_reset = 0x0;

   if ( (v = _dev.sys_cfg_1) != 0x4ff) {
     printv("FmTahoeMgr fail: fm_sys_cfg_1 = 0x%x != 0x4ff",v);
     return;
   }

   if ( (v = _dev.port_reset) != 0x1fffffe ) {
     printv("FmTahoeMgr fail: port reset = 0x%x != 0x1fffffe",v);
     return;
   }

   FmEplRegs& p1 = _dev.epl_phy[_testPort];
   if ( (v = p1.pcs_cfg_1) != 0 ) {
     printv("FmTahoeMgr fail: ports in reset but EPL reg reads= 0x%x",v);
     return;
   }

   p1.pcs_cfg_1 = 0xffffffff;
   if ( (v = p1.pcs_cfg_1) != 0 ) {
     printv("FmTahoeMgr fail: ports in reset but EPL reg allowed write");
     return;
   }

   configure(cfg);
}

FmTahoeMgr::~FmTahoeMgr()
{
  for(unsigned k=0; k<nInterrupts; k++)
    delete _handler[k];
}

int FmTahoeMgr::read     (FmConfig& cfg)
{
  memset(&cfg.portCfg,0,sizeof(cfg.portCfg));
  unsigned v = _dev.port_clk_sel;
  for(unsigned k=0; k<FmPort::MAX_PORT; k++) {
    unsigned pmask = 1<<(k+1);
    switch(_dev.epl_phy[k].speed()) {
    case FmEplRegs::X4Lanes: 
      if (v&pmask) cfg.portCfg.ports4Gb  |= pmask;
      else         cfg.portCfg.ports10Gb |= pmask;
      break;
    case FmEplRegs::X2_5Gb:
      cfg.portCfg.ports2_5Gb |= pmask;
      break;
    case FmEplRegs::X1Gb:
      cfg.portCfg.ports1Gb |= pmask;
      break;
    case FmEplRegs::X100Mb:
      cfg.portCfg.ports100Mb |= pmask;
      break;
    case FmEplRegs::X10Mb:
      cfg.portCfg.ports10Mb |= pmask;
      break;
    default:
      break;
    }
    if (_dev.epl_phy[k].rx_flip()) cfg.portCfg.portsRxFlip |= pmask;
    if (_dev.epl_phy[k].tx_flip()) cfg.portCfg.portsTxFlip |= pmask;
  }

  getMacAddress(&cfg.hostCfg.mac_addr[0]);

  for(unsigned k=0; k<FmSwitchConfig::Entries; k++)
    cfg.swCfg.fid_table[k] = _dev.fid_table[k];

  for(unsigned k=0; k<FmSwitchConfig::Entries; k++)
    cfg.swCfg.vid_table[k] = _dev.vid_table[k];

  cfg.swCfg.mac_table.clear();
  if (_macTable) {
    for(unsigned k=0; k<SwmMacTable::Entries; k++) {
      SwmMacTableEntry entry = _macTable->direct_entry(k);
      if (entry.valid() && entry.locked())
        cfg.swCfg.mac_table.entry[k] = entry;
    }
  }

  cfg.swCfg.port_vlan.untagged     = FmPortMask(0);
  cfg.swCfg.port_vlan.tagged       = FmPortMask(0);
  cfg.swCfg.port_vlan.strict       = FmPortMask(0);
  for(unsigned k=0; k<FmPort::MAX_PORT; k++) {
    FmPortMask m(1<<(k+1));
    unsigned v = _dev.port_cfg_1[k+1];
    cfg.swCfg.port_vlan.defaultVlan[k] = v&0xfff;
    if (v & FmTahoe::Port_Cfg_1::DropUntaggedFrames)
      cfg.swCfg.port_vlan.tagged   |= m;
    if (v & FmTahoe::Port_Cfg_1::DropTaggedFrames)
      cfg.swCfg.port_vlan.untagged |= m;
    if (v & FmTahoe::Port_Cfg_1::FilterIngVlanViolations)
      cfg.swCfg.port_vlan.strict   |= m;
  }

  for(unsigned k=0; k<FmPort::MAX_PORT; k++)
    cfg.swCfg.portfwd_table.forward[k] = FmPortMask(_dev.port_cfg_2[k+1]);

  for(unsigned k=0; k<FmConfig::NumberOfTriggers; k++) {
    FmTriggerConfig& t = cfg.trigCfg[k];
    unsigned v = _dev.trigger_cfg[k];
    if (v == 0x3) 
      t.enable = 0;
    else {
      t.enable = 1;
      t.mac_entry_trigger = (v>>28)&0xf;
      t.new_priority      = (v>>24)&0xf;
      t.mirror_port       = FmPort((v>>19)&0x1f);
      t.action            = FmTriggerConfig::Action((v>>16)&0x7);
      t.priorityMask = (v & (1<<10)) ? _dev.trigger_pri[k] : 0;
      t.sourcePortMask      = FmPortMask((v & (1<<4)) ? _dev.trigger_rx[k] : 0);
      t.destinationPortMask = FmPortMask((v & (1<<5)) ? _dev.trigger_tx[k] : 0);
      t.vlanMatch           = (v & (1<<6)) ? 1 : 0;
      if      (v & (1<<7)) t.xcast = FmTriggerConfig::Unicast;
      else if (v & (1<<8)) t.xcast = FmTriggerConfig::Broadcast;
      else if (v & (1<<9)) t.xcast = FmTriggerConfig::Multicast;
      if      (v & (1<<0 )) t.sourceMacMatch = FmTriggerConfig::Hit;
      else if (v & (1<<1 )) t.sourceMacMatch = FmTriggerConfig::Miss;
      else if (v & (1<<11)) t.sourceMacMatch = FmTriggerConfig::OneHit;
      if      (v & (1<<2 )) t.destinationMacMatch = FmTriggerConfig::Hit;
      else if (v & (1<<3 )) t.destinationMacMatch = FmTriggerConfig::Miss;
      else if (v & (1<<11)) t.destinationMacMatch = FmTriggerConfig::OneHit;
    }
  }

  //  cfg.protocolCfg = ;   // Difficult to extract any of this
  return 0;
}

int FmTahoeMgr::configure(const FmConfig& cfg)
{
#ifdef DBUG
   printv("FmTahoeMgr(%d) clearing mactable\n",_dev.id());
#endif
   if (!_macTable)
     _macTable = new FmMacTable(_dev);
   for(unsigned k=0; k<SwmMacTable::Entries; k++) {
     if (_config.swCfg.mac_table.entry[k].valid())
       _macTable->deleteEntry(_config.swCfg.mac_table.entry[k]);
   }

  //  Assign the new configuration

   //  config.portCfg = cfg.portCfg;
   _config = cfg;

   const unsigned ports312MHz = _config.portCfg.ports10Gb | _config.portCfg.ports2_5Gb;
   const unsigned _defaultPorts =
     0x1 |
     _config.portCfg.ports10Gb  |
     _config.portCfg.ports4Gb   |
     _config.portCfg.ports2_5Gb |
     _config.portCfg.ports1Gb   |
     _config.portCfg.ports100Mb |
     _config.portCfg.ports10Mb  ;

   unsigned v = 0x1fffffe & ~ports312MHz;
#ifdef DBUG
   printv("Setting clock selection 0x%x\n",v);
#endif
   _dev.port_clk_sel = v;

#ifdef DBUG
   printv("Reset MACs\n");
#endif
   //  Reset MACs
   for(unsigned k=1; k<=24; k++) {
     //     if ( _defaultPorts & (1<<k) ) {
       FmMReg& r = _dev.epl_phy[k-1].mac_cfg_2;
       unsigned v = r;
       v |= 3;
       r = v;
       v &= ~3;
       r = v;
       //     }
   }

#ifdef DBUG
   printv("Bringing all EPLs out of reset\n");
#endif
   _dev.port_reset = 0x1fffffe & ~_defaultPorts;
   //   _dev.port_reset = 0;

   for(unsigned k=1; k<=24; k++) {
     _dev.epl_phy[k-1].serdes_ctrl_1 = 0x88884444;
     const unsigned pmask = (1<<k);
     int lb     = _config.portCfg.portsLoopback & (1<<k);
     int rxFlip = _config.portCfg.portsRxFlip   & (1<<k);
     int txFlip = _config.portCfg.portsTxFlip   & (1<<k);
     if (_config.portCfg.ports10Gb & pmask)
       _dev.epl_phy[k-1].setup10Gb(lb, rxFlip, txFlip);
     else if (_config.portCfg.ports4Gb & pmask)
       _dev.epl_phy[k-1].setup4Gb(lb, rxFlip, txFlip);
     else if (_config.portCfg.ports2_5Gb & pmask)
       _dev.epl_phy[k-1].setup2_5Gb(lb, rxFlip, txFlip);
     else if (_config.portCfg.ports1Gb & pmask)
       _dev.epl_phy[k-1].setup1Gb(lb, rxFlip, txFlip);
     else if (_config.portCfg.ports100Mb & pmask)
       _dev.epl_phy[k-1].setup100Mb(lb, rxFlip, txFlip);
     else if (_config.portCfg.ports10Mb & pmask)
       _dev.epl_phy[k-1].setup10Mb(lb, rxFlip, txFlip);
     else
       _dev.epl_phy[k-1].disable();
   }

#ifdef DBUG
   printv("Bringing all EPLs configured\n");
#endif

   //  Set default watermarks (jitter [bug#6892] and pause [bug#6896])
   //   _dev.jitter_cfg = 0x00181818;
   //  Increase jitter watermark to avoid rare tx uflow
   _dev.jitter_cfg = 0x00383818;
  _dev.queue_cfg_3 = 0xffffffff;  // no WRED
   for(unsigned k=0; k<=24; k++) {
     //     if (_defaultPorts & (1<<k))
     _dev.queue_cfg_2[k] = 0x10; // rx private = 16kB
     //     _dev.global_pause_wm[k] = 0x0fff0fff;  // large pause watermark?
     _dev.global_pause_wm[k] = 0x01200144;
     _dev.rx_pause_wm    [k] = 0xe500ef;  // pause hog watermarks for jumbo frames
   }

   //  Set in FmMacTable.cc
   //   _dev.ma_table_cfg = 1;    // best-effort lookup

#ifdef DBUG
   printv("Setup FID forwarding\n");
#endif

   //  Setup forwarding tables
   //   printv("FmTahoeMgr writing FID entries");
   for(unsigned k=0; k<FmSwitchConfig::Entries; k++) {
     _dev.fid_table[k] = cfg.swCfg.fid_table[k];
   }

#ifdef DBUG
   printv("Setup port forwarding\n");
#endif

   // Vlan Port association
   for(unsigned k=0; k<FmPort::MAX_PORT; k++) {
     FmPort p(k+1);
     unsigned v = 0;
     v |= FmTahoe::Port_Cfg_1::EnableLearning;
     v |= cfg.swCfg.port_vlan.defaultVlan[k]&0xfff;
     if      (cfg.swCfg.port_vlan.untagged.contains(p))
       v |= FmTahoe::Port_Cfg_1::DropTaggedFrames;
     else if (cfg.swCfg.port_vlan.tagged.contains(p))
       v |= FmTahoe::Port_Cfg_1::DropUntaggedFrames;
     else if (cfg.swCfg.port_vlan.strict.contains(p))
       v |= FmTahoe::Port_Cfg_1::FilterIngVlanViolations;

     _dev.port_cfg_1[k+1] = v;
   }

   // Port forwarding
   for(unsigned k=0; k<FmPort::MAX_PORT; k++)
     _dev.port_cfg_2[k+1] = cfg.swCfg.portfwd_table.forward[k];

#ifdef DBUG
   printv("Setup trunks\n");
#endif

   for(unsigned k=1; k<=FmPort::MAX_PORT; k++)
     _dev.trunk_port_map[k] = 0;

   for(unsigned k=0; k<FmSwitchConfig::Trunks; k++) {
     _dev.trunk_group_1[k] = 0;
     _dev.trunk_group_2[k] = 0;
     _dev.trunk_group_3[k] = 0;
   }

   for(unsigned k=0; k<FmSwitchConfig::Trunks; k++) {
     _trunk_set(cfg.swCfg.trunk[k],k);
   }

#ifdef DBUG
   printv("Setup VLANs\n");
#endif

   {
     bool luse_vlans=false;
     for(unsigned k=0; k<FmSwitchConfig::Entries; k++) {
       _dev.vid_table[k] = cfg.swCfg.vid_table[k];
       luse_vlans |= !cfg.swCfg.vid_table[k].empty();
     }
     if (luse_vlans)
       _dev.sys_cfg_2 = FmTahoe::Sys_Cfg_2::VlanTagEnable;
   }

#ifdef DBUG
   printv("Preload MACtable\n");
#endif

   for(unsigned k=0; k<SwmMacTable::Entries; k++) {
     if (cfg.swCfg.mac_table.entry[k].valid())
       _macTable->addEntry(cfg.swCfg.mac_table.entry[k]);
   }

#ifdef DBUG
   printv("Setup handlers\n");
#endif

   if (!_eplHandler) _eplHandler = new FmEplHandler(_dev);
   { const unsigned oneLanePorts =
       _config.portCfg.ports10Mb |
       _config.portCfg.ports100Mb |
       _config.portCfg.ports1Gb |
       _config.portCfg.ports2_5Gb;
     const unsigned fourLanePorts =
       _config.portCfg.ports4Gb |
       _config.portCfg.ports10Gb;
     for(unsigned k=0; k<24; k++) {
       FmPort port(k+1);
       unsigned pmask = 1<<unsigned(port);
       if (pmask & oneLanePorts)
	 _eplHandler->enableOneLanePort(port);
       if (pmask & fourLanePorts)
	 _eplHandler->enableFourLanePort(port);
     }
   }

   _dev.sys_cfg_1 = FmTahoe::Sys_Cfg_1::DropPause;

   registerHandler(LCI ,new FmDefaultHandler("LCI"));
   registerHandler(TRG ,new FmDefaultHandler("TRG"));
   registerHandler(FCTL,_macTable);
   registerHandler(MGR ,new FmMgrHandler    (&_dev));
   registerHandler(EPL ,_eplHandler);
   registerHandler(SEC ,new FmDefaultHandler("SEC"));
   registerHandler(VLN2,new FmDefaultHandler("VLN2"));
   registerHandler(VLN1,new FmDefaultHandler("VLN1"));
   registerHandler(PERR,new FmDefaultHandler("PERR"));

   //  Clear interrupt status registers
   v = _dev.global_epl_int_detect;
   v = _dev.lci_ip;
   v = _dev.interrupt_detect;

#ifdef DBUG
   printv("Set StoreAndForward matrix\n");
#endif

   setSAFMatrix();

#ifdef DBUG
   printv("Configure triggers\n");
#endif

   //
   //  Configure the triggers
   //
   for(unsigned k=0; k<16; k++) {
     _config.trigCfg[k] = cfg.trigCfg[k];
     configureTrigger(k,cfg.trigCfg[k]);
   }

   printv("FmTahoeMgr(%d) configure done",_dev.id());
   return 1;
}

const FmMacTable& FmTahoeMgr::macTable() const
{
  return const_cast<FmTahoeMgr*>(this)->macTable();
}

FmMacTable& FmTahoeMgr::macTable()
{
  if (!_macTable) _macTable = new FmMacTable(_dev);
  return *_macTable;
}

FmPortMask FmTahoeMgr::portsUp() const
{
  unsigned v = 1;
  for(unsigned k=0; k<24; k++) {
    unsigned u = _dev.epl_phy[k].pcs_ip;
    //    if (u & 0x2000)
    if (u == 0x2000)
      v |= (2<<k);
  }
  return FmPortMask(v);
}

cm::fm::PortSpeed FmTahoeMgr::portSpeed(FmPort p) const
{
  const FmPortConfig& pc = _config.portCfg;
  if (pc.ports10Gb .contains(p)) return cm::fm::X10Gb;
  if (pc.ports4Gb  .contains(p)) return cm::fm::X4Gb;
  if (pc.ports2_5Gb.contains(p)) return cm::fm::X2_5Gb;
  if (pc.ports1Gb  .contains(p)) return cm::fm::X1Gb;
  if (pc.ports100Mb.contains(p)) return cm::fm::X100Mb;
  if (pc.ports10Mb .contains(p)) return cm::fm::X10Mb;
  return cm::fm::None;
}

void FmTahoeMgr::handle_interrupt()
{
  unsigned is = _dev.interrupt_detect;

  if (is & FmTahoe::Interrupt_Detect::LCI_Int) { // lci_ip
    unsigned m = _dev.lci_im;
    unsigned p = _dev.lci_ip;

    p &= ~m;

    if ( p & FmTahoe::Lci_Ip::TxRdy ) {
      p          &= ~FmTahoe::Lci_Ip::TxRdy;
      _dev.lci_im = m | FmTahoe::Lci_Ip::TxRdy;
    }
    if ( p & FmTahoe::Lci_Ip::RxRdy ) {
      p          &= ~FmTahoe::Lci_Ip::RxRdy;
      _dev.lci_im = m | FmTahoe::Lci_Ip::RxRdy;
    }
    p &= 0xF8;
    _dev.lci_im = (m & 0x7) | p;
    if ( p ) {
      _handler[LCI]->call(p);
    }
  }
  if (is & FmTahoe::Interrupt_Detect::Trigger_Int) { // trigger_ip
    _genHandle(TRG,_dev.trigger_im,_dev.trigger_ip);
  }
  if (is & FmTahoe::Interrupt_Detect::Frame_Cntl_Int) {
    unsigned p = _dev.frame_ctrl_ip;
    if (p)
      _handler[FCTL]->call(p);
  }
  if (is & FmTahoe::Interrupt_Detect::Mgr_Int) { // mgr_ip
    _handler[MGR]->call(_dev.mgr_ip);
  }
  /*
  if (is & FmTahoe::Interrupt_Detect::EPL_Int) {
    unsigned p = _dev.global_epl_int_detect;
    if ( p ) {
      _handler[EPL]->call(p);
    }
  }
  */
  if (is & FmTahoe::Interrupt_Detect::Port_Security_Int) { // port_mac_sec_ip
    _genHandle(SEC,_dev.port_mac_sec_im,_dev.port_mac_sec_ip);
  }
  if (is & FmTahoe::Interrupt_Detect::Port_VLAN_Int2) { // port_vlan_ip_2
    _genHandle(VLN2,_dev.port_vlan_im_2,_dev.port_vlan_ip_2);
  }
  if (is & FmTahoe::Interrupt_Detect::Port_VLAN_Int1) { // port_vlan_ip_1
    _genHandle(VLN1,_dev.port_vlan_im_1,_dev.port_vlan_ip_1);
    /*
    printf("rxDmaFifo(%d)\n",id());
    _logFifo((unsigned*)0xe0002000,8);
    printf("txDmaFifo(%d)\n",id());
    _logFifo((unsigned*)0xe0002800,8);
    */
  }
  if (is & FmTahoe::Interrupt_Detect::Parity_Error_Int) { // perr_ip
    _genHandle(PERR,_dev.perr_im,_dev.perr_ip);
  }
}

FmHandler* FmTahoeMgr::registerHandler(FmTahoeMgr::InterruptType t,
				       FmHandler* h)
{
  FmHandler* o = _handler[t];
  _handler[t] = h;
  return o;
}

void FmTahoeMgr::_genHandle(FmTahoeMgr::InterruptType t,
			    FmMReg& im,
			    FmMReg& ip)
{
  unsigned m = im;
  unsigned p = ip;
  //  printf("genHandle(%x) ip %x im %x\n",t,p,m);
  p &= ~m;
  if ( p ) {
    im = m & ~p;
    _handler[t]->call(p);
  }
}

static ether_addr_t mac_addr_buffer;

unsigned char* FmTahoeMgr::getMacAddress() const
{
  getMacAddress(&mac_addr_buffer.ether_addr_octet[0]);
  return &mac_addr_buffer.ether_addr_octet[0];
}

void FmTahoeMgr::getMacAddress( unsigned char* addr ) const
{
  unsigned uaddr = _dev.sys_cfg_3;
  unsigned laddr = _dev.sys_cfg_4;
  addr[0] = (uaddr>> 8)&0xff;
  addr[1] = (uaddr>> 0)&0xff;
  addr[2] = (laddr>>24)&0xff;
  addr[3] = (laddr>>16)&0xff;
  addr[4] = (laddr>> 8)&0xff;
  addr[5] = (laddr>> 0)&0xff;
}

void FmTahoeMgr::setMacAddress( unsigned char* addr )
{
  _dev.sys_cfg_3 =
    ((addr[0]&0xff) << 8) |
    ((addr[1]&0xff) << 0);
  _dev.sys_cfg_4 =
    ((addr[2]&0xff) << 24) |
    ((addr[3]&0xff) << 16) |
    ((addr[4]&0xff) << 8) |
    ((addr[5]&0xff) << 0);
}

void FmTahoeMgr::setSAFMatrix( )
{
  FmConfig& config = _config;

  //  Set store and forward matrix
  for(unsigned k=1; k<=24; k++) {
    const unsigned pmask = (1<<k);
    unsigned mask = 0xffffffff;
    if ( config.portCfg.ports10Gb  & pmask )  mask = ~config.portCfg.ports10Gb;
    if ( config.portCfg.ports4Gb   & pmask )  mask = ~config.portCfg.ports4Gb;
    if ( config.portCfg.ports2_5Gb & pmask )  mask = ~config.portCfg.ports2_5Gb;
    if ( config.portCfg.ports1Gb   & pmask )  mask = ~config.portCfg.ports1Gb;
    if ( config.portCfg.ports100Mb & pmask )  mask = ~config.portCfg.ports100Mb;
    if ( config.portCfg.ports10Mb  & pmask )  mask = ~config.portCfg.ports10Mb;
    _dev.saf_matrix[k] = mask;
  }
}

void FmTahoeMgr::configurePort(FmPort fmPort,
			       cm::fm::PortSpeed speed,
			       unsigned rxFlip,
			       unsigned txFlip,
			       unsigned drive,
                               unsigned trunk)
{
  FmConfig& config = _config;

  unsigned port = unsigned(fmPort)-1;

  FmTahoe::disableInterrupts(_dev.id());

  unsigned v,p;
  const unsigned pmask = 1<<(port+1);

  v = _dev.port_reset;
  v |= pmask;
  _dev.port_reset = v;

  unsigned ports312MHz = config.portCfg.ports10Gb | config.portCfg.ports2_5Gb;
  if (speed == cm::fm::X2_5Gb || speed == cm::fm::X10Gb)
    ports312MHz |= pmask;
  else
    ports312MHz &= ~pmask;
  p = 0x1fffffe & ~ports312MHz;
  v = _dev.port_clk_sel;
  if (v != p) {
    //    printf("Updating CLK_SEL %x -> %x\n",v,p);
    _dev.port_clk_sel = p;
  }

  v = _dev.port_reset;
  if (speed == cm::fm::None) {
    p = v | pmask;
    _dev.epl_phy[port].disable    ();
  }
  else {
    p = v & ~pmask;
  }

  if (p != v) {
    //    printf("Updating PORT_RESET %x -> %x\n",v,p);
    _dev.port_reset = p;
  }

  if (!_eplHandler) _eplHandler = new FmEplHandler(_dev);
  unsigned fourLane = config.portCfg.ports4Gb | config.portCfg.ports10Gb;
  unsigned oneLane  =
    config.portCfg.ports10Mb |
    config.portCfg.ports100Mb |
    config.portCfg.ports1Gb |
    config.portCfg.ports2_5Gb;
  if (speed > cm::fm::X2_5Gb && (pmask & ~fourLane))
    _eplHandler->enableFourLanePort(fmPort);
  else if (speed > cm::fm::None && (pmask & ~oneLane))
    _eplHandler->enableOneLanePort(fmPort);

  config.portCfg.ports10Mb  &= ~pmask;
  config.portCfg.ports100Mb &= ~pmask;
  config.portCfg.ports1Gb   &= ~pmask;
  config.portCfg.ports2_5Gb &= ~pmask;
  config.portCfg.ports4Gb   &= ~pmask;
  config.portCfg.ports10Gb  &= ~pmask;

  if (rxFlip) config.portCfg.portsRxFlip |= pmask;
  if (txFlip) config.portCfg.portsTxFlip |= pmask;

  v = _dev.trunk_port_map[fmPort];
  if (v & (1<<4)) {               // is mapped to a trunk
    if (trunk == 0 || speed==cm::fm::None) {
      unsigned group = v&0xf;
      _trunk_remove(fmPort, group);
    }
    else if ((trunk-1) != (v&0xf)) {  // wrong trunk
      unsigned group = v&0xf;
      _dev.epl_phy[port].disable();
      _trunk_remove(fmPort, group);
      _trunk_add   (fmPort, trunk-1);
    }
  }
  else if (trunk) {
    _trunk_add(fmPort, trunk-1);
  }

  switch(speed) {
  case cm::fm::None  :
    break;
  case cm::fm::X10Mb  :
    _dev.epl_phy[port].setup10Mb (0);
    config.portCfg.ports10Mb  |=  pmask;
    break;
  case cm::fm::X100Mb :
    _dev.epl_phy[port].setup100Mb(0);
    config.portCfg.ports100Mb |=  pmask;
    break;
  case cm::fm::X1Gb:
    _dev.epl_phy[port].setup1Gb  (0,rxFlip,txFlip);
    config.portCfg.ports1Gb   |=  pmask;
    break;
  case cm::fm::X2_5Gb:
    _dev.epl_phy[port].setup2_5Gb(0,rxFlip,txFlip);
    config.portCfg.ports2_5Gb |=  pmask;
    break;
  case cm::fm::X4Gb:
    _dev.epl_phy[port].setup4Gb (0,rxFlip,txFlip);
    config.portCfg.ports4Gb  |=  pmask;
    break;
  case cm::fm::X10Gb:
    _dev.epl_phy[port].setup10Gb (0,rxFlip,txFlip,drive);
    config.portCfg.ports10Gb  |=  pmask;
    break;
  }

  setSAFMatrix();

  FmTahoe::enableInterrupts(_dev.id());
}

void FmTahoeMgr::configureLCI()
{
  //
  //  Setup the CPU interface
  //
  unsigned v = _dev.lci_cfg;
  v |= FmTahoe::Lci_Cfg::BigEndian;  //  Setup the CPU as big-endian
  v |= FmTahoe::Lci_Cfg::TxCompCRC;  //  Compute the CRC for tx packets
  v |= FmTahoe::Lci_Cfg::RxEnable;   //  Enable the receipt of packets
  _dev.lci_cfg = v;
}

void FmTahoeMgr::configureTrigger(unsigned id, const FmTriggerConfig& cfg)
{
  _unusedTriggers |= (1<<id);
  _dev.trigger_cfg[id] = 0x3; // disable
  if (cfg.enable) {
    unsigned v =
      ( (cfg.mac_entry_trigger&0xf) << 28 ) |
      ( (cfg.new_priority&0xf) << 24 ) |
      ( (cfg.mirror_port&0x1f) << 19 ) |
      ( (cfg.action) << 16 );
    if (cfg.priorityMask) {
      v |= (1<<10);
      _dev.trigger_pri[id] = cfg.priorityMask;
    }
    if (cfg.sourcePortMask) {
      v |= (1<<4);
      _dev.trigger_rx [id] = cfg.sourcePortMask;
    }
    if (cfg.destinationPortMask) {
      v |= (1<<5);
      _dev.trigger_tx [id] = cfg.destinationPortMask;
    }
    if (cfg.vlanMatch)
      v |= (1<<6);
    if (cfg.xcast==FmTriggerConfig::Unicast  )      v |= (1<<7);
    if (cfg.xcast==FmTriggerConfig::Broadcast)      v |= (1<<8);
    if (cfg.xcast==FmTriggerConfig::Multicast)      v |= (1<<9);
    switch( cfg.sourceMacMatch ) {
    case FmTriggerConfig::Hit:    v |= (1<<0); break;
    case FmTriggerConfig::Miss:   v |= (1<<1); break;
    case FmTriggerConfig::OneHit: v |= (1<<11); break;
    default: break;
    }
    switch( cfg.destinationMacMatch ) {
    case FmTriggerConfig::Hit:    v |= (1<<2); break;
    case FmTriggerConfig::Miss:   v |= (1<<3); break;
    case FmTriggerConfig::OneHit: v |= (1<<11); break;
    default: break;
    }
    _dev.trigger_cfg[id] = v;
    _unusedTriggers &= ~(1<<id);
  }
}

unsigned FmTahoeMgr::unusedTriggers() const
{
  return _unusedTriggers;
}

unsigned FmTahoeMgr::lowestPriorityUnusedTrigger() const
{
  for(int i=31; i>=0; i--)
    if (_unusedTriggers&(1<<i))
      return i;
  return 63;
}

unsigned FmTahoeMgr::highestPriorityUnusedTrigger() const
{
  for(int i=0; i<32; i++)
    if (_unusedTriggers&(1<<i))
      return i;
  return 63;
}

void FmTahoeMgr::changePortMulticastGroupMembership(FmPortMask fmPorts,
						    unsigned   mcast,
						    bool       isListening)
{
  FmConfig& config = _config;
  fmPorts |= config.protocolCfg.mcastFlood;

  ether_addr_t addr;
  addr.ether_addr_octet[0] = 0x01;
  addr.ether_addr_octet[1] = 0x00;
  addr.ether_addr_octet[2] = 0x5e;
  addr.ether_addr_octet[3] = 0x7F & (mcast>>16);
  addr.ether_addr_octet[4] = 0xFF & (mcast>> 8);
  addr.ether_addr_octet[5] = 0xFF & (mcast>> 0);

  const unsigned vlanId = 0;
  const unsigned locked = 1;  // new entries should get the locked (static) setting
  SwmMacTableEntry e( addr, fmPorts, vlanId, 0, locked, 1);
  if (!_macTable) _macTable = new FmMacTable(_dev);
  int index = _macTable->findMatch( e );
  if (index >= 0) {
    e = _macTable->entry( index );
    if (isListening)
      e.set_port_mask( e.port_mask() | fmPorts );
    else {
      unsigned m = unsigned(e.port_mask()) & ~ unsigned(fmPorts);
      if (m)
	e.set_port_mask( FmPortMask(m) | config.protocolCfg.mcastFlood );
      else
	e.invalidate();
    }
    _macTable->updateEntry( index, e );
  }
  else if (isListening) {
    _macTable->addEntry( e );
  }
}

void FmTahoeMgr::trapIGMP(bool enable)
{
  if (enable)  _dev.sys_cfg_1 |=  FmTahoe::Sys_Cfg_1::TrapIGMP;
  else         _dev.sys_cfg_1 &= ~FmTahoe::Sys_Cfg_1::TrapIGMP;
}

unsigned FmTahoeMgr::id() const { return _dev.id(); }

FmTahoe& FmTahoeMgr::dev() { return _dev; }

void FmTahoeMgr::_logFifo(unsigned* fifo,unsigned n) const
{
  char buff[256];
  char* bptr = buff;
  while(n--) {
    bptr += sprintf(bptr," %08x",*fifo);
    fifo++;
  }
  char* const bEnd = bptr;
  bptr = buff;
  while( bptr < bEnd ) {
    printf("%72.72s\n",bptr);
    bptr += 72;
  }
}

void FmTahoeMgr::hop(FmPortMask mask, bool allow)
{
  unsigned forward = FmPortMask::allPorts();
  if (!allow) forward ^= mask;
  for(unsigned i=1; i<=FmPort::MAX_PORT; i++) {
    if (mask.contains(FmPort(i)))
      _dev.port_cfg_2[i] = _config.swCfg.portfwd_table.forward[i-1] & forward;
  }
}

#if 0
static int _find_trunk_port(unsigned v, FmPort port)
{
  for(int k=0; k<6; k++, v>>=5)
    if ((v&0x1f)==port) return k;
  return -1;
}
#endif

void FmTahoeMgr::_trunk_set   (FmPortMask m, unsigned trunk)
{
  // Disable hops within a trunk group
  hop(m,false);

  unsigned k = trunk;
  unsigned n = 0;
  unsigned g1 = 0;
  unsigned g2 = 0;
  unsigned i;
  for(i=1; i<=FmPort::MAX_PORT && m; i++) {
    FmPortMask p(1<<i);
    if (m & p) {
      m &= ~p;
      if (n>=12) {
        // link-aggregate group is full
        // isolate this port
        _dev.port_cfg_2[i] = 0;
        // 
        // We also have a problem if we receive traffic
        // on this port (MAC table gets updated)
        for(unsigned j=1; j<=FmPort::MAX_PORT; j++)
          _dev.port_cfg_2[j] &= ~p;
      }
      else {
        if (n<6)       g1 |= i<<(5*n);
        else if (n<12) g2 |= i<<(5*(n-6));
        
        _dev.trunk_port_map[i] = ((1<<4)|k);
        n++;
      }
    }
  }
  
  _dev.trunk_group_1[k] = g1;
  _dev.trunk_group_2[k] = g2;
  _dev.trunk_group_3[k] =  n;
}

void FmTahoeMgr::_trunk_add   (FmPort port, unsigned trunk)
{
  _config.swCfg.trunk[trunk] |= FmPortMask(1<<port);
  _trunk_set(_config.swCfg.trunk[trunk],trunk);
}

void FmTahoeMgr::_trunk_remove(FmPort port, unsigned trunk)
{
  _dev.trunk_port_map[port] = 0;

  _config.swCfg.trunk[trunk] &= ~FmPortMask(1<<port);
  _trunk_set(_config.swCfg.trunk[trunk],trunk);

  _dev.port_cfg_2[port] = _config.swCfg.portfwd_table.forward[port-1];
}

