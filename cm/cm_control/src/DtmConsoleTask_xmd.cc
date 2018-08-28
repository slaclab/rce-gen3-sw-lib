
#include "control/DtmConsoleTask.hh"

#include "net/Manager.hh"
#include "fm/FmConfig.hh"
#include "phy/Vsc8234.hh"

#include "control/ChrDisplay.hh"

#include "fm/SwmRequest.hh"
#include "fm/SwmReply.hh"
#include "control/SwmDebug.hh"

#include "fm/FmTahoe.hh"
#include "fm/FmTahoeMgr.hh"

#include "ffs/IdFile.hh"

#include "debug/Debug.hh"
#include "debug/Print.hh"

#include <stdio.h>

unsigned dtm_command;
unsigned dtm_arg;
unsigned dtm_arg2;

static unsigned display_cnt=0;
static CimNet::Manager* fm;

static void execute    (CimNet::Manager*);
static void dumpFm     ();
static void basePhyMgmt();
static void genPacket  ();
static void rwReg      ();
static void reboot     ();
static void cfgPorts   ();
static void dumpPort   ();
static void display    ();

DtmConsoleTask::DtmConsoleTask(cm::net::Manager* m) :
  fm(m),
  fmCfg(new FmConfig)
{
}


void DtmConsoleTask::run()
{
  dtm_command = 0;

  while(1) {
    _execute(this->fm);
    dtm_command = 0;
  }
}

DtmConsoleTask::~DtmConsoleTask() {}

void DtmConsoleTask::_execute(cm::net::Manager* mgr)
{
    fm = mgr;

    switch(dtm_command) {
    case 1:
      { service::debug::clearMessages(); break; }
    case 2:
      { ChrDisplay::instance()->writeStr("TEST",ChrDisplay::Rotate90); break; }
    case 3:
      { cm::ffs::IdFile* idf = new cm::ffs::IdFile;
        printf("Read Board ID %s %s %s\n",
               idf->name(), idf->version(), idf->serialNumber());
        break; }
    case 4:
      { dumpFm(); break; }
    case 5:
      { basePhyMgmt(); break; }
    case 6:
      { genPacket(); break; }
    case 7:
      { rwReg(); break; }
    case 8:
      { cfgPorts(); break; }
    case 9:
      { reboot(); break; }
    case 10:
      { dumpPort(); break; }
    default:
      { display();
        timespec tv; tv.tv_sec=1; nanosleep(&tv,0); break; }
    }

}

#include "control/AbsConsole.hh"
class MyConsole : public AbsConsole {
public:
  MyConsole() {}
  ~MyConsole() {}
public:
  void printv(const char* msgformat, ...) {
    va_list va;
    va_start(va,msgformat);
    vprintf(msgformat,va);
    va_end(va);
  }
  void reserve(unsigned) {}
};

void dumpFm()
{
  printf("dumpFm begin\n");
  if (!fm) {
    printf("fm = 0\n");
    return;
  }

  MyConsole     o;
  FmTahoe*      t     = &fm->deviceMgr(0).dev();
  SwmRequest*   req;
  char*         reply = new char[0x80000];
  unsigned      reply_len;
  memset(reply,0,0x80000);

  printf("AllPortEplRequest\n");
  fm->process(*(req = new SwmAllPortEplRequest( t )), reply, reply_len);
  delete req;
  if (reply_len) {
    const SwmAllPortEplReply& rrep = *(const SwmAllPortEplReply*)reply;
    for(unsigned j=0; j<24; j++) {
      o.printv("  Port 0x%x\n",j );
      o << rrep.result()[j];
    }
  }

  printf("AllPortMibRequest\n");
  fm->process(*(req = new SwmAllPortMibRequest( t )), reply, reply_len);
  delete req;
  if (reply_len) {
      const SwmAllPortMibReply& rrep = *(const SwmAllPortMibReply*)reply;
      for(unsigned j=0; j<25; j++) {
	o.printv("  Port 0x%x\n",j );
	o << rrep.result().port(j);
      }
  }

  fm->process(*(req = new SwmGlobalMibRequest( t )), reply, reply_len);
  delete req;
  if (reply_len) {
    const SwmGlobalMibReply& rrep = *(const SwmGlobalMibReply*)reply;
    o.printv("== MIB Global (0x%x) ==\n", rrep.reg());
    o << rrep.result();
  }

  fm->process(*(req = new SwmStreamStatusRequest( t )), reply, reply_len);
  delete req;
  if (reply_len) {
    const SwmStreamStatusReply& rrep = *(const SwmStreamStatusReply*)reply;
    o.printv("== Stream Status ==\n");
    o << rrep.result();
  }
#if 0
  fm->process(*(req = new SwmMacTableReadRequest( t )), reply, reply_len);
  delete req;
  if (reply_len) {
    const SwmMacTableReadReply& rrep = *(const SwmMacTableReadReply*)reply;
    o.printv("== MAC Table BEG ==\n");
    o << rrep.result();
    o.printv("== MAC Table END ==\n");
  }
#endif
  delete[] reply;

  printf("dumpFm end\n");
}

void basePhyMgmt()
{
  printf("basePhyMgmt begin\n");
  Vsc8234* phy = new (0)Vsc8234;
  phy->initialize();

  unsigned links=0;
  for(unsigned k=0; k<Vsc8234::NPORTS; k++) {
    static const unsigned reg[] = { 0,1,2,3,9,10,30 };
    for(unsigned j=0; j<sizeof(reg)/sizeof(unsigned); j++) {
      unsigned v = phy->dev[k].r[reg[j]];
      printf("dev[%d].r[%d] = %04x\n", k,reg[j],v);
    }
    if (phy->dev[k].linkStatus())
      links |= 1<<k;
  }
  printf("links = %x\n",links);
  printf("basePhyMgmt end\n");
}

void genPacket()
{
  printf("genPacket begin\n");

  ether_addr_t src;
  src.ether_addr_octet[0] = 0x80;
  src.ether_addr_octet[1] = 0x00;
  src.ether_addr_octet[2] = 0x56;
  src.ether_addr_octet[3] = 0x00;
  src.ether_addr_octet[4] = 0x40;
  src.ether_addr_octet[5] = 0x23;

  ether_addr_t dst;
  dst.ether_addr_octet[0] = 0xff;
  dst.ether_addr_octet[1] = 0xff;
  dst.ether_addr_octet[2] = 0xff;
  dst.ether_addr_octet[3] = 0xff;
  dst.ether_addr_octet[4] = 0xff;
  dst.ether_addr_octet[5] = 0xff;

  unsigned size = 1024;

  char* buf = new char[size+12];
  char* b = buf;
  memcpy(b,&dst,6); b += 6;
  memcpy(b,&src,6); b += 6;
  for(unsigned k=0; k<size; k++)
    *b++ = k&0xff;

  unsigned* ub = (unsigned*)buf;
  unsigned* ue = ub + (unsigned(b-buf)>>2);
  SwmPacket pkt(0);
  while( ub < ue )
    pkt.append(*ub++);

  fm->deviceMgr(0).dev().txPacket(pkt.data(), pkt.bytes(), FmPortMask(pkt.dst()));

  printf("genPacket end\n");
}

void rwReg()
{
  printf("rwReg begin\n");

  FmMReg& reg = *(FmMReg*)(dtm_arg<<2);
  unsigned v0 = reg;

  reg = dtm_arg2;
  unsigned v1 = reg;

  reg = v0;
  unsigned v2 = reg;

  printf("reg(%x:%x) %x : %x : %x\n",dtm_arg,dtm_arg2,v0,v1,v2);

  printf("rwReg end\n");
}

void cfgPorts()
{
  printf("cfgPorts begin\n");

  if (!fm) {
    printf("No cm::net::Manager\n");
  }
  else {
    MyConsole     o;
    FmTahoe*      t     = &fm->deviceMgr(0).dev();
    SwmRequest*   req;
    char*         reply = new char[0x80000];
    unsigned      reply_len;

    {
      unsigned kpt = dtm_arg&0x1f;
      if (kpt<1 || kpt>24) kpt=1;

      req = new SwmEplReadRequest(&t->epl_phy[(kpt-1)&0x1f]);

      { SwmEplReadRequest* rr = static_cast<SwmEplReadRequest*>(req);
        printf("t %p  epl %p  reg %p\n",
               t,&t->epl_phy[(kpt-1)&0x1f],rr->reg()); }

      printf("Port %d : before\n",kpt);

      fm->process(*req, reply, reply_len);
      if (reply_len) {
        const SwmEplReadReply& rrep = *(const SwmEplReadReply*)reply;
        o.printv("== EPL Port (0x%x) ==\n", rrep.reg());
        o << rrep.result();
      }

      cm::fm::PortSpeed speed = cm::fm::None;
      switch(dtm_arg>>8) {
      case 0 : speed = cm::fm::X1Gb; break;
      case 1 : speed = cm::fm::X2_5Gb; break;
      case 2 : speed = cm::fm::X10Gb; break;
      default: speed = cm::fm::None; break;
      }
      fm->deviceMgr(0).configurePort(FmPort(kpt), speed, 0, 0, 0);

      printf("Port %d : after\n",kpt);

      fm->process(*req, reply, reply_len);
      if (reply_len) {
        const SwmEplReadReply& rrep = *(const SwmEplReadReply*)reply;
        o.printv("== EPL Port (0x%x) ==\n", rrep.reg());
        o << rrep.result();
      }

      delete req;
    }
    delete[] reply;
  }

  printf("cfgPorts end\n");
}

void reboot()
{
  printf("reboot begin\n");

  FmTahoe& _dev = fm->deviceMgr(0).dev();
  FmConfig& config = *new FmConfig;

  config.print();

  timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 20000000; // (20 msec should be sufficient)

  volatile unsigned v;

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
      service::debug::printv("FmTahoeMgr boot_status poll1 timeout (%x)",v);
      return;
    }
  }

  //  Begin boot state machine
  //  ( Enable DFT/JTAG with 0xd05 )
  //  Errata item #10 "Contention between 1Gb Mode and LED Interface
  //    Port Status
  //   _dev.chip_mode = 0xd04;
  _dev.chip_mode = 0xd00;

  service::debug::printv("FmTahoeMgr(%d) waiting for boot complete",_dev.id());

  //  Poll until boot is complete
  { unsigned k=20480;
    do {
      v = _dev.boot_status;
    } while( v && --k );
    if (!k) {
      service::debug::printv("FmTahoeMgr boot status didn't clear in 20480 reads");
      return;
    }
    else
      service::debug::printv("FmTahoeMgr boot status cleared in %d reads",20480-k);
  }

  //  Read vital product data register
  {
    v = _dev.vpd;
    if ( (v & 0x0fffffff) != 0xae1842b )
      service::debug::printv("Read incorrect product code %08x (%08x)",v,0xae1842b);
  }

  //  Ref design writes and reads from shadow fusebox here as a test

  //  Program frame handler clock for 300MHz
  //  (some contradiction in the description of bit 0 for this register)
  //  pllout<<15 | m << 4 | n << 11 | p << 2
  //  p=0; m=1b; n=3  (297 MHz)
  _dev.pll_fh_ctrl = 0x19b0;

  //   ts.tv_sec = 3;
  ts.tv_sec = 1;
  ts.tv_nsec = 0;
  nanosleep(&ts, 0);

  //  Poll until frame handler pll locks
  { unsigned k = 10240;
    do {
      v = _dev.pll_fh_stat;
    } while ( v==0 && --k );
    if (!k) {
      service::debug::printv("FmTahoeMgr timeout waiting for FH_PLL to lock");
      return;
    }
    else
      service::debug::printv("FmTahoeMgr fh pll locked");
  }

  //  Release pll_fh
  //   service::debug::printv("FmTahoeMgr releasing FH reset");
  _dev.soft_reset = 0x0;

  if ( (v = _dev.sys_cfg_1) != 0x4ff) {
    service::debug::printv("FmTahoeMgr fail: fm_sys_cfg_1 = 0x%x != 0x4ff",v);
    return;
  }

  if ( (v = _dev.port_reset) != 0x1fffffe ) {
    service::debug::printv("FmTahoeMgr fail: port reset = 0x%x != 0x1fffffe",v);
    return;
  }

  const unsigned _testPort = 5;
  FmEplRegs& p1 = _dev.epl_phy[_testPort];
  if ( (v = p1.pcs_cfg_1) != 0 ) {
    service::debug::printv("FmTahoeMgr fail: ports in reset but EPL reg reads= 0x%x",v);
    return;
  }

  p1.pcs_cfg_1 = 0xffffffff;
  if ( (v = p1.pcs_cfg_1) != 0 ) {
    service::debug::printv("FmTahoeMgr fail: ports in reset but EPL reg allowed write");
    return;
  }

  const unsigned ports312MHz = config.portCfg.ports10Gb | config.portCfg.ports2_5Gb;
  const unsigned _defaultPorts =
    0x1 |
    config.portCfg.ports10Gb  |
    config.portCfg.ports4Gb   |
    config.portCfg.ports2_5Gb |
    config.portCfg.ports1Gb   |
    config.portCfg.ports100Mb |
    config.portCfg.ports10Mb  ;

  v = 0x1fffffe & ~ports312MHz;
  //   service::debug::printv("Setting clock selection 0x%x",v);
  _dev.port_clk_sel = v;

  //   service::debug::printv("Bringing all EPLs out of reset");
  _dev.port_reset = 0x1fffffe & ~_defaultPorts;
  //   _dev.port_reset = 0;

  for(unsigned k=1; k<=24; k++) {
    _dev.epl_phy[k-1].serdes_ctrl_1 = 0x88884444;
    const unsigned pmask = (1<<k);
    int lb     = config.portCfg.portsLoopback & (1<<k);
    int rxFlip = config.portCfg.portsRxFlip   & (1<<k);
    int txFlip = config.portCfg.portsTxFlip   & (1<<k);
    if (config.portCfg.ports10Gb & pmask)
      _dev.epl_phy[k-1].setup10Gb(lb, rxFlip, txFlip);
    else if (config.portCfg.ports4Gb & pmask)
      _dev.epl_phy[k-1].setup4Gb(lb, rxFlip, txFlip);
    else if (config.portCfg.ports2_5Gb & pmask)
      _dev.epl_phy[k-1].setup2_5Gb(lb, rxFlip, txFlip);
    else if (config.portCfg.ports1Gb & pmask)
      _dev.epl_phy[k-1].setup1Gb(lb, rxFlip, txFlip);
    else if (config.portCfg.ports100Mb & pmask)
      _dev.epl_phy[k-1].setup100Mb(lb, rxFlip, txFlip);
    else if (config.portCfg.ports10Mb & pmask)
      _dev.epl_phy[k-1].setup10Mb(lb, rxFlip, txFlip);
    else
      _dev.epl_phy[k-1].disable();
  }

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

  //  Setup forwarding tables
  //   service::debug::printv("FmTahoeMgr writing FID entries");
  for(unsigned k=0; k<FmSwitchConfig::Entries; k++)
    _dev.fid_table[k] = config.swCfg.fid_table[k];

  //   service::debug::printv("FmTahoeMgr writing VID entries");
  for(unsigned k=0; k<FmSwitchConfig::Entries; k++)
    _dev.vid_table[k] = config.swCfg.vid_table[k];

  // Turn off learning
//  for(unsigned k=0; k<24; k++) {
//    v = _dev.port_cfg_1[k];
//    _dev.port_cfg_1[k] = v &~ 0x100000;
//  }

  //  MAC table entry interrupts
  const unsigned mask = ( FmTahoe::Frame_Ctrl_Ip::Entry_Aged |
                          FmTahoe::Frame_Ctrl_Ip::Entry_Learned |
                          FmTahoe::Frame_Ctrl_Ip::Bin_Full |
                          FmTahoe::Frame_Ctrl_Ip::FIFO_Full );

  _dev.frame_ctrl_im = _dev.frame_ctrl_im & ~mask;
  //   _dev.frame_ctrl_im = 0;
  _dev.mgr_im = 0;
  _dev.perr_im = 0;
  _dev.lci_im = FmTahoe::Lci_Ip::RxEOT | FmTahoe::Lci_Ip::RxRdy;
  _dev.port_vlan_im_1 = 0;
  _dev.port_vlan_im_2 = 0;
  _dev.port_mac_sec_im = 0;
  _dev.trigger_im = 0;

  //  Clear interrupt status registers
  v = _dev.global_epl_int_detect;
  v = _dev.lci_ip;
  v = _dev.interrupt_detect;

  //
  //  Setup the CPU interface
  //
  v = _dev.lci_cfg;
  v |= FmTahoe::Lci_Cfg::BigEndian;  //  Setup the CPU as big-endian
  v |= FmTahoe::Lci_Cfg::TxCompCRC;  //  Compute the CRC for tx packets
  v |= FmTahoe::Lci_Cfg::RxEnable;   //  Enable the receipt of packets
  _dev.lci_cfg = v;

  _dev.sys_cfg_7          = 0x7530;   // age entries once/2.4ms, whole table/40s
  _dev.global_pause_wm[0] = 0xffffffff;
  _dev.rx_pause_wm[0]     = 0xffffffff;  // no pause for CPU
  //  _dev.queue_cfg_2[0]     = 0x3ff;       // large private queue for CPU
  _dev.queue_cfg_5        = 1000;
  for(int i=0; i<=24; i++)
    _dev.queue_cfg_1[i] = 0x03ff02ff;  // allow hog ports   { [9:0] = 0xff (works), 0x3ff (broken) }

  _dev.frame_time_out     = 0x989680;  // 5 minutes

  //  The default is to send broadcasts and control protocol addresses to the CPU
  //  _dev.sys_cfg_1 |= 0x40;  // send bcasts to cpu
  //  _dev.sys_cfg_1 |= (1<<14);  // do not flood multicasts

  //  setSAFMatrix();

  //
  //  Configure the triggers
  //
  //  for(unsigned k=0; k<16; k++)
  //    configureTrigger(k,cfg.trigCfg[k]);

  *fmCfg = config;
  delete& config;

  printf("reboot end\n");
}

void dumpPort()
{
  printf("dumpPort begin\n");

  char*         reply = new char[0x80000];
  memset(reply,0,0x80000);
  unsigned      reply_len;
  SwmEplReadRequest* req = new SwmEplReadRequest(&fm->deviceMgr(0).dev().epl_phy[dtm_arg&0x1f]);
  fm->process(*req,reply,reply_len);

  printf("reply_len %d\n",reply_len);
  if (reply_len) {
    MyConsole  o;
    const SwmEplReadReply& rrep = *(const SwmEplReadReply*)reply;
    o.printv("== EPL Port (0x%x:0x%x) ==\n", req->reg(),rrep.reg());
    o << rrep.result();
  }

  delete req;
  delete[] reply;

  printf("dumpPort end\n");
}

void display()
{
  if (!fm) {
    return;
  }

  display_cnt++;
  unsigned pup = fm->deviceMgr(0).portsUp();

  char buff[8];
  sprintf(buff,"%01X%c%c%c",
          display_cnt&0xf,
          pup&(1<<19) ? 'U':'D',
          pup&(1<< 3) ? 'U':'D',
          pup&(1<< 8) ? 'U':'D');

  CimDisplay::instance()->writeStr(buff,CimDisplay::Rotate90);
}


void DtmConsoleTask::_replAll    (AbsConsole&) {}
void DtmConsoleTask::_dumpFm     (AbsConsole&) {}
void DtmConsoleTask::_basePhyMgmt(AbsConsole&) {}
void DtmConsoleTask::_genPacket  (AbsConsole&,const char*) {}
void DtmConsoleTask::_rwReg      (AbsConsole&,unsigned,unsigned) {}
void DtmConsoleTask::_reboot     () {}
void DtmConsoleTask::_porttest   () {}
void DtmConsoleTask::_dumpSerdes (const FmPortMask&) {}
bool DtmConsoleTask::handle(RceConsole::RemoteConsole& o,
                            const char* cmd,
                            const char* args) { return false; }
void DtmConsoleTask::help  (AbsConsole& o) {}

#include <rtems/shellconfig.h>
