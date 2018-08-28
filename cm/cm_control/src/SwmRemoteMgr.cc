
#include "cm_control/SwmRemoteMgr.hh"

#include "cm_control/SwmDebug.hh"
#include "cm_control/AbsConsole.hh"
#include "fm/PortMap.hh"
#include "fm/SwmReply.hh"

#include "time/Time.hh"


void SwmRemoteMgr::process( const SwmReply& reply, AbsConsole& o )
{
  { unsigned sec, nsec;
    char buff[64];
    tool::time::time(sec,nsec);
    o.printv("%s\n",tool::time::asctime( sec, nsec, buff ));
  }

  switch(reply.type()) {
  case SwmReply::RegRead :
    {
      SwmRegReadReply& rrep = (SwmRegReadReply&)reply;
      o.printv("0x%x = 0x%x\n",rrep.reg(), rrep.result());
    }
    break;
  case SwmReply::EplRead :
    {
      SwmEplReadReply& rrep = (SwmEplReadReply&)reply;
      o.printv("== EPL Port (0x%x) ==\n", rrep.reg());
      o << rrep.result();
    }
    break;
  case SwmReply::PortMIB :
    {
      SwmPortMibReply& rrep = (SwmPortMibReply&)reply;
      o.printv("== MIB Port (0x%x) ==\n", rrep.reg());
      o << rrep.result();
    }
    break;
  case SwmReply::PortMSC :
    {
      SwmPortMscReply& rrep = (SwmPortMscReply&)reply;
      o.printv("=== MSC Port (0x%x) ==\n", rrep.reg());
      o << rrep.result();
    }
    break;
  case SwmReply::GlobalMIB :
    {
      SwmGlobalMibReply& rrep = (SwmGlobalMibReply&)reply;
      o.printv("== MIB Global (0x%x) ==\n", rrep.reg());
      o << rrep.result();
    }
    break;
  case SwmReply::PktRead :
    {
      SwmPktReadReply& rrep = (SwmPktReadReply&)reply;
      o.printv("== Trapped Packet ==\n");
      o << rrep.result();
    }
    break;
  case SwmReply::MacEntryRead :
    {
      SwmMacEntryReadReply& rrep = (SwmMacEntryReadReply&)reply;
      if (_port_map) {
	SwmMacTableEntry& e = const_cast<SwmMacTableEntry&>(rrep.result());
	e.set_port_mask(FmPortMask(_map_ports(e.port_mask())));
      }
      o.printv("== MAC Entry ==\n");
      o << rrep.result();
    }
    break;
  case SwmReply::MacTableRead :
  case SwmReply::MacTableDirect :
    {
      SwmMacTableReadReply& rrep = (SwmMacTableReadReply&)reply;
      if (_port_map) {
	for(unsigned k=0; k<SwmMacTable::Entries; k++) {
	  SwmMacTableEntry& e = const_cast<SwmMacTableEntry&>(rrep.result().entry[k]);
	  e.set_port_mask(FmPortMask(_map_ports(e.port_mask())));
	}
      }
      o.printv("== MAC Table BEG ==\n");
      o << rrep.result();
      o.printv("== MAC Table END ==\n");
    }
    break;
  case SwmReply::VlanEntryRead :
    {
      SwmVlanEntryReadReply& rrep = (SwmVlanEntryReadReply&)reply;
      o.printv("== VLAN Entry ==\n");
      o << rrep.result();
    }
    break;
  case SwmReply::FidEntryRead :
    {
      SwmFidEntryReadReply& rrep = (SwmFidEntryReadReply&)reply;
      o.printv("== FID Entry ==\n");
      o << rrep.result();
    }
    break;
  case SwmReply::StreamStatus :
    {
      SwmStreamStatusReply& rrep = (SwmStreamStatusReply&)reply;
      o.printv("== Stream Status ==\n");
      o << rrep.result();
    }
    break;
  case SwmReply::AllPortMIB :
    {
      SwmAllPortMibReply& rrep = (SwmAllPortMibReply&)reply;
      for(unsigned j=0; j<25; j++) {
	o.printv("  Port 0x%x\n",j );
	o << rrep.result().port(j);
      }
    }
    break;
  case SwmReply::AllPortEPL :
    {
      SwmAllPortEplReply& rrep = (SwmAllPortEplReply&)reply;
      for(unsigned j=0; j<24; j++) {
	o.printv("  Port 0x%x\n",j );
	o << rrep.result()[j];
      }
    }
    break;
  default:
    o.printv("Unknown reply type 0x%x\n", reply.type());
  }
}

unsigned SwmRemoteMgr::_map_ports(unsigned mask) const
{
  unsigned v = 0;
  for(unsigned k=0; k<32; k++) {
    if (mask & (1<<_port_map->port(k)))
      v |= (1<<k);
  }
  return v;
}
