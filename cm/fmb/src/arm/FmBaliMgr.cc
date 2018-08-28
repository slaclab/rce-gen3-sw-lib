
#include "fmb/FmBaliMgr.hh"

#include "cm_svc/Print.hh"

/**
#include "fm/FmMacTable.hh"
**/
#include "fmb/FmBali.hh"
#include "fmb/SwmGlortDestEntry.hh"
#include "fmb/SwmGlortRamEntry.hh"
/**
#include "fm/FmMgrHandler.hh"
#include "fm/FmEplHandler.hh"
#include "fm/FmHandler.hh"
#include "fm/FmConfig.hh"
**/
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>

//#define DBUG

static const unsigned _testPort = 5;

using namespace cm::service;
using Bali::FmBali;
using Bali::FmBaliMgr;

static bool _boot_status(FmBali& d)
{
  volatile unsigned v;
#if 0
  unsigned k=1024;
  do {
    v = d.boot_status;
  } while( v==0 && --k );
  if (!k) {
    printf("FAIL: FmBali_init boot_status poll timeout (%x)\n",v);
    return false;
  }
#else
  printf("_boot_status polling with NO TIMEOUT...\n\r");
  do {
    v = d.boot_status;
  } while( v==0 );
#endif
  return true;
}

/**
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
**/

FmBaliMgr::FmBaliMgr( FmBali& d, const FmConfig& cfg ) :
  _dev(d),
  /**
     _macTable(0),
     _eplHandler(0),
  **/
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
void FmBaliMgr::reset(const FmConfig& cfg)
{
  timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 20000000; // (20 msec should be sufficient)

  volatile unsigned v;
  unsigned int sr;

   FmReg::init();

   _dev.resetCore();

   //  Hardware initialization goes here
   _dev.resetChip();
   nanosleep(&ts, 0);

   _dev.releaseChip();
   nanosleep(&ts, 0);

   /*  Read vital product data register */
   {
     v = _dev.vpd;
     printf("FmBali_init: Read vpd 0x%08x ",v);
     if (((v >> 12) & 0xffff) != 0xae19) {
       throw(std::string("Incorrect vpd"));
     }
   }

   v = _dev.vpd_info_1[0];
   printf("vpd %s\n",(v&1)?"locked":"unlocked");

   v = _dev.vpd_info_1[1];
   printf("maskRev %x\n",v&0xff);
   
   /***************************************************
    * Step 2: Initialize the async memory repair.
    **************************************************/
   printf("FmBali_init:  Async memory repair\n\r");
   _dev.boot_ctrl=1;
   if (!_boot_status(_dev)) return;

   /***************************************************
    * Step 3: Initialize the scheduler.
    **************************************************/
   printf("FmBali_init:  Initialize scheduler\n\r");
   _dev.boot_ctrl=0;
   if (!_boot_status(_dev)) return;


   /***************************************************
    * Step 4: Initialize the frame handler PLL.
    **************************************************/
   v = _dev.pll_fh_stat;
   printf("FmBali_init: Read pll_fh_stat %08x\n\r",v);
   
   v = _dev.pll_fh_ctrl;
   printf("FmBali_init: Read pll_fh_ctrl %08x\n\r",v);
   
   v &= ~2;
   _dev.pll_fh_ctrl=v;
   
   printf("polling\n\r");

   /*  Poll PLL status until zero or timeout */
   { unsigned k=1024;
     do {
       v = _dev.pll_fh_stat;
     } while( v==0 && --k );
     if (!k) {
       printf("FAIL: FmBali_init pll_fh_stat poll1 timeout (%x)\n\r",v);
       return;
     }
   }

   /***************************************************
    * Step 5: Bring all modules out of reset.
    **************************************************/
   printf("soft_reset value before: "); // FIXME
   sr = _dev.soft_reset;
   printf("0x%08x\n\r", sr);

   printf("clearing soft reset\n\r");
   _dev.soft_reset=0;

   printf("soft_reset value after: "); // FIXME
   sr = _dev.soft_reset;
   printf("0x%08x\n\r", sr);

   /***************************************************
    * Step 6: Bring MSB/EPL0 out of reset.
    **************************************************/
#if 1
   dumpEplPortCtrl(__LINE__);
#endif

#if 0
   printf("enable EPL0\n\r");
   _dev.epl_port_ctrl[0]=0x1;
   _dev.epl_port_ctrl[0]=0x9;
#else
   releasePort(0);
#endif
   
   /***************************************************
    * Step 7: Enable the FFU.
    **************************************************/
   v = _dev.sys_cfg_8;
   printf("FmBali_init:  sys_cfg_8 before = %08x\n\r",v);

   printf("FmBali_init:  Enable FFU\n\r");
   v |= 0x1;
   _dev.sys_cfg_8=v;

   /***************************************************
    * Step 8: Initialize the sync memory repair.
    **************************************************/
   printf("FmBali_init:  Sync memory repair\n\r");
   _dev.boot_ctrl=3;
   if (!_boot_status(_dev)) return;

   /***************************************************
    * Step 9: Initialize the FFU.
    **************************************************/
   printf("FmBali_init:  Program FFU_INIT_SLICE\n\r");
   v = _dev.ffu_init_slice;
   printf("FFU_INIT_SLICE = %08x\n",v);
   _dev.ffu_init_slice=v;

   //   if (!_boot_status(_dev)) return;

   /***************************************************
    * Step 10: Bring all ports out of reset.
    *          Note that we need to do this before
    *          memory init so that we initialize the
    *          tag table.
    **************************************************/
   printf("FmBali_init: Bring all ports out of reset.\n\r");
   for (int ii=1; ii<=24; ii++) {
     releasePort(ii);

//   TODO
//   (void)switchPtr->WriteUINT32( sw, FM4000_SERDES_CTRL_3(i),
//                                 ( serdesHysteresis | (pcsHysteresis << 20) ) );
   }

#if 1
   dumpEplPortCtrl(__LINE__);
#endif

   /***************************************************
    * Step 11: Initialize the memory.
    **************************************************/
   printf("FmBali_init:  Memory Initialization\n\r");
   _dev.boot_ctrl=2;
   if (!_boot_status(_dev)) return;

   /***************************************************
    * Step 12: Disable the FFU until later.
    **************************************************/
   printf("FmBali_init:  Disable FFU\n\r");
   _dev.sys_cfg_8=0;
              
   /***************************************************
    * Step 13: Miscellaneous port configuration.
    **************************************************/
   
   /***************************************************
    * Fix For Errata #17 on CPU port.
    * Setting priority counters to count by
    * traffic class
    **************************************************/
   v = _dev.stats_cfg[0];
   v |= (1<<15);
   _dev.stats_cfg[0] = v;

   for (int ii=1; ii<=24; ii++) {
     /***************************************************
      * Turn off check end bit, due to bali bringup
      * issues. Turn on local fault and remore fault 
      * response. 
      **************************************************/
     //  Not sure this accomplishes anything for us
     v = _dev.epl_phy[ii].pcs_cfg_1;
     v &= (1<<31);
     v |= (3<<21);
     _dev.epl_phy[ii].pcs_cfg_1 = v;

     /***************************************************
      * Enable the drain bit.  The link debouncer will
      * properly reset this bit when the link goes up.
      **************************************************/
     v = _dev.epl_phy[ii].mac_cfg_2;
     v |= (1<<9);  // drain Tx
     _dev.epl_phy[ii].mac_cfg_2 = v;

     /***************************************************
      * Set sync buf config for proper PCS configuration.
      **************************************************/
     _dev.epl_phy[ii].syncbuf_cfg = 0x3f00;

     /***************************************************
      * Set the jitter watermark to the proper 
      * experimentally derived value.
      **************************************************/
     _dev.epl_phy[ii].jitter_timer = 0x1c1006;

     /***************************************************
      * Lower the number of active stats groups.
      **************************************************/
     // if switch version is fm4224_a1

     /***************************************************
      * Fix For Errata #17
      * Setting priority counters to count by
      * traffic class
      **************************************************/ 
     v = _dev.stats_cfg[ii];
     v |= (1<<15);
     _dev.stats_cfg[ii] = v;

     /***************************************************
      * Set the learning mode as needed.
      **************************************************/
//      v = _dev.port_cfg_3[ii];
//      v &= ~(3<<5);
//      _dev.port_cfg_3[ii] = v;

     /***************************************************
      * Set default src glort
      **************************************************/
     _dev.port_cfg_isl[ii] = ii;

     /***************************************************
      * Set the learning rate limiter 
      **************************************************/
     // skipping
  }

#ifdef PROFILE
   time = get_timer(time);
   printf("FmBali_init startup complete %lu ms\n\r", time);
#endif

   v = _dev.sys_cfg_1;
   printf("FmBali_init: Read sys_cfg_1 %08x\n\r",v);

   if ( ((v = (_dev.sys_cfg_1))&0x7c2f) != 0x1c2f) {
     printf("FmBali_init fail: sys_cfg_1 & 0x7c2f = 0x%08x (not 0x1c2f)\n\r",v);
     return;
   }

#ifdef PROFILE
   time = get_timer(0);
#endif

   configure(cfg);
}

FmBaliMgr::~FmBaliMgr()
{
  /**
  for(unsigned k=0; k<nInterrupts; k++)
    delete _handler[k];
  **/
}

int FmBaliMgr::configure(const FmConfig& cfg)
{
  volatile unsigned v;

  //
  //  Where do we write the static MAC entries?
  //

   _config = cfg;

   const unsigned ports312MHz = cfg.portCfg.ports10Gb | cfg.portCfg.ports2_5Gb;

   for(unsigned k=1; k<=FmPort::MAX_PORT; k++) {

    // NOTE!  epl_port_ctrl bit 0 (reset) is active low!
    //        setting bit 0 puts the port OUT of reset.

     v = (_dev.epl_port_ctrl[k]);
     v = 0x1;
     _dev.epl_port_ctrl[k]=v;

     if (ports312MHz & (1<<k))
       v &= ~0x4;
     else
       v |= 0x4;
     _dev.epl_port_ctrl[k]=v;

     // data path init already done in releasePort()
     v &= ~0x8;
     _dev.epl_port_ctrl[k]=v;

     Bali::FmEplRegs* p = &_dev.epl_phy[k-0];           // EPL_PORT_OFFSET=0
     const unsigned pmask = (1<<k);
     int lb     = cfg.portCfg.portsLoopback & (1<<k);
     int rxFlip = cfg.portCfg.portsRxFlip   & (1<<k);
     int txFlip = cfg.portCfg.portsTxFlip   & (1<<k);
     if (cfg.portCfg.ports10Gb & pmask)
       p->setup10Gb (lb, rxFlip, txFlip, 0);
     else if (cfg.portCfg.ports4Gb & pmask)
       p->setup4Gb  (lb, rxFlip, txFlip);
     else if (cfg.portCfg.ports2_5Gb & pmask)
       p->setup2_5Gb(lb, rxFlip, txFlip);
     else if (cfg.portCfg.ports1Gb & pmask)
       p->setup1Gb  (lb, rxFlip, txFlip);
     else if (cfg.portCfg.ports100Mb & pmask)
       p->setup100Mb(lb, rxFlip, txFlip);
     else if (cfg.portCfg.ports10Mb & pmask)
       p->setup10Mb (lb, rxFlip, txFlip);
     else
       p->disable   ();
   }

#ifdef PROFILE
   time = get_timer(time);
   printf("FmBali_init epl config %lu ms\n", time);
#endif

#ifdef PROFILE
   time = get_timer(0);
#endif

   /*  Setup forwarding tables (STP) */
   for(unsigned k=0; k<FmSwitchConfig::Entries; k++) {
     _dev.ingress_fid_table[k] = cfg.swCfg.fid_table[k];
     _dev.egress_fid_table [k] = cfg.swCfg.fid_table[k];
   }

   /* Vlan Port association */
   for(unsigned k=0; k<FmPort::MAX_PORT; k++) {
     FmPort p = FmPort(k+1);
     unsigned defv = cfg.swCfg.port_vlan.defaultVlan[k]&0xfff;
     unsigned v = (_dev.port_cfg_1[k+1]);
     v &= ~0xfff;
     v |= defv;
     if      (cfg.swCfg.port_vlan.untagged & p)
       v |= (1<<22);
     else if (cfg.swCfg.port_vlan.tagged & p)
       v |= (1<<23);
     _dev.port_cfg_1[k+1]=v;

     v = (_dev.port_cfg_3[k+1]);
     if (cfg.swCfg.port_vlan.strict & p)
       v |= (1<<0);
     _dev.port_cfg_3[k+1]=v;

     for(unsigned i=0; i<0x80; i++) {
       _dev.epl_phy[k].vlantag_table[i] = 0;
     }
     if (cfg.swCfg.port_vlan.tagged & p)
       _dev.epl_phy[k].vlantag_table[defv>>5] = 1<<(defv&0x1f);
   }

   /* Port forwarding */
   for(unsigned k=1; k<=FmPort::MAX_PORT; k++)
     _dev.port_cfg_2[k]=cfg.swCfg.portfwd_table.forward[k-1];

   /* Flood GLORT */
   _dev.glort_dest_table[0] = Bali::SwmGlortDestEntry(FmPortMask::phyPorts());
   _dev.glort_cam[0]=(0) | (0xffff<<16);
   _dev.glort_ram[0] = Bali::SwmGlortRamEntry(0);

   /* Individual port GLORT */
   for(unsigned k=1; k<=FmPort::MAX_PORT; k++) {
     _dev.glort_dest_table[k] = Bali::SwmGlortDestEntry(FmPortMask(1<<k));
     _dev.glort_cam[k]= (k&0xffff) | (0xffff<<16);
     _dev.glort_ram[k] = Bali::SwmGlortRamEntry(k);
   }

   /* LAG */
   for(unsigned k=0; k<FmSwitchConfig::Trunks; k++) {
     unsigned m = cfg.swCfg.trunk[k];
     unsigned q = k+FmPort::MAX_PORT+1;
     if (m) {
       unsigned i;
       unsigned n=0;

       for(i=1; i<=FmPort::MAX_PORT; i++) {
         if (m & (1<<i)) n++;
       }

       unsigned p=0;
       for(i=1; i<=FmPort::MAX_PORT; i++) {
         if (m & (1<<i)) {
           v = ((n&0xf)<<0) |
             ((p&0xf)<<4) |
             (1<<9);
           _dev.lag_cfg[i]=v;
           p++;
         }
       }

       v = ((k+1)<<8) | (8<<16);
       _dev.canonical_glort_cam[k]=v;

       _dev.glort_dest_table[q] = Bali::SwmGlortDestEntry(FmPortMask(m));

       v = ((k+1)<<8) | (0xff00);
       _dev.glort_cam[q]=v;

       _dev.glort_ram[q] = Bali::SwmGlortRamEntry(q, 0, 4, 0, 0, n, 0);
     }
   }

   /* VLAN table */
   for(unsigned k=0; k<FmSwitchConfig::Entries; k++) {
     _dev.ingress_vid_table[k]=cfg.swCfg.vid_table[k];
     _dev.egress_vid_table [k]=cfg.swCfg.vid_table[k];
   }

#ifdef PROFILE
   time = get_timer(time);
   printf("FmBali init switch tables %lu ms\n", time);
#endif

   /* Turn off learning
   **    for(k=0; k<24; k++) {
   **      v = _dev.port_cfg_1[k];
   **      _dev.port_cfg_1[k] = v &~ 0x100000;
   **    } */
   /*
   _dev.frame_ctrl_im= 
             (_dev.frame_ctrl_im) & 
             ~(Entry_Aged | Entry_Learned | Bin_Full | FIFO_Full);
   _dev.mgr_im=0;
   */
   _dev.perr_im=0;
   _dev.lci_im= FmBali::Lci_Ip::RxEOT | FmBali::Lci_Ip::RxRdy;
   _dev.port_vlan_im_1=0;
   _dev.port_vlan_im_2=0;
   _dev.port_mac_sec_im=0;
   _dev.trigger_im[0]=0;
   _dev.trigger_im[1]=0;

   //  Clear interrupt status registers
   _dev.global_epl_int_detect = 0xffffffff;
   _dev.lci_ip                = 0xffffffff;
   _dev.interrupt_detect      = 0xffffffff;

   _dev.sys_cfg_7=0x7530;   /* age entries once/2.4ms, whole table/40s */
/*    _dev.global_pause_wm[0],0xffffffff); */
/*    _dev.rx_pause_wm[0],0xffffffff);  /\* no pause for CPU *\/ */

/*    _dev.queue_cfg_5,1000); */
/*    for(k=0; k<=24; k++) */
/*      _dev.queue_cfg_1[k],0x03ff02ff);  /\* allow hog ports   { [9:0] = 0xff (works), 0x3ff (broken) } *\/ */

   /*  Prevent traps  */
   _dev.sys_cfg_1 = 0;
   _dev.cpu_trap_mask_fh=0;       /* drop all traps */

   _dev.frame_time_out=0x989680;  /* 5 minutes */

   setSAFMatrix();

   printf("FmBali_init SUCCESS\n\r");
#if 1

#endif
   return 1;
}

FmPortMask FmBaliMgr::portsUp() const
{
  unsigned v = 0;
//for(unsigned k=0; k<24; k++) {
  for(unsigned k=1; k<25; k++) {
    unsigned u = _dev.epl_phy[k].pcs_ip;
//  if (u == 0x2000)
    if (u & 0x2000)
      v |= (1<<k);
  }
  return FmPortMask(v);
}

cm::fm::PortSpeed FmBaliMgr::portSpeed(FmPort p) const
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

/**
void FmBaliMgr::handle_interrupt()
{
  unsigned is = _dev.interrupt_detect;

  if (is & FmBali::Interrupt_Detect::LCI_Int) { // lci_ip
    unsigned m = _dev.lci_im;
    unsigned p = _dev.lci_ip;

    p &= ~m;

    if ( p & FmBali::Lci_Ip::TxRdy ) {
      p          &= ~FmBali::Lci_Ip::TxRdy;
      _dev.lci_im = m | FmBali::Lci_Ip::TxRdy;
    }
    if ( p & FmBali::Lci_Ip::RxRdy ) {
      p          &= ~FmBali::Lci_Ip::RxRdy;
      _dev.lci_im = m | FmBali::Lci_Ip::RxRdy;
    }
    p &= 0xF8;
    _dev.lci_im = (m & 0x7) | p;
    if ( p ) {
      _handler[LCI]->call(p);
    }
  }
  if (is & FmBali::Interrupt_Detect::Trigger_Int) { // trigger_ip
    _genHandle(TRG,_dev.trigger_im,_dev.trigger_ip);
  }
  if (is & FmBali::Interrupt_Detect::Frame_Cntl_Int) {
    unsigned p = _dev.frame_ctrl_ip;
    if (p)
      _handler[FCTL]->call(p);
  }
  if (is & FmBali::Interrupt_Detect::Mgr_Int) { // mgr_ip
    _handler[MGR]->call(_dev.mgr_ip);
  }
#if 0
  if (is & FmBali::Interrupt_Detect::EPL_Int) {
    unsigned p = _dev.global_epl_int_detect;
    if ( p ) {
      _handler[EPL]->call(p);
    }
  }
#endif
  if (is & FmBali::Interrupt_Detect::Port_Security_Int) { // port_mac_sec_ip
    _genHandle(SEC,_dev.port_mac_sec_im,_dev.port_mac_sec_ip);
  }
  if (is & FmBali::Interrupt_Detect::Port_VLAN_Int2) { // port_vlan_ip_2
    _genHandle(VLN2,_dev.port_vlan_im_2,_dev.port_vlan_ip_2);
  }
  if (is & FmBali::Interrupt_Detect::Port_VLAN_Int1) { // port_vlan_ip_1
    _genHandle(VLN1,_dev.port_vlan_im_1,_dev.port_vlan_ip_1);
#if 0
    printf("rxDmaFifo(%d)\n",id());
    _logFifo((unsigned*)0xe0002000,8);
    printf("txDmaFifo(%d)\n",id());
    _logFifo((unsigned*)0xe0002800,8);
#endif
  }
  if (is & FmBali::Interrupt_Detect::Parity_Error_Int) { // perr_ip
    _genHandle(PERR,_dev.perr_im,_dev.perr_ip);
  }

  //  Clear interrupt bits (was clear on read in Tahoe)
  _dev.lci_ip          = 0x3;
  _dev.trigger_ip[0]   = 0xffffffff;
  _dev.trigger_ip[1]   = 0xffffffff;
  _dev.frame_ctrl_ip   = 0xffffffff;
  _dev.mgr_ip          = 0xffffffff;
  _dev.port_mac_sec_ip = 0xffffffff;
  _dev.port_vlan_ip_1  = 0xffffffff;
  _dev.port_vlan_ip_2  = 0xffffffff;
}

FmHandler* FmBaliMgr::registerHandler(FmBaliMgr::InterruptType t,
				       FmHandler* h)
{
  FmHandler* o = _handler[t];
  _handler[t] = h;
  return o;
}

void FmBaliMgr::_genHandle(FmBaliMgr::InterruptType t,
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
**/

static ether_addr_t mac_addr_buffer;

unsigned char* FmBaliMgr::getMacAddress() const
{
  getMacAddress(&mac_addr_buffer.ether_addr_octet[0]);
  return &mac_addr_buffer.ether_addr_octet[0];
}

void FmBaliMgr::getMacAddress( unsigned char* addr ) const
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

void FmBaliMgr::setMacAddress( unsigned char* addr )
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

void FmBaliMgr::setSAFMatrix( )
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

void FmBaliMgr::dumpEplPortCtrl(int line)
{
   printf("[%s at line %d]\n\r", __FUNCTION__, line);
   for (int ii=0; ii<=24; ii++) {
     unsigned jj = _dev.epl_port_ctrl[ii];
     printf("  EPL_PORT_CTRL[%02d]: 0x%08x ", ii, jj);
     printf(" %s", (jj & 1) ? "(active)" : "(reset mode)");
     printf(" %s\n\r", (ii == 0) ? "(cpu port)" : "");
   }
}

void FmBaliMgr::releasePort(int port)
{
  volatile unsigned v;

  printf("[releasePort(%d)]\n\r", port);
  v = _dev.epl_port_ctrl[port];

  /***************************************************
   * We must set the datapath initialization bit
   * before we exit reset.
   **************************************************/
  v &= ~8;
  _dev.epl_port_ctrl[port] = v;


  /***************************************************
   * We must set the clock selection bit before we
   * exit reset.
   **************************************************/
  // TODO A or B clock?

  /***************************************************
   * Now put the port out of reset.
   **************************************************/
  v &= ~1;
  _dev.epl_port_ctrl[port] = v;

  /***************************************************
   * After exiting reset, the datapath initialization
   * bit should always be cleared, since we only need to
   * initialize the data path once per switch reset.
   **************************************************/
//  v &= ~8;
//  _dev.epl_port_ctrl[port] = v;
}

void FmBaliMgr::configurePort(FmPort fmPort,
			       cm::fm::PortSpeed speed,
			       unsigned rxFlip,
			       unsigned txFlip,
			       unsigned drive,
                               unsigned trunk)
{
  FmConfig& config = _config;

  unsigned port = unsigned(fmPort)-0; // EPL_PORT_OFFSET=0

  unsigned v;
  const unsigned pmask = 1<<(port+0); // EPL_PORT_OFFSET=0
  int lb     = config.portCfg.portsLoopback & (1<<port);

  unsigned ports312MHz = config.portCfg.ports10Gb | config.portCfg.ports2_5Gb;
  if (speed == cm::fm::X2_5Gb || speed == cm::fm::X10Gb)
    ports312MHz |= pmask;
  else
    ports312MHz &= ~pmask;

  // skip this while debugging.
  // FmBaliMgr::reset() brings all EPL ports out of reset earlier.
  // NOTE!  epl_port_ctrl bit 0 (reset) is active low!
  //        setting bit 0 puts the port OUT of reset.

//  _dev.epl_phy[port].disable();

  _dev.epl_port_ctrl[port] = 0;

  if (ports312MHz & pmask)
    v &= ~0x4;
  else
    v |= 0x4;
  _dev.epl_port_ctrl[port]=v;

  v &= ~0x8;
  _dev.epl_port_ctrl[port]=v;

  v |= 1;
  _dev.epl_port_ctrl[port]=v;

  config.portCfg.ports10Mb  &= ~pmask;
  config.portCfg.ports100Mb &= ~pmask;
  config.portCfg.ports1Gb   &= ~pmask;
  config.portCfg.ports2_5Gb &= ~pmask;
  config.portCfg.ports4Gb   &= ~pmask;
  config.portCfg.ports10Gb  &= ~pmask;

  if (rxFlip) config.portCfg.portsRxFlip |= pmask;
  if (txFlip) config.portCfg.portsTxFlip |= pmask;

  /**
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
  **/

  Bali::FmEplRegs* p = &_dev.epl_phy[port];           // EPL_PORT_OFFSET=0
  switch(speed) {
  case cm::fm::X10Gb:
    config.portCfg.ports10Gb |= pmask;
    p->setup10Gb (lb, rxFlip, txFlip, 0);
    break;
  case cm::fm::X4Gb:
    config.portCfg.ports4Gb |= pmask;
    p->setup4Gb  (lb, rxFlip, txFlip);
    break;
  case cm::fm::X2_5Gb:
    config.portCfg.ports2_5Gb |= pmask;
    p->setup2_5Gb(lb, rxFlip, txFlip);
    break;
  case cm::fm::X1Gb:
    config.portCfg.ports1Gb |= pmask;
    p->setup1Gb  (lb, rxFlip, txFlip);
    break;
  case cm::fm::X100Mb:
    config.portCfg.ports100Mb |= pmask;
    p->setup100Mb(lb, rxFlip, txFlip);
    break;
  case cm::fm::X10Mb:
    config.portCfg.ports10Mb |= pmask;
    p->setup10Mb (lb, rxFlip, txFlip);
    break;
  case cm::fm::None:
  default:
    p->disable   ();
    break;
  }

  setSAFMatrix();

  /**
  FmBali::enableInterrupts(_dev.id());
  **/
}

void FmBaliMgr::configureLCI()
{
  //
  //  Setup the CPU interface
  //
  unsigned v = _dev.lci_cfg;
  v |= FmBali::Lci_Cfg::BigEndian;  //  Setup the CPU as big-endian
  _dev.lci_cfg = v;
}

/**
void FmBaliMgr::configureTrigger(unsigned id, const FmTriggerConfig& cfg)
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

unsigned FmBaliMgr::unusedTriggers() const
{
  return _unusedTriggers;
}

unsigned FmBaliMgr::lowestPriorityUnusedTrigger() const
{
  for(int i=31; i>=0; i--)
    if (_unusedTriggers&(1<<i))
      return i;
  return 63;
}

unsigned FmBaliMgr::highestPriorityUnusedTrigger() const
{
  for(int i=0; i<32; i++)
    if (_unusedTriggers&(1<<i))
      return i;
  return 63;
}

void FmBaliMgr::changePortMulticastGroupMembership(FmPortMask fmPorts,
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

void FmBaliMgr::trapIGMP(bool enable)
{
  if (enable)  _dev.sys_cfg_1 |=  FmBali::Sys_Cfg_1::TrapIGMP;
  else         _dev.sys_cfg_1 &= ~FmBali::Sys_Cfg_1::TrapIGMP;
}
**/

unsigned FmBaliMgr::id() const { return _dev.id(); }

FmBali& FmBaliMgr::dev() { return _dev; }

/**
void FmBaliMgr::_logFifo(unsigned* fifo,unsigned n) const
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

void FmBaliMgr::hop(FmPortMask mask, bool allow)
{
  unsigned forward = FmPortMask::allPorts();
  if (!allow) forward ^= mask;
  for(unsigned i=1; i<=FmPort::MAX_PORT; i++) {
    if (mask.contains(FmPort(i)))
      _dev.port_cfg_2[i] = _config.swCfg.portfwd_table.forward[i-1] & forward;
  }
}
**/

#if 0
static int _find_trunk_port(unsigned v, FmPort port)
{
  for(int k=0; k<6; k++, v>>=5)
    if ((v&0x1f)==port) return k;
  return -1;
}
#endif

/**
void FmBaliMgr::_trunk_set   (FmPortMask m, unsigned trunk)
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

void FmBaliMgr::_trunk_add   (FmPort port, unsigned trunk)
{
  _config.swCfg.trunk[trunk] |= FmPortMask(1<<port);
  _trunk_set(_config.swCfg.trunk[trunk],trunk);
}

void FmBaliMgr::_trunk_remove(FmPort port, unsigned trunk)
{
  _dev.trunk_port_map[port] = 0;

  _config.swCfg.trunk[trunk] &= ~FmPortMask(1<<port);
  _trunk_set(_config.swCfg.trunk[trunk],trunk);

  _dev.port_cfg_2[port] = _config.swCfg.portfwd_table.forward[port-1];
}

**/
