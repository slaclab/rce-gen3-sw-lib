#include "MACHandler.hh"

#include "fm/FmTahoeMgr.hh"
#include "fm/FmTahoe.hh"
#include "fm/FmMacTable.hh"
#include "fm/FmConfig.hh"

#include <stdio.h>

MACHandler::MACHandler(FmTahoeMgr& m,
                       unsigned    allow,
                       unsigned    deny) :
  _fm   (m),
  _allow(FmPortMask::allPorts()&~deny),
  _deny (deny )
{
  unsigned t = m.highestPriorityUnusedTrigger();
  if (t==FmTahoeMgr::NoUnusedTrigger)
    printf("Failed to allocate a trigger for Mesh MAC filtering\n");
  else {
    FmTriggerConfig c;
    c.mac_entry_trigger   = t;
    c.new_priority        = 0;
    c.mirror_port         = FmPort(0);
    c.action              = FmTriggerConfig::Discard;
    c.xcast               = FmTriggerConfig::Any;
    c.vlanMatch           = 0;
    c.sourceMacMatch      = FmTriggerConfig::Hit;
    c.destinationMacMatch = FmTriggerConfig::Either;
    c.priorityMask        = 0;
    c.sourcePortMask      = FmPortMask(deny);
    c.destinationPortMask = FmPortMask(0);
    c.enable              = 1;
    m.configureTrigger(t,c);
  }
  _trig = t;

  _entries.clear();
  for(unsigned v=0; v<FmMacTable::Entries; v++) 
    _setup_entry(v);
  
  //
  //  Disable learning on the deny ports to allow simultaneous
  //  learning on the allow ports (to get into the MAC FIFO)
  //
  {
    unsigned mask=deny;
    for(unsigned i=0; mask!=0; i++)
      if (mask&(1<<i)) {
        mask&=~(1<<i);
        m.dev().port_cfg_1[i] &= ~FmTahoe::Port_Cfg_1::EnableLearning;
      }
  }

  //
  //  Define a trigger to trap entries from the deny ports
  //  (for learning) [not implemented]
  //

  m.dev().frame_ctrl_im = ~(FmTahoe::Frame_Ctrl_Ip::Entry_Aged |
                            FmTahoe::Frame_Ctrl_Ip::Entry_Learned);
  m.registerHandler(FmTahoeMgr::FCTL,this);
}

MACHandler::~MACHandler()
{
}

void MACHandler::_setup_entry(unsigned v)
{
  FmMacTable& t = _fm.macTable();
  SwmMacTableEntry e(t.entry(v));
  unsigned m = unsigned(e.port_mask()) & _allow;

  if (!e.valid() || e.locked() || m==0)
    return;

  ether_addr_t src;
  for(unsigned j=0; j<6; j++)
    src.ether_addr_octet[j] = e.addr_octet(5-j);
  SwmMacTableEntry f(src,
                     FmPortMask(m),
                     e.learning_group(),
                     _trig,
                     1);

  //  t.updateEntry(v,f);
  t.deleteEntry(f);
  t.addEntry(f);

  _entries.insert(v);
}

void MACHandler::call(unsigned p)
{
  FmMacTable& t = _fm.macTable();
  t.call(p);

  if ( p & FmTahoe::Frame_Ctrl_Ip::FIFO_Full ) {
    _entries.clear();
    for(unsigned v=0; v<FmMacTable::Entries; v++)
      _setup_entry(v);
  }
  else {
    unsigned i=0;
    int v;
    while( (v=t.updated(i++))!=-1 )
      _setup_entry(v);
  }
}

void MACHandler::clear_port(FmPort p)
{
  FmMacTable& t = _fm.macTable();
  unsigned mask = (1<<p);
  if (_allow & mask) {
    std::set<uint16_t> r;
    for(std::set<unsigned short>::iterator it=_entries.begin();
        it!=_entries.end(); it++) {
      SwmMacTableEntry e(t.entry(*it));
      unsigned m = unsigned(e.port_mask()) & mask;
      if (m) {
        r.insert(*it);
        t.deleteEntry(t.entry(*it));
      }
    }
    for(std::set<unsigned short>::iterator it=r.begin();
        it!=r.end(); it++)
      _entries.erase(*it);
  }
}
