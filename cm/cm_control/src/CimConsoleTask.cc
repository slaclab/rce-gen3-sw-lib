#include "bldInfo.h"

#include "control/CimConsoleTask.hh"

#include "control/SwmRemoteMgr.hh"
#include "control/ConsoleHandle.hh"
#include "control/SwmDebug.hh"
#include "control/RceSlaveTest.hh"

#include "net/Manager.hh"

#include "fm/SwmRequest.hh"
#include "fm/SwmReply.hh"
#include "fm/FmTahoe.hh"
#include "fm/FmTahoeMgr.hh"
#include "fm/FmConfig.hh"
#include "fm/SwmPacket.hh"
#include "fm/PortMap.hh"

#include "fci/Manager.hh"
#include "fci/impl/Page.hh"
#include "ffs/IdFile.hh"

#include "phy/mv88e1149.hh"
#include "phy/XfpManager.hh"
#include "phy/BxPhy.hh"

#include "smbus/LM84.hh"

#include "ipmi/IpmTask.hh"
//#include "ipmi/IpmPortState.hh"
#include "ipmi/IpmSDRRequestor.hh"
//#include "ipmi/FRURequestor.hh"
//#include "ipmi/IpmFRUData.hh"
//#include "ipmi/IpmEvent.hh"

#include "cm_svc/CmHw.hh"

extern "C" {
#include <rtems/rtems_bsdnet.h>
}

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <list>

static const unsigned GB_AUX_PORT = 0x17;

static int addscan(ether_addr_t& addr, const char*& args);
static int ipscan(unsigned& addr, const char*& args);
static void _reboot();
static void _setboot(unsigned);
static unsigned _getboot();
static void _reload_fpga(unsigned);
static unsigned _get_rctl();
static void _set_rctl(unsigned);
static void _z2scan         (cm::net::Manager&, AbsConsole&, const char*);
static void _show_int_brief (cm::net::Manager&, AbsConsole&);
static void _show_stat_brief(cm::net::Manager&, AbsConsole&);

#define SCANUARG(var,name)                      \
  unsigned var;                                 \
  if (!sscanf(args,"%x",&var)) {                \
    o.printv("Error scanning %s\n",name);       \
    continue;                                   \
  }                                             \
  args = strtok(0," ");                         \
  args = &args[strlen(args)+1];

static const int BSIZ=0x80000;

CimConsoleTask::CimConsoleTask(cm::net::Manager& mgr,
                               cm::ipm::IpmTask& ipmt) :
  _mode  (UserMode),
  _mgr   (mgr),
  _dev   (new(0) FmTahoe),
  _rmgr  (new    SwmRemoteMgr),
  _reply (new char[BSIZ]),
  _cache (new char[2*sizeof(FmConfig)+
                   service::fci::Params::PagesPerBlock*sizeof(service::fci::Page)]),
  _ipmt  (ipmt),
  _addendum(new RceSlaveTest)
{
}

CimConsoleTask::~CimConsoleTask()
{
  delete[] _cache;
  delete[] _reply;
  delete   _rmgr;
  delete   _addendum;
}

static void _portFormatError(AbsConsole& o)
{
  o.printv("Incorrect Port Id NXX : N={'b','f'}, XX=number\n");
}


unsigned CimConsoleTask::_getPort(const char* port) const throw(tool::exception::Event)
{
  unsigned i;
  if (sscanf(&port[1],"%x",&i)) {
    if (i<32) {
      if      (port[0]=='b') return unsigned(PortMap::base_map()  ->port(i));
      else if (port[0]=='f') return unsigned(PortMap::fabric_map()->port(i)) | (1<<16);
    }
  }
  throw tool::exception::Event("CimConsoleTask::_getPort unrecognized port %s",port);
}


bool CimConsoleTask::handle(service::console::RemoteConsole& console,
                            const char* cmd,
                            const char* args)
{
  ConsoleHandle o(console);

  unsigned _reply_len;
  FmTahoe* t = _dev;

  try {
    do {
      SwmRequest* req     = 0;

      //
      // User mode
      //
      if (_mode==UserMode) {
        if (!strcasecmp(cmd,"PORT_STATUS")) {
          int iport = _getPort(args);
          if (iport<0) { _portFormatError(o); continue; }
          if ((iport--&0x1f)==0) { o.printv("CPU port\n"); continue; }
          _dev = new(iport>>16) FmTahoe;
          req = new SwmEplReadRequest(&_dev->epl_phy[iport&0x1f]);
        }
        else if (!strcasecmp(cmd,"PORT_COUNTER")) {
          int iport = _getPort(args);
          if (iport<0) { _portFormatError(o); continue; }
          _dev = new(iport>>16) FmTahoe;
          req = new SwmPortMibRequest(&_dev->port_mib[iport&0x1f]);
        }
        else if (!strcasecmp(cmd,"PORT_MACS")) {
          char c;
          sscanf(args,"%c",&c);
          if      (c=='b') _dev = new(0) FmTahoe;
          else if (c=='f') _dev = new(1) FmTahoe;
          else continue;
          req = new SwmMacTableReadRequest( _dev );
        }
        else if (!strcasecmp(cmd,"PORTS")) {
          _dumpUserPorts( o );
          continue;
        }
        else if (!strcasecmp(cmd,"EXPERT")) {
          _mode = ExpertMode;
          _rmgr->setPortMap(0);
          continue;
        }
        else {
          return false;
        }

        if (req) {
          // Handle the Request
          _mgr.process(*req, (void*)_reply, _reply_len);

          if (_reply_len) {
            //  Handle the _Reply
            _rmgr->setPortMap( _dev==0 ? PortMap::base_map() : PortMap::fabric_map() );
            _rmgr->process((const SwmReply&)*_reply, o);
          }

          delete req;
          req = 0;
          continue;
        }
      }

      //
      // Expert mode
      //
      if (!strcasecmp(cmd,"DEV")) {
        SCANUARG(i,"device ID");
        o.printv("Setting active device ID to %x\n",i);
        _dev = new (i) FmTahoe;
      }
      else if (!strcasecmp(cmd,"RREG")) {
        SCANUARG(ireg,"register ID");
        o.printv("Reading register 0x%x\n",ireg);
        req = new SwmRegReadRequest((FmMReg*)t+ireg);
      }
      else if (!strcasecmp(cmd,"WREG")) {
        unsigned ireg,ival;
        if (!sscanf(args,"%x %x",&ireg,&ival)) {
          o.printv("Error scanning register ID and value\n");
          continue;
        }
        o.printv("Writing register 0x%x with 0x%x\n", ireg, ival);
        req = new SwmRegWriteRequest((FmMReg*)t+ireg,ival);
      }
      else if (!strcasecmp(cmd,"REPL")) {
        SCANUARG(iport,"port ID");
        req = new SwmEplReadRequest(&t->epl_phy[iport]);
      }
      else if (!strcasecmp(cmd,"REPLALL")) {
        req = new SwmAllPortEplRequest( t );
      }
      else if (!strcasecmp(cmd,"READCM")) {
        req = new SwmStreamStatusRequest( t );
      }
      else if (!strcasecmp(cmd,"FLOOD")) {
        SCANUARG(enable,"enable");
        unsigned v = t->sys_cfg_1;
        if (enable)
          v |= (3<<13);
        else
          v &= ~(3<<13);
        t->sys_cfg_1 = v;
      }
      else if (!strcasecmp(cmd,"AGE")) {
        SCANUARG(ticks,"ticks");
        if (ticks)
          t->sys_cfg_7 = ticks;
        else
          t->sys_cfg_7 = 0x80000000;
      }
      else if (!strcasecmp(cmd,"CUTTHRU")) {
        unsigned state=1;
        sscanf(args,"%x",&state);
        if (state) {
          unsigned ds[4]; memset(ds,0,4*sizeof(unsigned));
          for(unsigned k=0; k<24; k++)
            ds[t->epl_phy[k].ds()] |= (1<<(k+1));
          for(unsigned k=0; k<24; k++)
            t->saf_matrix[k+1] = ~ds[t->epl_phy[k].ds()];
        }
        else {
          for(unsigned k=0; k<24; k++)
            t->saf_matrix[k+1] = 0xffffffff;
        }
      }
      else if (!strcasecmp(cmd,"TRAP")) {
        SCANUARG(ports,"port mask");
        // trap the packet on a trigger and redirect to port 0
        t->trigger_cfg[0] = 0x10010 | (0<<19);
        t->trigger_rx [0] = ports;

        // receive the trapped packet
        SwmPacket pkt_rx(*t);
        o.printv("\nrx:  len 0x%x  dst 0x%x  tail 0x%x\n",
                 pkt_rx.len(), pkt_rx.dst(), pkt_rx.data()[pkt_rx.len()]);
        const unsigned* p = pkt_rx.data();
        for(unsigned j=0; j<pkt_rx.len(); j++) {
          o.printv("%08x",*p++);
          if (j%8==7) o.printv("\n");
        }
        o.printv("\n");
        // remove the trap
        t->trigger_rx [0] = 0;
      }
      else if (!strcasecmp(cmd,"RPMIBALL")) {
        req = new SwmAllPortMibRequest( t );
      }
      else if (!strcasecmp(cmd,"RPMIB")) {
        SCANUARG(iport,"port ID");
        req = new SwmPortMibRequest( &t->port_mib[iport] );
      }
      else if (!strcasecmp(cmd,"RPMSC")) {
        SCANUARG(iport,"port ID");
        req = new SwmPortMscRequest( &t->port_msc[iport] );
      }
      else if (!strcasecmp(cmd,"RGMIB")) {
        req = new SwmGlobalMibRequest( t );
      }
      else if (!strcasecmp(cmd,"CPUPORTSTATUS")) {
        o.printv("%08x\n",(unsigned)(t->cpuPortStatus()));
      }
      else if (!strcasecmp(cmd,"RTEST")) {
        unsigned reg, cycles, delay;
        if (sscanf(args,"%x %x %x",&reg,&cycles,&delay)!=3) {
          o.printv("Error scanning register, cycles, delay\n");
          continue;
        }
        FmSReg* r = (FmSReg*)t;
        r += reg;
        FmReg::clear();
        volatile unsigned v = *r;
        if (!delay) {
          do {
            v &= *r;
          } while (cycles--);
        }
        else {
          do {
            v &= *r;
            volatile unsigned u=0xffffffff;
            for(unsigned k=0; k<delay; k++)
              u ^= (u+1);
            v &= u;
          } while (cycles--);
        }
        if (FmReg::status())
          o.printv("Failure\n");
        else
          o.printv("Success 0x%x\n",v);
      }
      else if (!strcasecmp(cmd,"SSCAN")) {
        SCANUARG(portID,"port ID");
        FmPort port(portID);
        FmEplRegs& r0 = ((FmTahoe*)(new (0) FmTahoe))->epl_phy[portID-1];
        FmEplRegs& r1 = ((FmTahoe*)(new (1) FmTahoe))->epl_phy[portID-1];

        _mgr.deviceMgr(1).configurePort(port,cm::fm::X10Gb,1,0,0);
        for(unsigned k=0; k<0x300; k++) {
          _mgr.deviceMgr(0).configurePort(port,cm::fm::X10Gb,1,0,k);
          unsigned s = r1.serdes_ip;
          o.reserve(150);
          o.printv("setting       : 0x%x\n", k);
          o.printv("serdes_ctrl_1 : 0x%x\n", (unsigned)r0.serdes_ctrl_1);
          o.printv("serdes_ctrl_2 : 0x%x\n", (unsigned)r0.serdes_ctrl_2);
          o.printv("serdes_ip     : 0x%x\n", (unsigned)r1.serdes_ip);
          o.printv(" (0x%x)\n",s);
        }
      }
      else if (!strcasecmp(cmd,"SSCANB")) {
        unsigned remoteIP,sw,port;
        char buff[16];
        if (sscanf(args,"%x %x %x %s",&remoteIP,&sw,&port,buff)!=4) {
          o.printv("Error scanning host IP, switch, port, speed\n");
          continue;
        }
        cm::fm::PortSpeed speed = cm::fm::None;
        if      (!strcasecmp(buff,"10G"))     speed = cm::fm::X10Gb;
        else if (!strcasecmp(buff,"4G"))      speed = cm::fm::X4Gb;
        else if (!strcasecmp(buff,"2_5G"))    speed = cm::fm::X2_5Gb;
        else if (!strcasecmp(buff,"1G"))      speed = cm::fm::X1Gb;
        else if (!strcasecmp(buff,"100M"))    speed = cm::fm::X100Mb;
        else if (!strcasecmp(buff,"10M"))     speed = cm::fm::X10Mb;

        service::console::RemoteConsole con2(1101);
        FmEplRegs& r1 = ((FmTahoe*)(new (sw) FmTahoe))->epl_phy[port];

        con2.printv(" \n");
        con2.flush(remoteIP,1100);
        con2.printv("DEV %d\n",sw);
        con2.flush(remoteIP,1100);
        //      con2.clear_input();

        for(unsigned k=0; k<0x300; k++) {
          con2.printv("UP%s %x 0 0 %x\n",buff,port,k);
          con2.flush(remoteIP,1100);
          con2.getline();

          unsigned s = r1.serdes_ip;
          o.reserve(150);
          o.printv("setting       : 0x%x\n", k);
          //    o.printv("serdes_ctrl_1 : 0x%x\n", (unsigned)r0.serdes_ctrl_1);
          //    o.printv("serdes_ctrl_2 : 0x%x\n", (unsigned)r0.serdes_ctrl_2);
          o.printv("serdes_ip     : 0x%x (0x%x)\n", (unsigned)r1.serdes_ip,s);
        }
      }
      else if (!strcasecmp(cmd,"SENDPKT")) {
        ether_addr_t src,dst;
        if (!addscan(src,args)) {
          o.printv("Error scanning src address\n");
          continue;
        }
        if (!addscan(dst,args)) {
          o.printv("Error scanning dst address\n");
          continue;
        }
        SCANUARG(size,"packet size");

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

        t->txPacket(pkt.data(), pkt.bytes(), FmPortMask(pkt.dst()));
      }
      else if (!strcasecmp(cmd,"DOWN")) {
        SCANUARG(portID,"port ID");
        FmPort port(portID);
        _mgr.deviceMgr(t->id()).configurePort(port,cm::fm::None);
      }
      else if (!strncmp(cmd,"UP",2)) {
        unsigned portID,rxFlip=0,txFlip=0,drive=0;
        if (!sscanf(args,"%x %x %x %x",&portID,&rxFlip,&txFlip,&drive)) {
          o.printv("Error scanning port ID\n");
          continue;
        }
        FmPort port(portID);
        cm::fm::PortSpeed speed = cm::fm::None;
        if      (!strcasecmp(&cmd[2],"10G"))  speed = cm::fm::X10Gb;
        else if (!strcasecmp(&cmd[2],"4G"))   speed = cm::fm::X4Gb;
        else if (!strcasecmp(&cmd[2],"2_5G")) speed = cm::fm::X2_5Gb;
        else if (!strcasecmp(&cmd[2],"1G"))   speed = cm::fm::X1Gb;
        else if (!strcasecmp(&cmd[2],"100M")) speed = cm::fm::X100Mb;
        else if (!strcasecmp(&cmd[2],"10M"))  speed = cm::fm::X10Mb;

        _mgr.deviceMgr(t->id()).configurePort(port,speed,
                                              rxFlip,txFlip,drive);
      }
      else if (!strcasecmp(cmd,"READMAC")) {
        ether_addr_t addr;
        if (!addscan(addr,args)) {
          o.printv("Error scanning MAC address\n");
          continue;
        }
        SwmMacTableEntry e(addr,FmPortMask(0),0,0,0,0);
        req = new SwmMacEntryReadRequest( t, e );
      }
      else if (!strcasecmp(cmd,"ADDMAC")) {
        ether_addr_t addr;
        unsigned portmask, vlan, trigId;
        char lockChar;
        if (!addscan(addr,args) ||
            sscanf(args,"%x %x %x %c",
                   &portmask, &vlan, &trigId, &lockChar )!=4) {
          o.printv("Error scanning MAC entry\n");
          continue;
        }
        SwmMacTableEntry e(addr,FmPortMask(portmask),vlan,trigId,lockChar=='S'? 1:0,1);
        req = new SwmMacEntryWriteRequest( t, e );
      }
      else if (!strcasecmp(cmd,"DELETEMAC")) {
        ether_addr_t addr;
        if (!addscan(addr,args)) {
          o.printv("Error scanning MAC address\n");
          continue;
        }
        SwmMacTableEntry e(addr,FmPortMask(0),0,0,0,0);
        req = new SwmMacEntryWriteRequest( t, e );
      }
      else if (!strcasecmp(cmd,"READMACTAB")) {
        req = new SwmMacTableReadRequest( t );
      }
      else if (!strcasecmp(cmd,"READMACTABD")) {
        req = new SwmMacTableDirectRequest( t );
      }
      else if (!strcasecmp(cmd,"READVLANENTRY")) {
        SCANUARG(entry,"entry ID");
        req = new SwmVlanEntryReadRequest( &t->vid_table[entry] );
      }
      else if (!strcasecmp(cmd,"WRITEVLANENTRY")) {
        unsigned entry, m, vcnt, rfl;
        if (sscanf(args,"%x %x %x %x",&entry,&m,&vcnt,&rfl)!=4) {
          o.printv("Error scanning vlan entry id, ports, vcnt, reflect\n");
          continue;
        }
        SwmVlanTableEntry e(m,0,0,vcnt,rfl);
        req = new SwmVlanEntryWriteRequest( &t->vid_table[entry], e );
      }
      else if (!strcasecmp(cmd,"READFIDENTRY")) {
        SCANUARG(entry,"entry ID");
        req = new SwmFidEntryReadRequest( &t->fid_table[entry] );
      }
      else if (!strcasecmp(cmd,"WRITEFIDENTRY")) {
        unsigned entry, ufwd;
        if (sscanf(args,"%x %x",&entry,&ufwd)!=2) {
          o.printv("Error scanning stp entry id\n");
          continue;
        }
        FmPortMask empty(0), fwd(ufwd);
        SwmFidTableEntry e(empty,empty,fwd);
        req = new SwmFidEntryWriteRequest( &t->fid_table[entry], e );
      }
      else if (!strcasecmp(cmd,"SNAKE_EN")) {
#if 0
        cm::fm::PortSpeed speed = cm::fm::None;
        args = strtok(0," ");
        if      (!strcasecmp(args,"1"  )) speed = cm::fm::X1Gb;
        else if (!strcasecmp(args,"2_5")) speed = cm::fm::X2_5Gb;
        else if (!strcasecmp(args,"10" )) speed = cm::fm::X10Gb;
        else {
          o.printv("Cannot parse speed %s\n",args);
          continue;
        }
#endif
        _nsnports = 0;
        args = strtok(0," ");
        while ( args ) {
          sscanf(args,"%x",&_snports[_nsnports++]);
          args = strtok(0," ");
        }
        if (!_nsnports) break;

        _nsnports--;

        o.printv("%x->%x (+HOST)\n", GB_AUX_PORT, _snports[0]);
        t->port_cfg_2[GB_AUX_PORT+1] = (1<<(_snports[0]+1)) | (1<<0);

        for(unsigned k=0; k<_nsnports; k++) {
          o.printv("%x->%x\n", _snports[k], _snports[k+1]);
          t->port_cfg_2[_snports[k]+1] = 1<<(_snports[k+1]+1);
#if 0
          _mgr.deviceMgr(t->id()).configurePort(_snports[k],speed);
#endif
        }
      }
      else if (!strcasecmp(cmd,"SNAKE_DIS")) {
        o.printv("%x->ALL\n", GB_AUX_PORT);
        t->port_cfg_2[GB_AUX_PORT+1] = 0x1FFFFFF;

        for(unsigned k=0; k<_nsnports; k++) {
          o.printv("%x->ALL\n", _snports[k]);
          t->port_cfg_2[_snports[k]+1] = 0x1FFFFFF;
#if 0
          _mgr.deviceMgr(t->id()).configurePort(_snports[k],cm::fm::None);
#endif
        }
        _nsnports = 0;
      }
      else if (!strcasecmp(cmd,"SINK")) {
        SCANUARG(port,"port ID");
        t->port_cfg_2[port] = 0;
      }
      else if (!strcasecmp(cmd,"PTEST")) {
        unsigned d0,up0,d1,up1;
        if (sscanf(args,"%x %x %x %x",&d0,&up0,&d1,&up1)!=4) {
          o.printv("Error scanning dev/port ids\n");
          continue;
        }

        FmPort p0(up0), p1(up1);

        timespec ts; ts.tv_sec = 0; ts.tv_nsec = 10000000;

        //  prepare the test packet
        const unsigned size = 0x40;
        char* buf = new char[size];
        char* b = buf;
        memset(b,0x0f,6); b += 6;  // destination
        memset(b,0x00,6); b += 6;  // source
        for(unsigned k=0; k<size-12; k++)
          *b++ = k&0xff;

        unsigned* ub = (unsigned*)buf;
        unsigned* ue = ub + ((size+3)>>2);
        SwmPacket pkt(0);
        while( ub < ue )
          pkt.append(*ub++);

        //  Sink data received at both of these ports
        FmTahoe* t0 = new(d0) FmTahoe;
        FmTahoe* t1 = new(d1) FmTahoe;
        t0->port_cfg_2[unsigned(p0)] = 0;
        t1->port_cfg_2[unsigned(p1)] = 0;

        for(unsigned k_rx = 0; k_rx < 2; k_rx++) {
          for(unsigned k_tx = 0; k_tx < 2; k_tx++) {
            //  Configure the ports
            //    0xF -> 0x10
            _mgr.deviceMgr(d0).configurePort(p0,cm::fm::X10Gb,k_rx,k_tx);
            _mgr.deviceMgr(d1).configurePort(p1,cm::fm::X10Gb,k_rx,k_tx);

            { //  Read the port counters
              SwmPortMibCounters before( t1->port_mib[unsigned(p1)] );
              //  Transmit the packet out the first interface
              t0->txPacket(pkt.data(), pkt.bytes(), FmPortMask(1<<unsigned(p0)));
              nanosleep(&ts,0);
              //  Read the port counters
              SwmPortMibCounters after( t1->port_mib[unsigned(p1)] );

              o.printv("before\n");
              o << before;
              o.printv("after\n");
              o << after;

              o.printv("%x/%x(%x) -> %x/%x(%x) : ", d0,unsigned(p0), k_tx, d1,unsigned(p1), k_rx);
              if      (before.rxMcast     < after.rxMcast)
                o.printv("OK\n");
              else if (before.rxFcsErrors < after.rxFcsErrors)
                o.printv("CRC\n");
              else
                o.printv("--\n");
            }

            { //  Read the port counters
              SwmPortMibCounters before( t0->port_mib[unsigned(p0)] );
              //  Transmit the packet out the first interface
              t1->txPacket(pkt.data(), pkt.bytes(), FmPortMask(1<<unsigned(p1)));
              nanosleep(&ts,0);
              //  Read the port counters
              SwmPortMibCounters after( t0->port_mib[unsigned(p0)] );

              o.printv("before\n");
              o << before;
              o.printv("after\n");
              o << after;

              o.printv("%x/%x(%x) -> %x/%x(%x) : ", d1,unsigned(p1), k_tx, d0,unsigned(p0), k_rx);
              if      (before.rxMcast     < after.rxMcast)
                o.printv("OK\n");
              else if (before.rxFcsErrors < after.rxFcsErrors)
                o.printv("CRC\n");
              else
                o.printv("--\n");
            }
          }
        }
        delete[] buf;
      }
      else if (!strcasecmp(cmd,"VSN")) {
        o.printv("HW Version: %s\n",cm::svc::CmHw::get_version_tag());
        time_t vsntim = cm::svc::CmHw::get_version_time();
        o.printv("HW Build time: %s\n",ctime(&vsntim));
        o.printv("SW Version: %s\n",cm::control::bldRevision());
        o.printv("SW Build time: %s\n",cm::control::bldDate());
        continue;
      }
      else if (!strcasecmp(cmd,"RESET")) {
        req = new SwmResetRequest( t );
      }
      else if (!strcasecmp(cmd,"ENINTR")) {
        FmTahoe::enableInterrupts(t->id());
      }
      else if (!strcasecmp(cmd,"DISINTR")) {
        FmTahoe::disableInterrupts(t->id());
      }
      else if (!strcasecmp(cmd,"WRITEID")) {
        char* name = strtok(0," ");
        char* vsn  = strtok(0," ");
        char* sno  = strtok(0," ");
        char* macB = strtok(0," ");
        char* macF = strtok(0," ");
        cm::ffs::IdFile* idf = new cm::ffs::IdFile(name,vsn,sno,macB,macF);
        o.printv("Wrote IDFILE : %s %s %s %s %s\n",
                 idf->name(), idf->version(), idf->serialNumber(),
                 idf->macBase(), idf->macFabric());
        delete idf;
      }
      else if (!strcasecmp(cmd,"READID")) {
        cm::ffs::IdFile* idf = new cm::ffs::IdFile;
        o.printv("Read IDFILE : %s %s %s %s %s\n",
                 idf->name(), idf->version(), idf->serialNumber(),
                 idf->macBase(), idf->macFabric());
        delete idf;
      }
      else if (!strcasecmp(cmd,"DUMPCFG")) {
        FmConfig* fmCfg = reinterpret_cast<FmConfig*>(_cache);
        for(unsigned k=0; k<2; k++) {
          o.printv("=== FF0%d ===\n" ,k);
          o << fmCfg[k];
        }
      }
      else if (!strcasecmp(cmd,"EXTRACTCFG")) {
        FmConfig* fmCfg = reinterpret_cast<FmConfig*>(_cache);
        for(unsigned k=0; k<_mgr.nDevices(); k++)
          fmCfg[k] = _mgr.config(k);
      }
      else if (!strcasecmp(cmd,"LOADCFG")) {
        SCANUARG(file,"file ID");
        service::fci::Manager* mgr = service::fci::Manager::instance();
        service::fci::File f(file);
        if (mgr->exists(f))
          mgr->read(f,_cache,2*sizeof(FmConfig));
        else
          o.printv("File %s does not exist\n", file);
      }
      else if (!strcasecmp(cmd,"SAVECFG")) {
        SCANUARG(file,"file ID");
        service::fci::Manager* mgr = service::fci::Manager::instance();
        service::fci::File f(file);
        if (mgr->exists(f))
          mgr->remove(f);
        mgr->write(f,_cache,2*sizeof(FmConfig));
        mgr->writeDir();
      }
      else if (!strcasecmp(cmd,"EDITCFG")) {
        unsigned sw, ip_addr, ip_netmask;
        char if_name[16];
        if (sscanf(args,"%x %s %x %x",&sw,if_name,&ip_addr,&ip_netmask)!=4) {
          o.printv("Error scanning switch ID, if_name, ip_addr, ip_netmask\n");
          continue;
        }
        for(unsigned k=0; k<4; k++)
          args = strtok(0," ");
        ether_addr_t hw_addr;
        if (!addscan(hw_addr,args)) {
          o.printv("Error scanning mac addr\n");
          continue;
        }

        FmConfig* fmCfg = reinterpret_cast<FmConfig*>(_cache);
        FmHostIfConfig& h = fmCfg[sw].hostCfg;
        strncpy(h.if_name,if_name,8);
        h.ip_addr = ip_addr;
        h.ip_netmask = ip_netmask;
        strncpy((char*)h.mac_addr,(char*)&hw_addr.ether_addr_octet[0],6);
      }
      else if (!strcasecmp(cmd,"BOOTSEL")) {
        unsigned v = _getboot();
        o.printv("Boot vector is set to %x\n",v);
      }
      else if (!strcasecmp(cmd,"BOOTSET")) {
        unsigned vec=0;
        if (sscanf(args,"%x",&vec))
          _setboot(vec&0xf);
      }
#if 1
      else if (!strcasecmp(cmd,"REBOOT")) {
        unsigned vec=0;
        if (sscanf(args,"%x",&vec)) {
          _setboot(vec&0xf);
        }
        _reboot();
      }
#endif
      else if (!strcasecmp(cmd,"RELOAD")) {
        unsigned rev=0;
        if (sscanf(args,"%x",&rev))
          _reload_fpga(rev);
      }
      else if (!strcasecmp(cmd,"RCTLR")) {
        unsigned v = _get_rctl();
        o.printv("Rctl vector is %x\n",v);
      }
      else if (!strcasecmp(cmd,"RCTLW")) {
        unsigned v=0;
        if (sscanf(args,"%x",&v))
          _set_rctl(v);
      }
      else if (!strcasecmp(cmd,"RPHY")) {
        unsigned reg,dev;
        if (sscanf(args,"%x %x",&dev,&reg)!=2) {
          o.printv("Error scanning device and register\n");
          continue;
        }
        MvPhy* d = (MvPhy*)0;
        d += dev;
        unsigned v = d->r[reg];
        o.printv("RPHY %x %x = %x\n",dev,reg,v);
      }
      else if (!strcasecmp(cmd,"WPHY")) {
        unsigned reg,dev,val;
        if (sscanf(args,"%x %x %x",&dev,&reg,&val)!=3) {
          o.printv("Error scanning device, register, and value\n");
          continue;
        }
        MvPhy* d = (MvPhy*)0;
        d += dev;
        d->r[reg] = val;
      }
      else if (!strcasecmp(cmd,"RXFPF")) {
        unsigned phy,dev,reg;
        if (sscanf(args,"%x %x %x",&phy,&dev,&reg)!=3) {
          o.printv("Error scanning phy, device and register\n");
          continue;
        }
        XfpManager<FrontPanel>* xfp = XfpManager<FrontPanel>::instance();
        unsigned v = xfp->phy(phy).dev[dev].r[reg];
        o.printv("RXFPF %x/%x/%x = %x\n",phy,dev,reg,v);
      }
      else if (!strcasecmp(cmd,"WXFPF")) {
        unsigned phy,reg,dev,val;
        if (sscanf(args,"%x %x %x %x",&phy,&dev,&reg,&val)!=4) {
          o.printv("Error scanning phy, device, register, and value\n");
          continue;
        }
        XfpManager<FrontPanel>* xfp = XfpManager<FrontPanel>::instance();
        xfp->phy(phy).dev[dev].r[reg] = val;
      }
      else if (!strcasecmp(cmd,"RXFPR")) {
        unsigned phy,dev,reg;
        if (sscanf(args,"%x %x %x",&phy,&dev,&reg)!=3) {
          o.printv("Error scanning phy, device and register\n");
          continue;
        }
        XfpManager<RearModule>* xfp = XfpManager<RearModule>::instance();
        unsigned v = xfp->phy(phy).dev[dev].r[reg];
        o.printv("RXFPR %x/%x/%x = %x\n",phy,dev,reg,v);
      }
      else if (!strcasecmp(cmd,"WXFPR")) {
        unsigned phy,reg,dev,val;
        if (sscanf(args,"%x %x %x %x",&phy,&dev,&reg,&val)!=4) {
          o.printv("Error scanning phy, device, register, and value\n");
          continue;
        }
        XfpManager<RearModule>* xfp = XfpManager<RearModule>::instance();
        xfp->phy(phy).dev[dev].r[reg] = val;
      }
      else if (!strcasecmp(cmd,"RBE")) {
        unsigned dev,reg;
        if (sscanf(args,"%x %x",&dev,&reg)!=2) {
          o.printv("Error scanning device and register\n");
          continue;
        }
        BxPhy* bx = BxPhy::instance();
        unsigned v = bx->dev(dev).r[reg];
        o.printv("RBE %x/%x = %x\n",dev,reg,v);
      }
      else if (!strcasecmp(cmd,"WBE")) {
        unsigned reg,dev,val;
        if (sscanf(args,"%x %x %x",&dev,&reg,&val)!=3) {
          o.printv("Error scanning device, register, and value\n");
          continue;
        }
        BxPhy* bx = BxPhy::instance();
        bx->dev(dev).r[reg] = val;
      }
      else if (!strcasecmp(cmd,"BEINIT")) {
        BxPhy::instance()->reset();
        o.printv("BEINIT done\n");
      }
      else if (!strcasecmp(cmd,"BXEN")) {
        unsigned val;
        if (sscanf(args,"%x",&val)!=1) {
          o.printv("BXEN mask %08x\n",unsigned(BxPhy::instance()->manage()));
        }
        else {
          BxPhy::instance()->manage(FmPortMask(val));
        }
      }
      else if (!strcasecmp(cmd,"TMON")) {
        //  Temperature monitors
        unsigned itmp;
        if (sscanf(args,"%x",&itmp)!=1) {
          o.printv("Error scanning temperature device\n");
          continue;
        }
        LM84* tmon = new(itmp) LM84;
        const int treg = 1;
        unsigned val = tmon->r[treg];
        o.printv("TEMP %x = %x\n",itmp,val);
      }
      else if (!strcasecmp(cmd,"MCASTJOIN")) {
        unsigned addr;
        int port = 1201;
        if (!ipscan(addr,args)) {
          o.printv("Error scanning multicast address\n");
          continue;
        }
        cm::control::SocketTest* s = _sockets.head();
        const cm::control::SocketTest* const end = _sockets.empty();
        while( s!=end ) {
          if (s->address()==addr)
            break;
          s = s->flink();
        }
        if ( s==end ) {
          s = new cm::control::SocketTest(0xc0a80002,addr,port);
          _sockets.insert( s );
        }
        s->join();
        _mgr.deviceMgr(0).changePortMulticastGroupMembership(FmPortMask(1<<0),addr,true);
      }
      else if (!strcasecmp(cmd,"MCASTRESIGN")) {
        unsigned addr;
        if (!ipscan(addr,args)) {
          o.printv("Error scanning multicast address\n");
          continue;
        }
        cm::control::SocketTest* s = _sockets.head();
        const cm::control::SocketTest* const end = _sockets.empty();
        while( s!=end ) {
          if (s->address()==addr)
            break;
          s = s->flink();
          _mgr.deviceMgr(0).changePortMulticastGroupMembership(FmPortMask(1<<0),addr,false);
        }
        if ( s==end ) {
          o.printv("no socket with addr %x\n",addr);
          continue;
        }
        s->resign();
        s->remove();
        delete s;
      }
      else if (!strcasecmp(cmd,"ETHSTATS")) {
        rtems_bsdnet_show_mbuf_stats();
        rtems_bsdnet_show_if_stats();
        rtems_bsdnet_show_ip_stats();
        rtems_bsdnet_show_udp_stats();
      }
      else if (!strcasecmp(cmd,"USER")) {
        _mode = UserMode;
        continue;
      }
//       else if (!strcasecmp(cmd,"IPMIINIT")) {
//      _ipmc.initialize();
//       }
      else if (!strcasecmp(cmd,"IPMISDR")) {
        SCANUARG(slot,"slot");
        cm::ipm::IpmSDRRequestor* job = new cm::ipm::IpmSDRRequestor((slot|0x40)<<1, 0);
        _ipmt.post_requestor(job);
        continue;
      }
//       else if (!strcasecmp(cmd,"IPMIFRU")) {
//      SCANUARG(slot,"slot");
//      cm::ipm::FRURequestor* job = new cm::ipm::FRURequestor((slot|0x40)<<1, 0);
//      _ipmt.post_requestor(job);
//      continue;
//       }
//       else if (!strcasecmp(cmd,"IPMIGET")) {
//      unsigned hdl, slot, ch;
//      char netw;
//      if (sscanf(args,"%x %x %c %x",&hdl,&slot,&netw,&ch)!=4) {
//        o.printv("Error scanning <bus> <slot> <netw> <channel>\n");
//        continue;
//      }
//      cm::ipm::IpmHandle& ipm = hdl ? _ipm1 : _ipm0;
//      unsigned addr = ipm.address();
//      cm::ipm::PortState::Interface type = (netw=='b' || netw=='B') ?
//        cm::ipm::PortState::Base : cm::ipm::PortState::Fabric;
//      cm::ipm::GetPortStateRequest request((slot|0x40)<<1, 0,
//                                          addr, 0,
//                                          type, ch);
//      ipm.tx( &request, sizeof(request) );
//      continue;
//       }
//       else if (!strcasecmp(cmd,"IPMISET")) {
//      unsigned slot, ch, state;
//      char netw;
//      if (sscanf(args,"%x %c %x %x",&slot,&netw,&ch,&state)!=4) {
//        o.printv("Error scanning <slot> <netw> <channel> <state>\n");
//        continue;
//      }
//      unsigned addr = _ipm0.address();
//      cm::ipm::PortState::Interface type = (netw=='b' || netw=='B') ?
//        cm::ipm::PortState::Base : cm::ipm::PortState::Fabric;
//      cm::ipm::PortState pstate( type, ch, state!=0);
//      cm::ipm::SetPortStateRequest request((slot|0x40)<<1, 0,
//                                          addr, 0,
//                                          pstate);
//      _ipm0.tx( reinterpret_cast<const unsigned char*>(&request),
//                sizeof(request) );
//      continue;
//       }
//       else if (!strcasecmp(cmd,"IPMIGETPROP")) {
//      SCANUARG(slot,"slot");
//      cm::ipm::GetPICMGPropertiesRequest request((slot|0x40)<<1, 0, _ipm0.address(), 0);
//      _ipm0.tx( reinterpret_cast<const unsigned char*>(&request),
//                sizeof(request) );
//      continue;
//       }
//       else if (!strcasecmp(cmd,"IPMIGETFRUSIZE")) {
//      unsigned slot, id;
//      if (sscanf(args,"%x %x",&slot,&id)!=2) {
//        o.printv("Error scanning <slot> <id>\n");
//        continue;
//      }
//      cm::ipm::GetFRUInventoryAreaRequest request((slot|0x40)<<1, 0, _ipm0.address(), 0, id);
//      _ipm0.tx( reinterpret_cast<const unsigned char*>(&request),
//                sizeof(request) );
//      continue;
//       }
//       else if (!strcasecmp(cmd,"IPMIREADFRU")) {
//      unsigned slot, id, offset, count;
//      if (sscanf(args,"%x %x %x %x",&slot,&id,&offset,&count)!=4) {
//        o.printv("Error scanning <slot> <id> <offset> <count>\n");
//        continue;
//      }
//      cm::ipm::ReadFRUDataRequest request((slot|0x40)<<1, 0, _ipm0.address(), 0, id, offset, count);
//      _ipm0.tx( &request, sizeof(request) );
//      continue;
//       }
//       else if (!strcasecmp(cmd,"IPMIEVENT")) {
//      SCANUARG(state,"state");
//      cm::ipm::IpmEventRequest request(0x20, 0, _ipm0.address(), 0, 0, 0,
//                                      cm::ipm::M2, cm::ipm::M0, cm::ipm::Normal_State_Change);
//      _ipm0.tx( &request, sizeof(request) );
//       }
      else if (!strcasecmp(cmd,"IPMISNOOP")) {
        SCANUARG(lset,"lSet");
        _ipmt.snoop(lset!=0);
        continue;
      }
//       else if (!strcasecmp(cmd,"IPMISTAT")) {
//      unsigned v0 = _ipm0.status();
//      unsigned v1 = _ipm1.status();
//      o.printv("ipmistatus %x/%x\n",v0,v1);
//      continue;
//       }
      else if (!strcasecmp(cmd,"DUMPPAUSE")) {
        SCANUARG(iport,"iport");
        if (iport) {
          t->sys_cfg_1 &= ~FmTahoe::Sys_Cfg_1::DropPause;
          t->epl_phy[iport-1].mac_cfg_2 |= (1<<2); // disable rx pause
        }
        else {
          t->sys_cfg_1 |= FmTahoe::Sys_Cfg_1::DropPause;
          for(iport=0; iport<FmPort::MAX_PORT; iport++)
            t->epl_phy[iport].mac_cfg_2 &= ~(1<<2); // enable rx pause
        }
        continue;
      }
      else if (!strcasecmp(cmd,"Z2SCAN")) {
        _z2scan(_mgr, o, args);
      }
      else if (!strcasecmp(cmd,"Z2SEND")) {
        ether_addr_t src,dst;
        if (!addscan(src,args)) {
          o.printv("Error scanning src address\n");
          continue;
        }
        if (!addscan(dst,args)) {
          o.printv("Error scanning dst address\n");
          continue;
        }
        SCANUARG(size,"packet size");
        SCANUARG(ports,"ports");

        char* buf = new char[size+12];
        char* b = buf;
        memcpy(b,&dst,6); b += 6;
        memcpy(b,&src,6); b += 6;
        for(unsigned k=0; k<size; k++)
          *b++ = k&0xff;

        unsigned* ub = (unsigned*)buf;
        unsigned* ue = ub + (unsigned(b-buf)>>2);
        SwmPacket pkt(ports);
        while( ub < ue )
          pkt.append(*ub++);

        unsigned q = t->port_cfg_2[0];
        t->port_cfg_2[0] = FmPortMask(q | ports);
        t->txPacket(pkt.data(), pkt.bytes(), FmPortMask(pkt.dst()));
        timespec tv; tv.tv_sec=0; tv.tv_nsec=10000000;
        nanosleep(&tv,0);
        t->port_cfg_2[0] = q;
      }
      else if (!strcasecmp(cmd,"Z2PUP")) {
        _show_int_brief(_mgr,o);
      }
      else if (!strcasecmp(cmd,"Z2STAT")) {
        _show_stat_brief(_mgr,o);
      }
      else {
        return _addendum->handle(console, cmd, args);
      }

      if (req) {
        // Handle the Request
        _mgr.process(*req, (void*)_reply, _reply_len);

        if (_reply_len) {
          //  Handle the _Reply
          _rmgr->process((const SwmReply&)*_reply, o);
        }

        delete req;
        req = 0;
      }

    } while (0);
  } catch(tool::exception::Event& e) {}

  return true;
};


struct cmd_help_s {
  const char* command;
  const char* descr;
};

typedef struct cmd_help_s cmd_help_t;

static const char* user_info[] =
  { " Ports are mapped as follows",
    " 0    \t CPU host port",
    " 1..F \t Zone 2 slot (1..2=hub, 3..F=payload)",
    "10..17\t Zone 3 {RTM,HubXC}",
    "1C    \t FrontPanel RJ45",
    "1D    \t FrontPanel XFP",
    NULL };

static const char* expert_info[] = { NULL };

static cmd_help_t user_help[] =
  { { "PORT_STATUS <port>", "Link status of port <NXX> : N={'b','f'}, XX=slot" },
    { "PORT_COUNTER <port>", "Port traffic counters for port <NXX>" },
    { "PORT_MACS", "Dump MAC table" },
    { "PORTS", "Dump summary of all ports" },
    { "EXPERT", "Enter Expert mode" },
    { NULL, NULL } };

static cmd_help_t expert_help[] =
  { { "DEV <id>", "Select device <id> as active" },
    { "CUTTHRU <enable>", "Enable/Disable cut-through mode for ports with matching speeds" },
    { "TRAP <ports>", "Trap incoming packets on ports <ports> and redirect to the host" },
    //
    //  Low level commands
    //
    { "VSN", "Read the firmware version tag/time registers" },
    { "RESET", "Resets active device (no configuration)" },
    { "ENINTR", "Enables interrupts from active device" },
    { "DISINTR", "Disables interrupts from active device" },
    { "RREG <reg>", "Read register <reg> from active device" },
    { "WREG <reg> <val>", "Write <value> to register <reg> on active device" },
    { "REPL <port>", "Read ethernet port logic registers for port <port> on active device" },
    { "CPUPORTSTATUS", "Read the CPU port status for the active device" },
    { "RTEST <reg> <cycles> <delay>", "Tests the read cycle of register <reg>" },
    { "SSCAN <port>", "Tests the serdes setting." },
    { "SSCANB <ip_addr> <port>", "Tests the serdes setting." },
    { "SENDPKT <src> <dst> <size>", "CPU port sends a packet from MAC addr <src> to <dst>" },
    //
    //  Packet statistics
    //
    { "RPMIB <port>", "Read management information base counters on port <port> on the active device" },
    { "RPMIBALL", "Read management information base counters for all ports on the active device" },
    { "RPMSC <port>", "Read miscellanous counters for port <port> on the active device" },
    { "RGMIB", "Read global management information base counters for the active device" },
    //
    //  Port configuration commands
    //
    { "DOWN <port>" , "Disable port <port>" },
    { "UP<speed> <port>"  , "Enable port <port> at speed {\"10G\",\"2_5G\",\"1G\",\"100M\",\"10M\"}" },
    //
    //  MAC Table commands
    //
    { "READMAC <mac address>", "Read the matching entry for <mac address>" },
    { "ADDMAC <mac address> <ports> <vlan> <trig> <Static/Dynamic>", "Adds <mac address> entry" },
    { "DELETEMAC <mac address>","Removes <mac address> entry" },
    { "READMACTAB", "Reads entire cached mac address table" },
    { "READMACTABD", "Reads entire uncached mac address table" },
    { "AGE <ticks>", "Enable mac address table aging" },
    { "FLOOD <enable>", "Enable/Disable flooding of unmatched destination addresses" },
    //
    //  VLAN / Spanning Tree Protocol
    //
    { "READVLANENTRY <index>", "Reads VLAN entry for <index> (0-15)" },
    { "WRITEVLANENTRY <index> <ports> <vcnt> <rfl>", "Writes entry for VLAN <index>, port mask <ports>, counter <vcnt>, and reflect rule <rfl>" },
    { "READFIDENTRY <vlan>", "Reads Forwarding Information Database entry for <vlan> (0-4094)" },
    { "WRITEFIDENTRY <port> <mask>", "Sets Forwarding Information Database entry for <port> (0-24) to port mask <mask>" },
    { "SNAKE_EN <speed> <port list>","Sets speed{1,2_5,10} and masks so traffic rx on one port can only tx on the next port in the list" },
    { "SNAKE_DIS", "Disables ports and resets masks" },
    { "SINK <port>", "Sinks all rx traffic into <port>" },
    //
    //  Flash commands
    //
    { "RELOAD <rev>", "Reload the FPGA with revision <rev>" },
    { "BOOTSEL", "Read the image select (REBOOT) register" },
    { "BOOTSET <img>", "Write the image select (REBOOT) register" },
    { "DUMPCFG", "Dumps cached switch configurations" },
    { "EXTRACTCFG", "Extracts current switch configurations into the cache" },
    { "LOADCFG <file>", "Loads switch configurations from flash file# <file> into the cache" },
    { "SAVECFG <file>", "Saves cached switch configurations to flash file# <file>" },
    { "EDITCFG <switch> <name> <ip_addr> <ip_mask> <mac_addr>", "Edits cached switch host configurations" },
    { "READID", "Reads the IdFile" },
    { "WRITEID <board name> <vsn> <serialno> <basemac> <fabrmac>", "Writes the IdFile" },
    //
    //  Additional commands
    //
    { "RPHY <dev> <reg>", "Reads device <dev> register <reg>"},
    { "WPHY <dev> <reg> <value>", "Writes <value> to register <dev>/<reg>"},
    { "RXFPF <phy> <dev> <reg>", "Reads device <dev> register <reg>"},
    { "WXFPF <phy> <dev> <reg> <value>", "Writes <value> to register <dev>/<reg>"},
    { "RXFPR <phy> <dev> <reg>", "Reads device <dev> register <reg>"},
    { "WXFPR <phy> <dev> <reg> <value>", "Writes <value> to register <dev>/<reg>"},
    { "RBE <dev> <reg>","Reads Base Ethernet <dev> <reg>"},
    { "WBE <dev> <reg> <value>","Writes Base Ethernet <dev> <reg> with <value>"},
    { "BEINIT","Initializes base ethernet devices"},
    { "BXEN <mask>","Enables 10/100/1000-BaseT Phy for base channels <mask>" },
    { "MCASTJOIN <addr>","Join multicast group"},
    { "MCASTRESIGN <addr>","Resign from multicast group"},
    { "ETHSTATS <if>","Dump host ethernet interface statistics"},
    { "USER", "Enter User mode" },
    { NULL, NULL } };

void CimConsoleTask::help(AbsConsole& o)
{
  const char** help_info = (_mode==UserMode) ? user_info : expert_info;
  for(unsigned k=0; help_info[k] != NULL; k++) {
    o.printv("%s\n",help_info[k]);
  }

  cmd_help_t*  help_menu = (_mode==UserMode) ? user_help : expert_help;
  for(unsigned k=0; help_menu[k].command != NULL; k++) {
    const char* cmdStr = help_menu[k].command;
    const char* dscStr = help_menu[k].descr;
    const char* const cmdEnd = cmdStr + strlen(cmdStr);
    const char* const dscEnd = dscStr + strlen(dscStr);
    int cmdLen = 30;
    int dscLen = 50;
    char sepChr = '-';
    while (cmdStr < cmdEnd || dscStr < dscEnd ) {
      o.printv("%30.30s %c %-50.50s\n",
               cmdStr<cmdEnd ? cmdStr : " ",
               sepChr,
               dscStr<dscEnd ? dscStr : " " );
      cmdStr += cmdLen;
      dscStr += dscLen;
      cmdLen = 28;
      dscLen = 48;
      sepChr = ' ';
    }
  }
}

int addscan(ether_addr_t& addr, const char*& args)
{
  unsigned a[6];
  if (sscanf(args,"%x:%x:%x:%x:%x:%x",
             &a[0],
             &a[1],
             &a[2],
             &a[3],
             &a[4],
             &a[5])!=6)
    return 0;

  args = strtok(0," ");
  args = &args[strlen(args)+1];

  for(unsigned k=0;k<6;k++)
    addr.ether_addr_octet[k]=a[k];

  return 1;
}

int ipscan(unsigned& addr, const char*& args)
{
  unsigned a[4];
  if (sscanf(args,"%d.%d.%d.%d",
             &a[0],
             &a[1],
             &a[2],
             &a[3])!=4)
    return 0;

  args = strtok(0," ");
  args = &args[strlen(args)+1];

  addr = 0;
  for(unsigned k=0;k<4;k++)
    addr = (addr<<8) | a[k];

  return 1;
}

unsigned _getboot()
{
  const unsigned FLSHOPT = 0x2F8;
  unsigned v;
  asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(FLSHOPT));
  return v;
}

void _setboot(unsigned v)
{
  const unsigned FLSHOPT = 0x2F8;
  asm volatile("mtdcr %0, %1" : : "i"(FLSHOPT), "r"(v));
  asm volatile("eieio");
}

void _reboot()
{
  const unsigned DBCR0 = 0x3F2;
  unsigned rst = 0x1<<28;  // processor reset
  //  unsigned rst = 0x2<<28;  // chip reset
  //  unsigned rst = 0x3<<28;  // system reset
  asm volatile("mtspr %0, %1" : : "i"(DBCR0), "r"(rst));
}

void _reload_fpga(unsigned v)
{
  const unsigned RCTL = 0x2F5;
  unsigned w;
  asm volatile("mfdcr %0, %1" : "=r"(w) : "i"(RCTL));
  asm volatile("eieio");
  w &= ~0xF0000;
  w |=  0x40000;
  w |= (v&3) << 16;
  asm volatile("mtdcr %0, %1" : : "i"(RCTL), "r"(w));
  asm volatile("eieio");
  w ^= 0x40000;
  asm volatile("mtdcr %0, %1" : : "i"(RCTL), "r"(w));
  asm volatile("eieio");
  w ^= 0x80000;
  asm volatile("mtdcr %0, %1" : : "i"(RCTL), "r"(w));
}

unsigned _get_rctl()
{
  const unsigned RCTL = 0x2F5;
  unsigned w;
  asm volatile("mfdcr %0, %1" : "=r"(w) : "i"(RCTL));
  asm volatile("eieio");
  return w;
}

void _set_rctl(unsigned w)
{
  const unsigned RCTL = 0x2F5;
  asm volatile("mtdcr %0, %1" : : "i"(RCTL), "r"(w));
  asm volatile("eieio");
}

void CimConsoleTask::_dumpUserPorts(AbsConsole& o) const
{
  static const char* ttl_format = "%5s %12s %5s %4s %16s %16s\n";
  o.printv(ttl_format,"Port","  Port  ","Link" ,"Link","   Rx"  ,"   Tx"  );
  o.printv(ttl_format,"Name","Location","Speed"," Up" ,"  Bytes","  Bytes");
  o.printv(ttl_format,"----","--------","-----","----","-------","-------");

  static const char* dat_format = "%5s %12s %5s %4s %16lld %16lld\n";
  static const char* PortSpeedString[] = { "None", "10Mb", "100Mb", "1Gb", "2.5Gb", "4Gb", "10Gb" };

  for(unsigned k=0; k<32; k++) {
    try {
      PortDescription d = PortMap::base_map()->port_desc(k);
      unsigned long long rx_bytes = SwmMibCounter((new(0) FmTahoe)->port_mib[d.port()].rxGoodOctets);
      unsigned long long tx_bytes = (unsigned(d.port())==0) ? SwmMibCounter(0) :
        SwmMibCounter((new(0) FmTahoe)->epl_phy [unsigned(d.port())-1].stat_tx_bytecount);
      o.printv(dat_format, d.name(), d.location(),
               PortSpeedString[_mgr.deviceMgr(0).portSpeed(d.port())],
               _mgr.deviceMgr(0).portsUp().contains(d.port()) ? "Up" : "Down",
               rx_bytes, tx_bytes);
    } catch (tool::exception::Event& e) {}
  }

  for(unsigned k=0; k<32; k++) {
    try {
      PortDescription d = PortMap::fabric_map()->port_desc(k);
      unsigned long long rx_bytes = SwmMibCounter((new(1) FmTahoe)->port_mib[d.port()].rxGoodOctets);
      unsigned long long tx_bytes = (unsigned(d.port())==0) ? SwmMibCounter(0) :
        SwmMibCounter((new(1) FmTahoe)->epl_phy [unsigned(d.port())-1].stat_tx_bytecount);
      o.printv(dat_format, d.name(), d.location(),
               PortSpeedString[_mgr.deviceMgr(1).portSpeed(d.port())],
               _mgr.deviceMgr(1).portsUp().contains(d.port()) ? "Up" : "Down",
               rx_bytes, tx_bytes);
    } catch (tool::exception::Event& e) {}
  }
}


static const char* line_sep = "---------------------";

#define _enabled(p) fm.deviceMgr(1).portSpeed(FmPort(p))!=cm::fm::None

typedef struct {
  const char* name;
  unsigned    port;
} DtmInterface_t;

static DtmInterface_t _interfaces[] = { { "sl 1/2", 17 },
                                        { "sl 3"  , 21 },
                                        { "sl 4"  , 19 },
                                        { "sl 5"  , 13 },
                                        { "sl 6"  , 15 },
                                        { "sl 7"  , 11 },
                                        { "sl 8"  ,  9 },
                                        { "sl 9"  ,  7 },
                                        { "sl 10" ,  5 },
                                        { "sl 11" ,  3 },
                                        { "sl 12" ,  1 },
                                        { "sl 13" ,  2 },
                                        { "sl 14" ,  4 },
                                        { NULL, 0 } };

void _show_int_brief(cm::net::Manager& fm, AbsConsole& o)
{
  static const char* titles[] = { "interface",
                                  "enabled",
                                  "status",
                                  NULL };

  const FmPortMask up_ports = fm.deviceMgr(1).portsUp();

  for(unsigned i=0; titles[i]!=NULL; i++)
    o.printv("%s%c",titles[i],titles[i+1] ? '\t':'\n');
  for(unsigned i=0; titles[i]!=NULL; i++)
    o.printv("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
  for(unsigned i=0; _interfaces[i].name!=NULL; i++) {
    o.printv("%-*s\t", strlen(titles[0]), _interfaces[i].name);
    o.printv("%*s\t" , strlen(titles[1]), (_enabled(_interfaces[i].port)) ? "T":"F");
    o.printv("%*s\n" , strlen(titles[2]), (up_ports.contains(FmPort(_interfaces[i].port))) ? "UP":"DOWN");
  }
}

void _show_stat_brief(cm::net::Manager& fm, AbsConsole& o)
{
  static const char* titles[] = { "interface",
                                  "in packets",
                                  "out packets",
                                  "in errors",
                                  "out errors",
                                  NULL };

  char* reply = new char[0x80000];
  unsigned reply_len;
  SwmAllPortMibRequest* req = new SwmAllPortMibRequest( &fm.deviceMgr(1).dev() );
  fm.process(*req, (void*)reply, reply_len);
  delete req;
  if (!reply_len) {
    o.printv("Error fetching port statistics\n");
    delete[] reply;
    return;
  }

  const SwmAllPortMibReply& rrep = *(const SwmAllPortMibReply*)reply;

  for(unsigned i=0; titles[i]!=NULL; i++)
    o.printv("%s%c",titles[i],titles[i+1] ? '\t':'\n');
  for(unsigned i=0; titles[i]!=NULL; i++)
    o.printv("%.*s%c",strlen(titles[i]),line_sep,titles[i+1] ? '\t':'\n');
  for(unsigned i=0; _interfaces[i].name!=NULL; i++) {
    const SwmPortMibCounters& c = rrep.result().port(_interfaces[i].port);
    o.printv("%-*s\t", strlen(titles[0]), _interfaces[i].name);
    unsigned long long rxPkts = c.rxUcast + c.rxBcast + c.rxMcast;
    o.printv("%*lld\t" , strlen(titles[1]), rxPkts);
    unsigned long long txPkts = c.txUnicast + c.txBroadcast + c.txMulticast;
    o.printv("%*lld\t" , strlen(titles[2]), txPkts);
    unsigned long long rxErrs = c.rxFcsErrors + c.rxSymbolErrors;
    o.printv("%*lld\t" , strlen(titles[3]), rxErrs);
    unsigned long long txErrs = c.txTimeoutDrops + c.txErrorDrops;
    o.printv("%*lld\n" , strlen(titles[4]), txErrs);
  }
  delete[] reply;
}

void _z2scan(cm::net::Manager& fm, AbsConsole& o, const char* args)
{
  unsigned speed  = strtoul(args=strtok(0," "),NULL,16);
  unsigned rxflip = strtoul(args=strtok(0," "),NULL,16);
  unsigned txflip = strtoul(args=strtok(0," "),NULL,16);

  FmPortMask mask(0);
  std::list<int>* ports = new std::list<int>;
  while( (args=strtok(0," ")) ) {
    unsigned p = strtoul(args,NULL,16);
    ports->push_back(p);
    mask |= 1<<p;
  }

  for(unsigned k=0; k<=FmPort::MAX_PORT; k++) {
    unsigned m;
    if (mask.contains(FmPort(k))) {
      for(std::list<int>::const_iterator it=ports->begin(); it!=ports->end(); it++) {
        if (*it == k) {
          if (++it == ports->end())
            m = 1<<*(ports->begin());
          else
            m = 1<<*it;
          break;
        }
      }
    }
    else
      m = unsigned(FmPortMask::allPorts()) ^ unsigned(mask);

    o.printv("port %x -> %05x\n",k,m);
    fm.deviceMgr(1).dev().port_cfg_2[k] = FmPortMask(m);
  }

  for(std::list<int>::const_iterator it=ports->begin(); it!=ports->end(); it++) {
    FmPort port(*it);
    //    cm::fm::PortSpeed speed = CimFm::X10Gb;
    fm.deviceMgr(1).configurePort(port,(cm::fm::PortSpeed)speed,rxflip,txflip,0);
  }

  delete ports;
}
