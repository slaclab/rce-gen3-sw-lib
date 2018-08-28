
#include "fm/FmConfig.hh"
#include "cm_svc/XML.hh"

#include <stdio.h>
#include <string.h>

using namespace cm;

FmConfig::FmConfig() {}

FmConfig::FmConfig(const FmConfig& c) :
  portCfg    (c.portCfg    ),
  hostCfg    (c.hostCfg    ),
  swCfg      (c.swCfg      ),
  protocolCfg(c.protocolCfg)
{
  for(unsigned k=0; k<NumberOfTriggers; k++)
    trigCfg[k] = c.trigCfg[k];
}

FmProtocolConfig::FmProtocolConfig() :
  igmpVsn (NoIGMP),
  garpImpl(NoGARP),
  stpImpl (NoSTP)
{}

FmProtocolConfig::FmProtocolConfig(const FmProtocolConfig& c) :
  igmpVsn (c.igmpVsn),
  garpImpl(c.garpImpl),
  stpImpl (c.stpImpl)
{}

void FmProtocolConfig::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if      (tag.name == "igmpVsn")
      igmpVsn = IGMPVsn(XML::IO::extract_i(p));
    else if (tag.name == "garpImpl")
      garpImpl = GARPImpl(XML::IO::extract_i(p));
    else if (tag.name == "stpImpl")
      stpImpl  = STPImpl(XML::IO::extract_i(p));
    else if (tag.name == "mcastFlood")
      mcastFlood = FmPortMask(XML::IO::extract_i(p));
  XML_iterate_close(FmProtocolConfig,tag);
}

void FmProtocolConfig::save(char*& p) const
{
  XML_insert( p, "IGMPVsn", "igmpVsn", XML::IO::insert(p,unsigned(igmpVsn)));
  XML_insert( p, "GARPImpl", "garpImpl", XML::IO::insert(p,unsigned(garpImpl)));
  XML_insert( p, "STPImpl", "stpImpl", XML::IO::insert(p,unsigned(stpImpl)));
  XML_insert( p, "FmPortMask", "mcastFlood", XML::IO::insert(p,unsigned(mcastFlood)));
}

FmTriggerConfig::FmTriggerConfig() :
  mirror_port(0),
  sourcePortMask(0),
  destinationPortMask(0),
  enable(0)
{}

FmTriggerConfig::FmTriggerConfig(const FmTriggerConfig& c) :
  mac_entry_trigger  (c.mac_entry_trigger),
  new_priority       (c.new_priority),
  mirror_port        (c.mirror_port),
  action             (c.action),
  xcast              (c.xcast),
  vlanMatch          (c.vlanMatch),
  sourceMacMatch     (c.sourceMacMatch),
  destinationMacMatch(c.destinationMacMatch),
  priorityMask       (c.priorityMask),
  sourcePortMask     (c.sourcePortMask),
  destinationPortMask(c.destinationPortMask),
  enable             (c.enable)
{}

void FmTriggerConfig::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if      (tag.name == "mac_entry_trigger")
      mac_entry_trigger = XML::IO::extract_i(p);
    else if (tag.name == "new_priority")
      new_priority = XML::IO::extract_i(p);
    else if (tag.name == "mirror_port")
      mirror_port  = FmPort(XML::IO::extract_i(p));
    else if (tag.name == "action")
      action = Action(XML::IO::extract_i(p));
    else if (tag.name == "xcast")
      xcast = XCast(XML::IO::extract_i(p));
    else if (tag.name == "vlanMatch")
      vlanMatch = XML::IO::extract_i(p);
    else if (tag.name == "sourceMacMatch")
      sourceMacMatch = MACMatch(XML::IO::extract_i(p));
    else if (tag.name == "destinationMacMatch")
      destinationMacMatch = MACMatch(XML::IO::extract_i(p));
    else if (tag.name == "priorityMask")
      priorityMask = XML::IO::extract_i(p);
    else if (tag.name == "sourcePortMask")
      sourcePortMask = FmPortMask(XML::IO::extract_i(p));
    else if (tag.name == "destinationPortMask")
      destinationPortMask = FmPortMask(XML::IO::extract_i(p));
    else if (tag.name == "enable")
      enable = XML::IO::extract_i(p);
  XML_iterate_close(FmProtocolConfig,tag);
}

void FmTriggerConfig::save(char*& p) const
{
  XML_insert(p,"unsigned"  , "mac_entry_trigger", XML::IO::insert(p,mac_entry_trigger));
  XML_insert(p,"unsigned"  , "new_priority"     , XML::IO::insert(p,new_priority));
  XML_insert(p,"FmPort"    , "mirror_port"      , XML::IO::insert(p,unsigned(mirror_port)));
  XML_insert(p,"Action"    , "action"           , XML::IO::insert(p,action));
  XML_insert(p,"XCast"     , "xcast"            , XML::IO::insert(p,xcast));
  XML_insert(p,"unsigned"  , "vlanMatch"        , XML::IO::insert(p,vlanMatch));
  XML_insert(p,"MACMatch"  , "sourceMacMatch"   , XML::IO::insert(p,sourceMacMatch));
  XML_insert(p,"MACMatch"  , "destinationMacMatch", XML::IO::insert(p,destinationMacMatch));
  XML_insert(p,"unsigned"  , "priorityMask"     , XML::IO::insert(p,priorityMask));
  XML_insert(p,"FmPortMask", "sourcePortMask"   , XML::IO::insert(p,unsigned(sourcePortMask)));
  XML_insert(p,"FmPortMask", "destinationPortMask", XML::IO::insert(p,unsigned(destinationPortMask)));
  XML_insert(p,"unsigned"  , "enable"           , XML::IO::insert(p,enable));
}

FmSwitchConfig::FmSwitchConfig()
{
  FmPortMask empty(0);
  FmPortMask full (FmPortMask::phyPorts());
  fid_table[0] = SwmFidTableEntry (empty,empty,full);
  for(unsigned k=1; k<Entries; k++)
    fid_table[k] = SwmFidTableEntry (empty,empty,full);

  for(unsigned k=0; k<Entries; k++)
    vid_table[k] = SwmVlanTableEntry(0,0,0,0,0);
  vid_table[1] = SwmVlanTableEntry(-1UL,0,-1UL,-1UL,0);

  for(unsigned k=0; k<FmPort::MAX_PORT; k++)
    portfwd_table.forward[k] = FmPortMask::allPorts();

  mac_table.clear();
}

FmSwitchConfig::FmSwitchConfig(const FmSwitchConfig& c) :
  mac_table(c.mac_table),
  portfwd_table(c.portfwd_table)
{
  for(unsigned k=0; k<Entries; k++)
    fid_table[k] = c.fid_table[k];

  for(unsigned k=0; k<Entries; k++)
    vid_table[k] = c.vid_table[k];
}

void FmSwitchConfig::load(const char*& p)
{
  unsigned nt=0;
  XML_iterate_open(p,tag)
    if      (tag.name == "trunk")
      trunk[nt++] = FmPortMask(XML::IO::extract_i(p));
  XML_iterate_close(FmSwitchConfig,tag);
}

void FmSwitchConfig::save(char*& p) const
{
  for(unsigned k=0; k<Trunks; k++) {
    XML_insert(p,"FmPortMask", "trunk", XML::IO::insert(p,unsigned(trunk[k])));
  }
}

FmHostIfConfig::FmHostIfConfig() :
  ip_addr   (0),
  ip_netmask(0)
{
  if_name[0] = '\0';
  memset(mac_addr,0,sizeof(mac_addr));
}

FmHostIfConfig::FmHostIfConfig(const FmHostIfConfig& c) :
  ip_addr   (c.ip_addr),
  ip_netmask(c.ip_netmask)
{
  memcpy(reinterpret_cast<char*>(if_name) ,
	 reinterpret_cast<const char*>(c.if_name) ,8);
  memcpy(reinterpret_cast<char*>(mac_addr),
	 reinterpret_cast<const char*>(c.mac_addr),6);
}

void FmHostIfConfig::load(const char*& p)
{
}

void FmHostIfConfig::save(char*& p) const
{
}

FmPortConfig::FmPortConfig() :
  ports10Mb    (0),
  ports100Mb   (0),
  ports1Gb     (0),
  ports2_5Gb   (0),
  ports4Gb     (0),
  ports10Gb    (0),
  portsRxFlip  (0),
  portsTxFlip  (0),
  portsLoopback(0)
{}

FmPortConfig::FmPortConfig(const FmPortConfig& c) :
  ports10Mb    (c.ports10Mb    ),
  ports100Mb   (c.ports100Mb   ),
  ports1Gb     (c.ports1Gb     ),
  ports2_5Gb   (c.ports2_5Gb   ),
  ports4Gb     (c.ports4Gb     ),
  ports10Gb    (c.ports10Gb    ),
  portsRxFlip  (c.portsRxFlip  ),
  portsTxFlip  (c.portsTxFlip  ),
  portsLoopback(c.portsLoopback)
{
}

void FmPortConfig::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if      (tag.name == "ports10Mb")
      ports10Mb = FmPortMask(XML::IO::extract_i(p));
    else if (tag.name == "ports100Mb")
      ports100Mb = FmPortMask(XML::IO::extract_i(p));
    else if (tag.name == "ports1Gb")
      ports1Gb  = FmPortMask(XML::IO::extract_i(p));
    else if (tag.name == "ports2_5Gb")
      ports2_5Gb = FmPortMask(XML::IO::extract_i(p));
    else if (tag.name == "ports4Gb")
      ports4Gb = FmPortMask(XML::IO::extract_i(p));
    else if (tag.name == "ports10Gb")
      ports10Gb = FmPortMask(XML::IO::extract_i(p));
    else if (tag.name == "portsRxFlip")
      portsRxFlip = FmPortMask(XML::IO::extract_i(p));
    else if (tag.name == "portsTxFlip")
      portsTxFlip = FmPortMask(XML::IO::extract_i(p));
    else if (tag.name == "portsLoopback")
      portsLoopback = FmPortMask(XML::IO::extract_i(p));
  XML_iterate_close(FmPortConfig,tag)
}

void FmPortConfig::save(char*& p) const
{
  XML_insert(p, "FmPortMask", "ports10Mb"    , XML::IO::insert(p,unsigned(ports10Mb)));
  XML_insert(p, "FmPortMask", "ports100Mb"   , XML::IO::insert(p,unsigned(ports100Mb)));
  XML_insert(p, "FmPortMask", "ports1Gb"     , XML::IO::insert(p,unsigned(ports1Gb)));
  XML_insert(p, "FmPortMask", "ports2_5Gb"   , XML::IO::insert(p,unsigned(ports2_5Gb)));
  XML_insert(p, "FmPortMask", "ports4Gb"     , XML::IO::insert(p,unsigned(ports4Gb)));
  XML_insert(p, "FmPortMask", "ports10Gb"    , XML::IO::insert(p,unsigned(ports10Gb)));
  XML_insert(p, "FmPortMask", "portsRxFlip"  , XML::IO::insert(p,unsigned(portsRxFlip)));
  XML_insert(p, "FmPortMask", "portsTxFlip"  , XML::IO::insert(p,unsigned(portsTxFlip)));
  XML_insert(p, "FmPortMask", "portsLoopback", XML::IO::insert(p,unsigned(portsLoopback)));
}
 
FmPortMask FmPortConfig::update( PortParameter p, FmPortMask fmPortMask )
{
  unsigned v(fmPortMask);
  //  Some port configurations should not be changed (autonegotiation capable)
  const unsigned UPDATE_MASK = ~0x1000000;

  FmPortMask r(0);
#define _update( q ) { r = q; q = FmPortMask((unsigned(q) & ~UPDATE_MASK) | (v & UPDATE_MASK)); }
  switch( p ) {
  case X_10Mb    : _update( ports10Mb     ); break;
  case X_100Mb   : _update( ports100Mb    ); break;
  case X_1Gb     : _update( ports1Gb      ); break;
  case X_2_5Gb   : _update( ports2_5Gb    ); break;
  case X_4Gb     : _update( ports4Gb      ); break;
  case X_10Gb    : _update( ports10Gb     ); break;
  case X_RxFlip  : _update( portsRxFlip   ); break;
  case X_TxFlip  : _update( portsTxFlip   ); break;
  case X_Loopback: _update( portsLoopback ); break;
  default        : break;
  }
  return r;
}

void FmPortConfig::clear(FmPortMask m)
{
  ports10Mb    &= ~m;
  ports100Mb   &= ~m;
  ports1Gb     &= ~m;
  ports2_5Gb   &= ~m;
  ports4Gb     &= ~m;
  ports10Gb    &= ~m;
  portsRxFlip  &= ~m;
  portsTxFlip  &= ~m;
  portsLoopback&= ~m;
}

void FmConfig::print() const
{
  const FmPortConfig& p = portCfg;
#define printMask( ttl, v ) { printf("%15s : %x\n", ttl, unsigned(p.v) ); }
  printMask(  "10Mb", ports10Mb);
  printMask( "100Mb", ports100Mb);
  printMask(   "1Gb", ports1Gb);
  printMask( "2.5Gb", ports2_5Gb);
  printMask(  "10Gb", ports10Gb);
  printMask("rxFlip", portsRxFlip);
  printMask("txFlip", portsTxFlip);
  printMask("loopbk", portsLoopback);

  const FmHostIfConfig& h = hostCfg;
  printf("%15s : %s\n","host name", h.if_name );
  unsigned char* a = (unsigned char*)&h.ip_addr;
  printf("%15s : %d.%d.%d.%d\n", "boot ip_addr",
	 int(a[0]),
	 int(a[1]),
	 int(a[2]),
	 int(a[3]) );
  a = (unsigned char*)&h.ip_netmask;
  printf("%15s : %d.%d.%d.%d\n", "boot ip_netm",
	 int(a[0]),
	 int(a[1]),
	 int(a[2]),
	 int(a[3]) );
  { char b[64];
    const unsigned char* c = h.mac_addr;
    sprintf(b,"%02x:%02x:%02x:%02x:%02x:%02x",c[0],c[1],c[2],c[3],c[4],c[5]);
    printf("%s\n",b);
  }

  //  for(unsigned k=0; k<SwmMacTable::Entries; k++) {
  for(unsigned k=0; k<10; k++) {
    const SwmMacTableEntry& e = swCfg.mac_table.entry[k];
    if (e.valid())
      e.print();
  }

  printf("Port Forwarding\n");
  for(unsigned k=0; k<FmPort::MAX_PORT; k++)
    printf("%d -> 0x%x\n",k+1,unsigned(swCfg.portfwd_table.forward[k]));
};

void FmConfig::load(const char*& p)
{
  unsigned trigger=0;
  XML_iterate_open(p,tag)
    if      (tag.element == "FmPortConfig")
      portCfg.load(p);
    else if (tag.element == "FmSwitchConfig")
      swCfg  .load(p);
    else if (tag.element == "FmTriggerConfig")
      trigCfg[trigger++].load(p);
    else if (tag.element == "FmProtocolConfig")
      protocolCfg.load(p);
  XML_iterate_close(FmConfig,tag);
}

void FmConfig::save(char*& p) const
{
  XML_insert(p, "FmPortConfig", "portCfg", portCfg.save(p));
  XML_insert(p, "FmSwitchConfig", "swCfg", swCfg.save(p));
  for(unsigned i=0; i<NumberOfTriggers; i++) {
    XML_insert(p, "FmTriggerConfig", "trigCfg", trigCfg[i].save(p));
  }
  XML_insert(p, "FmProtocolConfig", "protocolCfg", protocolCfg.save(p));
}

