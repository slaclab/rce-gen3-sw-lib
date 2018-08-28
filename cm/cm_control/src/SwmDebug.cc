
#include "cm_control/SwmDebug.hh"

#include "cm_control/AbsConsole.hh"

#include "fm/SwmPortMscCounters.hh"
#include "fm/SwmPortMibCounters.hh"
#include "fm/SwmGlobalMibCounters.hh"
#include "fm/SwmPacket.hh"
#include "fm/SwmMacTable.hh"
#include "fm/SwmVlanTableEntry.hh"
#include "fm/SwmFidTableEntry.hh"
#include "fm/SwmStreamStatus.hh"
#include "fm/SwmEplRegs.hh"
#include "fm/FmConfig.hh"

#include <stdio.h>

AbsConsole& operator<<(AbsConsole& o,const SwmPortMscCounters& c)
{
  o.printv("txPause = %x\n", c.txPause);
  o.printv("txFcsErrors = %x\n", c.txFcsErrors);
  o.printv("rxJabbers = %x\n", c.rxJabbers);
  return o;
}

static char* sprintf_ethaddr(char* buf,char* addr) {
  sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",
	  addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
  return buf;
}

AbsConsole& operator<<(AbsConsole& o,const SwmPacket& c)
{
  char buf[64],buf2[64];
  char* d = (char*)c.data();
  o.printv("srcPort %x, rxPri %x, vlanId %x, vlanAct %x, rxPad %x\n",
	   unsigned(c.srcPort()), c.rxPri(), c.vlanId(), c.vlanAct(), c.rxPad());
  if (c.uflow() || c.tailErr() || c.crcErr()) {
    o.printv("--");
    if (c.uflow  ()) o.printv("-uflow");
    if (c.tailErr()) o.printv("-tailErr");
    if (c.crcErr ()) o.printv("-crcErr");
    o.printv("---\n");
  }

  o.printv("%s > %s  len=%x\n",
	   sprintf_ethaddr(buf ,d),
	   sprintf_ethaddr(buf2,&d[6]),
	   c.len()<<2);
  const unsigned* s = c.data() + 3;
  const unsigned* end = s;
  end += (c.len() > 8) ? 8 : c.len()-3;
  while( s < end ) {
    o.printv(" %08x",*s++);
  }
  o.printv("\n");
  return o;
}

AbsConsole& operator<<(AbsConsole& o,const SwmMacTable& c)
{
  o.printv("%8.8s%20.20s%10.10s%10.10s%6.6s%6.6s%4.4s\n",
	   "Entry", "MAC Address", "Ports", "Lock", "Fid", "Trig", "P");
  for(unsigned j=0; j<SwmMacTable::Entries; j++) {
    const SwmMacTableEntry& e = c.entry[j];
    if (e.valid()) {
      o.printv("%8.8d",j);
      o << e;
      o.printv("\n");
    }
  }
  return o;
}

AbsConsole& operator<<(AbsConsole& o,const SwmMacTableEntry& c)
{
  if (c.valid()) {
    char buf[32];
    sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",
	    c.addr_octet(5),
	    c.addr_octet(4),
	    c.addr_octet(3),
	    c.addr_octet(2),
	    c.addr_octet(1),
	    c.addr_octet(0));
    o.printv("%20.20s",buf);
    sprintf(buf,"%08x",unsigned(c.port_mask()));
    o.printv("%10.10s",buf);
    if (c.locked())
      o.printv("%10.10s","Static ");
    else
      o.printv("%10.10s","Dynamic ");
    o.printv("%6.6x %6.6x %4.4x",
	     c.learning_group(), c.trigger(), c.parity());
  }
  return o;
}

AbsConsole& operator<<(AbsConsole& o,const SwmVlanTableEntry& c)
{
  unsigned m=0, t=0;
  for(unsigned j=0; j<=24; j++) {
    m |= c.portIsMember(j)  ? (1<<j) : 0;
    t |= c.portIsTagging(j) ? (1<<j) : 0;
  }
  char buf[64];
  sprintf(buf,"trigger/counter/reflect %x/%x/%x  members %06x  tagging %06x",
	  c.trigger(), c.counter(), c.reflect(), m, t);
  o.printv("%s\n",buf);
  return o;
}

AbsConsole& operator<<(AbsConsole& o,const SwmFidTableEntry& c)
{
  char buf[64];
  sprintf(buf,"listen/learn/forward: %02x/%02x/%02x",
	  unsigned(c.listening()),
	  unsigned(c.learning()),
	  unsigned(c.forwarding()));
  o.printv("%s\n",buf);
  return o;
}

static char _buff[64];

#define printReg( var ) { \
  if (c.var) \
    o.printv("%30.30s = 0x%x\n", # var, c.var); \
}

#define printRegs( var, len ) {         \
  for(unsigned j=0; j<len; j++) {              \
    if (c.var[j]) {                            \
      sprintf(_buff,"%s[%d] = ",# var,j);       \
      o.printv("%30.30s%x\n", _buff, c.var[j]); \
    }                                          \
  }                                            \
}

#define printStat( var ) { \
  if (c.var) \
    o.printv("%30.30s = 0x%llx\n", # var, (unsigned long long) c.var); \
}

#define printStats( var, len ) {	       \
    for(unsigned j=0; j<len; j++) {	       \
      if (c.var[j]) {			       \
	sprintf(_buff,"%s[%d]",# var,j);				\
	o.printv("%30.30s = 0x%llx\n", _buff, (unsigned long long)c.var[j]); \
      }									\
    }									\
  }

static unsigned _lanes(unsigned q)
{
  return
    ((q>>0)&1) |
    ((q>>2)&2) |
    ((q>>4)&4) |
    ((q>>6)&8) ;
}

AbsConsole& operator<<(AbsConsole& o,const SwmEplRegs& c)
{
  printReg(serdes_ctrl_1);
  printReg(serdes_ctrl_2);
  printReg(serdes_ctrl_3);
  printReg(serdes_test_mode);
  printReg(serdes_status);
  printReg(serdes_ip);
  unsigned v = c.serdes_ip & 0xfff;
  if (v&0x924)
    o.printv("%40.40s%x\n","disparity error  lanes ",_lanes(v>>2));
  if (v&0x492)
    o.printv("%40.40s%x\n","out-of-band char lanes ",_lanes(v>>1));
  if (v&0x249)
    o.printv("%40.40s%x\n","loss of signal   lanes ",_lanes(v>>0));
  printReg(serdes_im);
  printReg(serdes_bist_err_cnt);
  printReg(pcs_cfg_1);
  printReg(pcs_cfg_2);
  printReg(pcs_cfg_3);
  printReg(pcs_cfg_4);
  printReg(pcs_cfg_5);
  printReg(pcs_ip);
  printReg(pcs_im);
  printRegs(pacing_pri_wm,8);
  printReg(pacing_rate);
  printReg(pacing_status);
  printReg(mac_cfg_1);
  printReg(mac_cfg_2);
  printReg(mac_cfg_3);
  printReg(mac_cfg_4);
  printReg(mac_cfg_5);
  printReg(mac_cfg_6);
  printReg(tx_pri_map_1);
  printReg(tx_pri_map_2);
  printReg(mac_status);
  printReg(mac_ip);
  printReg(mac_im);
  printReg(stat_epl_error1);
  printReg(stat_tx_pause);
  printReg(stat_tx_crc);
  printReg(stat_epl_error2);
  printReg(stat_rx_jabber);
  printReg(epl_led_status);
  printReg(epl_int_detect);
  printReg(stat_tx_bytecount);

  return o;
}


AbsConsole& operator<<(AbsConsole& o,const SwmPortMibCounters& c)
{
  printStat(rxUcast);
  printStat(rxBcast);
  printStat(rxMcast);
  printStat(rxPause);
  printStat(rxFcsErrors);
  printStat(rxSymbolErrors);
  printStats(rxPri,8);
  printStat(txUnicast);
  printStat(txBroadcast);
  printStat(txMulticast);
  printStat(txTimeoutDrops);
  printStat(txErrorDrops);
  printStat(rxMinto63);
  printStat(rx64);
  printStat(rx65to127);
  printStat(rx128to255);
  printStat(rx256to511);
  printStat(rx512to1023);
  printStat(rx1024to1522);
  printStat(rx1523to2047);
  printStat(rx2048to4095);
  printStat(rx4096to8191);
  printStat(rx8192to10239);
  printStat(rx10240toMax);
  printStat(rxUndersized);
  printStat(rxOversized);
  printStat(rxFragments);
  printStat(rxGoodOctets);
  printStat(rxBadOctets);
  printStat(txMinto63);
  printStat(tx64);
  printStat(tx65to127);
  printStat(tx128to255);
  printStat(tx256to511);
  printStat(tx512to1023);
  printStat(tx1024to1522);
  printStat(tx1523to2047);
  printStat(tx2048to4095);
  printStat(tx4096to8191);
  printStat(tx8192to10239);
  printStat(tx10240toMax);
  printStat(fidForwarded);
  printStat(floodForwarded);
  printStat(stpDrops);
  printStat(reservedTraps);
  printStat(securityViolationDrops);
  printStat(vlanTagDrops);
  printStat(vlanIngressBVDrops);
  printStat(vlanEgressBVDrops);
  printStat(triggerRedirAndDrops);
  printStat(triggerMirrored);
  printStat(dlfDrops);
  printStat(broadcastDrops);
  printStat(cmrxDrops);
  printStats(rxOctetsPri,8);
  return o;
}

AbsConsole& operator<<(AbsConsole& o,const SwmGlobalMibCounters& c)
{
  printStat(cmGlobalLowDrops);
  printStat(cmGlobalHighDrops);
  printStat(cmGlobalPrivilegeDrops);
  printStats(cmTxDrops,25);
  printStats(trigCount,17);
  printStats(vlanUnicast,32);
  printStats(vlanXcast,32);
  printStats(vlanUnicastOctets,32);
  printStats(vlanXcastOctets,32);
  return o;
}

#define DUMPADDR(var,name) { \
  unsigned a[4]; \
  a[0] = (var>>24)&0xff; \
  a[1] = (var>>16)&0xff; \
  a[2] = (var>> 8)&0xff; \
  a[3] = (var>> 0)&0xff; \
  o.printv("%d.%d.%d.%d\n",a[0],a[1],a[2],a[3]); \
}

AbsConsole& operator<<(AbsConsole& o,const FmConfig& c)
{
  const FmPortConfig& p = c.portCfg;
#define printMask( ttl, v ) { o.printv("%15s : 0x%x\n", ttl, unsigned(p.v) ); }
  printMask(  "10Mb", ports10Mb);
  printMask( "100Mb", ports100Mb);
  printMask(   "1Gb", ports1Gb);
  printMask( "2.5Gb", ports2_5Gb);
  printMask(  "10Gb", ports10Gb);
  printMask("rxFlip", portsRxFlip);
  printMask("txFlip", portsTxFlip);
  printMask("loopbk", portsLoopback);

  const FmHostIfConfig& h = c.hostCfg;
  o.printv("%15s%s\n", "host name : ",  h.if_name );
  DUMPADDR(h.ip_addr   ,"ip addr : ");
  DUMPADDR(h.ip_netmask,"ip netm : ");
  { char b[64]; sprintf_ethaddr(b,(char*)h.mac_addr); o.printv("%s\n",b); }

  const FmSwitchConfig& s = c.swCfg;
  o.printv("Spanning Tree configuration (First 10 entries)\n");
  //  for(unsigned k=0; k<FmSwitchConfig::Entries; k++) {
  for(unsigned k=0; k<10; k++)
      o << s.fid_table[k];
  o.printv("VLAN configuration (First 10 entries)\n");
  //  for(unsigned k=0; k<FmSwitchConfig::Entries; k++)
  for(unsigned k=0; k<10; k++)
    o << s.vid_table[k];
  o.printv("MAC Routing configuration\n");
  o << s.mac_table;
  return o;
}


AbsConsole& operator<<(AbsConsole& o,const SwmStreamStatus& c)
{
  unsigned v = c._sharedStream;
  o.printv("%30.30s : 0x%x\n", "segs in stream memory", (v&0xfff));
  o.printv("%30.30s : 0x%x\n", "segs in shared memory", ((v>>16)&0xfff));
  char buf[32];
  for(unsigned k=0; k<=24; k++) {
    v = c._portStream[k];
    sprintf(buf,"  port %02x  rxq",k);
    o.printv("%30.30s : 0x%x\n", buf, (v&0xfff));
    sprintf(buf,"  port %02x  txq",k);
    o.printv("%30.30s : 0x%x\n", buf, ((v>>12)&0xfff));
  }
  return o;
}

AbsConsole& operator<<(AbsConsole& o,const SwmMibCounter& c)
{
  o.printv("0x%08x%08x",c.data[1],c.data[0]);
  return o;
}

