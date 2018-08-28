/*
**  Daemon process for controlling Fulcrum switches
**
*/

#include "logger/FormatMessage.hh"

#include "fm/FmTahoe.hh"
#include "fm/FmTahoeMgr.hh"
#include "fmb/FmBali.hh"
#include "fmb/FmBaliMgr.hh"
#include "fm/FmConfig.hh"
#include "fm/FmPort.hh"
#include "TahoeShell.hh"
#include "BaliShell.hh"
#include "DtmInterface.hh"
#include "cm_svc/ClusterIpInfo.hh"
#include "cm_svc/Print.hh"

#include <string>
#include <vector>
#include <sstream>

#include <getopt.h>

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdarg.h>

extern "C" {
#include "bsi/CMB_switch_cfg.h"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"
#include "map/MapBsi.h"
}

//
//  BSI Fabric Map bit shifts,masks
//
enum { s_slot=0, s_chan=4, s_port=8, s_stat=13 };
enum { m_slot=(1<<(s_chan-s_slot))-1 };
enum { m_chan=(1<<(s_port-s_chan))-1 };
enum { m_port=(1<<(s_stat-s_port))-1 };
enum { m_stat=(1<<(16-s_stat))-1 };
enum { ok_stat=0x4 };
enum { MAX_PORT=FmPort::MAX_PORT, ALL_PORTS=(2<<MAX_PORT)-1 };
enum { FABRIC_MAP_WORDS=8 };  // 32b words per slot
enum { FABRIC_MAP_SLOTS=16 };

void usage(const char* p)
{
  printf("Usage: %s [options]\n",p);
  printf("options: -m\tLoad configuration from BSI memory\n"
         "         -r\tReset switch\n"
         "         -b\tPoll BSI for updates\n"
         "         -t\tStart telnet server (port 30000)\n"
         "         -B <poll period(msec)>\tPeriod for BSI poll\n"
         "         -v\tverbose\n");
  exit(1);
}

class DtmEnv;

static void   load_bsi(DtmEnv* env);
static void*  poll_bsi(void*);
static void   sigintHandler(int);
static      FmTahoeMgr* _tm=0;
static Bali::FmBaliMgr* _bm=0;
static bool   lverbose=false;
static bool   lfifo=false;
static FmConfig _config;
static unsigned _zone2 = 0, _fp = ((1<<23) | (1<<24));
static unsigned _bsi_poll_period=100000;
static DtmInterface __interfaces[] = 
  { { "slot2" , 13 },
    { "slot3" , 14 },
    { "slot4" , 15 },
    { "slot5" , 16 },
    { "slot6" , 17 },
    { "slot7" , 18 },
    { "slot8" , 19 },
    { "slot9" , 20 },
    { "slot10", 21 },
    { "slot11", 22 },
    { "slot12", 10 },
    { "slot13", 11 },
    { "slot14", 12 },
    { "", 0 },
    { "", 0 },
    { "", 0 },
    { "dpm0.0", 2  },
    { "dpm0.1", 3  },
    { "dpm1.0", 4  },
    { "dpm1.1", 5  },
    { "dpm2.0", 6  },
    { "dpm2.1", 7  },
    { "dpm3.0", 8  },
    { "dpm3.1", 9  },
    { "", 0 },
    { "", 0 },
    { "", 0 },
    { "", 0 },
    { "fp_sfp", 23 },
    { "fp_sfp+",24 },
    { "", 0 },
    { "dtm"   , 1  },
    { "mgmt"  , 0  }};


class DtmEnv {
public:
  DtmEnv() : bsi (LookupBsi())
  {
    if (lverbose) {
      printf("Mapped BSI to 0x%x\n",(int)bsi);
    }
  }
  Bsi  bsi;
};

int main(int argc, char** argv)
{
  extern char* optarg;
  bool lteln=false;
  bool lreadfrombsi=false;
  bool lreset=false;
  bool lconfig=false;
  bool lpollbsi=false;
  int c;
  while ( (c=getopt( argc, argv, "mrctvbB:h")) != EOF ) {
    switch(c) {
    case 't': lteln =true; break;
    case 'v': lverbose=true; break;
    case 'm': lreadfrombsi=true; break;
    case 'r': lreset=true; break;
    case 'c': lconfig=true; break;
    case 'b': lpollbsi=true; break;
    case 'B': _bsi_poll_period = strtoul(optarg,NULL,0)*1000; break;
    case 'h':
    case '?': 
    default : usage(argv[0]); break;
    }
  }

  DtmEnv env;

  if (lreadfrombsi) {
    //
    //  Load _config from bsi
    //
    load_bsi   (&env);
  }

  if (lreset) {
    //
    //  Reset and configure 
    //
    _tm = new FmTahoeMgr(*new(0)FmTahoe,_config);
    try                    { _tm->reset(_config); }
    catch (std::string& e) { delete _tm; _tm=0; }
    if (!_tm) { // Tahoe failed
      _bm = new Bali::FmBaliMgr(*new(0)Bali::FmBali,_config);
      try                    { _bm->reset(_config); }
      catch (std::string& e) { delete _bm; _bm=0; }
      if (!_bm)
        exit(1);
    }
  }
  else {
    //
    //  Discover the chip and configure
    //
    _tm = new FmTahoeMgr(*new(0)FmTahoe,_config);
    switch ((unsigned(_tm->dev().vpd)>>12)&0xffff) {
    case 0xae18:  // Tahoe
      if (lconfig) _tm->configure(_config);
      break;
    case 0xae19:  // Bali
      delete _tm; _tm=0;
      _bm = new Bali::FmBaliMgr(*new(0)Bali::FmBali,_config);
      if (lconfig) _bm->configure(_config);
      break;
    default:
      exit(1);
    }
  }

  pthread_t id;
  pthread_attr_t flags;
  size_t min_stack,stacksize=0x10000;
  pthread_attr_init(&flags);
  pthread_attr_setdetachstate(&flags, PTHREAD_CREATE_DETACHED);
  pthread_attr_getstacksize(&flags, &min_stack);
  if (stacksize < min_stack) stacksize = min_stack;
  pthread_attr_setstacksize(&flags, stacksize);

  if (lteln) {
    //
    //  Start telnet server
    //
    if (_tm)
      pthread_create(&id, &flags, cm::shell::initialize, 
                     new TahoeShell(_config,__interfaces));
    else if (_bm)
      pthread_create(&id, &flags, cm::shell::initialize, 
                     new BaliShell(_config,__interfaces));
  }

  if (lpollbsi) {
    pthread_create(&id, &flags, poll_bsi, &env);
  }

  // Unix signal support
  struct sigaction int_action;

  int_action.sa_handler = sigintHandler;
  sigemptyset(&int_action.sa_mask);
  int_action.sa_flags = 0;
  int_action.sa_flags |= SA_RESTART;

  if (sigaction(SIGINT, &int_action, 0) > 0)
    printf("Couldn't set up SIGINT handler\n");
  if (sigaction(SIGKILL, &int_action, 0) > 0)
    printf("Couldn't set up SIGKILL handler\n");
  if (sigaction(SIGSEGV, &int_action, 0) > 0)
    printf("Couldn't set up SIGSEGV handler\n");
  if (sigaction(SIGABRT, &int_action, 0) > 0)
    printf("Couldn't set up SIGABRT handler\n");
  if (sigaction(SIGTERM, &int_action, 0) > 0)
    printf("Couldn't set up SIGTERM handler\n");

  if (!lverbose) {
    /* Close the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO); 
  } 

  sem_t _sem;
  sem_init(&_sem, 0, 0);
  sem_wait(&_sem);

  sigintHandler(0);
  return 0;
}

static const char* speedstr[] = { "NC","DIS","1GB","1GBL1","1GBL2","1GB-R","XAUI" };

static void setup_interface(unsigned  v, 
                            bool      lslot,
                            unsigned& mslot, 
                            unsigned& zone2)
{
  if (((v>>s_stat)&m_stat)==ok_stat) { // Done|Error|MBZ
    unsigned port  = (v>>s_port)&m_port;
    unsigned dslot = (v>>s_slot)&m_slot;
    unsigned dchan = (v>>s_chan)&m_chan;
    if (port < MAX_PORT) {
      mslot |= (1<<dslot);
      if (lslot) {
        zone2 |= (2<<port); // 1-based port numbers
        for(unsigned k=0; k<32; k++)
          if (__interfaces[k].port==port+1) {
            std::stringstream o;
            o << "lslot" << dslot+1
              << "-" << dchan+1;
            __interfaces[k].name = o.str();
            break;
          }
      }
    }
  }
}

void  load_bsi(DtmEnv* env)
{
  memset(&_config.portCfg,0,sizeof(_config.portCfg));
  memset(&_config.swCfg.trunk[0],0,FmSwitchConfig::Trunks*sizeof(FmPortMask));

  if (lfifo) {
    // Drain the FIFO
    uint32_t data = BsiRead32(env->bsi,BSI_FIFO_OFFSET);
    unsigned n = 0;
    while(BSI_FIFO_GET_VALID(data)) {
      data = BsiRead32(env->bsi,BSI_FIFO_OFFSET);
      n++;
    }
    if (lverbose)
      cm::service::printv("Drained %d entries from BSI FIFO\n",n);
  }


  if (lverbose)
    cm::service::printv("PORT  TYPE   GROUP\n");

  // Read from RAM
  for(unsigned port = 0; port<MAX_PORT; port++) {
    
    unsigned val = BsiRead32(env->bsi,BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + port);

    unsigned speed    = val&0xff;
    unsigned trunk    = (val>>16)&0xff;
    unsigned portMask = 1<<(port+1);

    switch(speed) {
    case CMB_SWITCH_PORT_TYPE_1000BASEX_LANE3 :
      _config.portCfg.portsRxFlip |= portMask;
      _config.portCfg.portsTxFlip |= portMask;
    case CMB_SWITCH_PORT_TYPE_1000BASEX_LANE0 :
      _config.portCfg.ports1Gb |= portMask;
      break;
    case CMB_SWITCH_PORT_TYPE_XAUI :
      _config.portCfg.ports10Gb |= portMask;
      break;
    default:
      break;
    }

    if (trunk)
      _config.swCfg.trunk[trunk-1] |= portMask;

    if (lverbose && speed >= CMB_SWITCH_PORT_TYPE_1000BASEX_LANE0)
      cm::service::printv("%4d  %5.5s  %d\n",port,speedstr[speed],trunk);
  }

  //  Get Fabric map to calculate port forwarding
  //    Need to get channel map
  //    Need to determine if we are a hub in dual star or not

  if (lverbose) {  // dump the whole thing
    Offset offset = BSI_FABRIC_MAP_OFFSET;
    for(unsigned i=0; i<FABRIC_MAP_SLOTS; i++) {
      for(unsigned j=0; j<FABRIC_MAP_WORDS; j++,offset++) {
        uint32_t v = BsiRead32(env->bsi, offset);
        cm::service::printv(" %04x %04x", v&0xffff, v>>16);
      }
      cm::service::printv("\n");
    }
    uint32_t v = BsiRead32(env->bsi, offset);
    cm::service::printv("[%08x]\n",v);
  }

  unsigned lslot = 0;
  while(!(lslot&0x80000000))
    lslot = BsiRead32(env->bsi,BSI_FABRIC_MAP_OFFSET + 
                      FABRIC_MAP_SLOTS*FABRIC_MAP_WORDS);
  
  //  bool fp_hub = lslot&0x40000000;
  bool fp_hub = true;

  lslot &= 0xf;

  unsigned node_slots=0, hub_slots=0;
  unsigned zone2(0);

  { Offset offset = BSI_FABRIC_MAP_OFFSET;
    for(unsigned i=0; i<FABRIC_MAP_SLOTS; i++) {
      unsigned mslot=0;
      for(unsigned j=0; j<FABRIC_MAP_WORDS; j++,offset++) {
        uint32_t v = BsiRead32(env->bsi, offset);
        setup_interface(v>> 0,i==lslot,mslot,zone2);
        setup_interface(v>>16,i==lslot,mslot,zone2);
      }
      
      //  Require at least three interconnects for a potential hub
      unsigned hslot=mslot;
      hslot &= (hslot-1);
      hslot &= (hslot-1);
      hslot &= (hslot-1);
      if (hslot)
        hub_slots  |= 1<<i;
      else if (mslot)
        node_slots |= 1<<i;
    }
  }

  _zone2 = zone2;

  if (lverbose) {
    cm::service::printv("fmd lslot %d: zone 2 %08x  hubs %08x  nodes %08x\n",
                        lslot, zone2, hub_slots, node_slots);
    for(unsigned i=0; i<32; i++)
      if (!__interfaces[i].name.empty())
        cm::service::printv("%s [%d]\n",
                            __interfaces[i].name.c_str(),
                            __interfaces[i].port);
  }

  bool lmesh = node_slots==0 || hub_slots==0;
  if (lmesh) {
    for(unsigned i=0; i<MAX_PORT; i++) {
      unsigned mask = unsigned(FmPortMask::allPorts());
      //
      //  Don't forward from zone2 to zone2 in a full mesh
      //
      if (zone2&(1<<(i+1)))
        mask &= ~zone2;

      _config.swCfg.portfwd_table.forward[i] = FmPortMask(mask);
    }
  }
  else if (hub_slots & (1<<lslot)) {

    for(unsigned i=0; i<MAX_PORT; i++)
      _config.swCfg.portfwd_table.forward[i] = FmPortMask::allPorts();

    //
    //  Don't forward from (other) hub to zone2
    //
    Offset offset = BSI_FABRIC_MAP_OFFSET + FABRIC_MAP_WORDS*lslot;
    for(unsigned j=0; j<FABRIC_MAP_WORDS; j++, offset++) {
      uint32_t v = BsiRead32(env->bsi, offset);
      if (((v>>s_stat)&m_stat)==ok_stat) { // Done|Error|MBZ
        unsigned port  = (v>>s_port)&m_port;
        unsigned dslot = (v>>s_slot)&m_slot;
        if (hub_slots & (1<<dslot))
          _config.swCfg.portfwd_table.forward[port] &= ~zone2;
      }
      v >>= 16;
      if (((v>>s_stat)&m_stat)==ok_stat) { // Done|Error|MBZ
        unsigned port  = (v>>s_port)&m_port;
        unsigned dslot = (v>>s_slot)&m_slot;
        if (hub_slots & (1<<dslot))
          _config.swCfg.portfwd_table.forward[port] &= ~zone2;
      }
    }
  }
  else {
    //
    //  For nodes in a dual star, trunk together the hub connections
    //
    for(unsigned i=0; i<MAX_PORT; i++) {
      unsigned mask = FmPortMask::allPorts();
      if (zone2&(1<<(i+1)))
        mask &= ~zone2;
      _config.swCfg.portfwd_table.forward[i] = FmPortMask(mask);
    }

    memset(_config.swCfg.trunk, 0, FmSwitchConfig::Trunks*sizeof(FmPortMask));
    _config.swCfg.trunk[FmSwitchConfig::Trunks-1] |= zone2;
  }

  //
  //  Front panel forwarding
  //
  for(unsigned i=0; i<MAX_PORT; i++) {
    //
    //  Don't forward from fp to fp
    //
    if (_fp&(1<<(i+1)))
      _config.swCfg.portfwd_table.forward[i] &= ~_fp;
    if (!fp_hub) {
      //
      // Don't forward from zone2 to fp unless a hub
      //
      if (zone2&(1<<(i+1)))
        _config.swCfg.portfwd_table.forward[i] &= ~_fp;
      //
      //  Don't forward from fp to zone2 unless a hub
      //
      if (_fp&(1<<(i+1)))
        _config.swCfg.portfwd_table.forward[i] &= ~zone2;
    }
  }

  //
  //  VLAN setup (for ingress/egress frames on external ports)
  //
  cm::svc::ClusterIpInfo info;
  _config.swCfg.port_vlan = SwmPortVlanConfig(SwmPortVlanConfig::Init);
  
  //
  //  Per port configuration
  //
  if (info.use_vlan()) {
    unsigned vlanId = info.vlan_id();
    if (info.reject_untagged())     // require tagged
      _config.swCfg.port_vlan.tagged     |=  _fp;
    if (info.reject_tagged())     // require untagged
      _config.swCfg.port_vlan.untagged   |=  _fp;
    if (info.reject_boundary_violation())     // filter ingress boundary violations
      _config.swCfg.port_vlan.strict     |=  _fp;

    for(unsigned i=0; i<FmPort::MAX_PORT; i++)
      _config.swCfg.port_vlan.defaultVlan[i] = vlanId;

    //
    //  Apply ingress tag rule to egress policy
    //
    for(unsigned i=0; i<0x1000; i++)
      _config.swCfg.vid_table[i] = SwmVlanTableEntry(0,0,0,0,0);

    _config.swCfg.vid_table[vlanId] = SwmVlanTableEntry(unsigned(FmPortMask::allPorts()),
                                                        info.reject_untagged() ? _fp : 0,
                                                        0, vlanId&0x1f, 0);
  }
  
  if (lverbose) {
    for(unsigned i=0; i<MAX_PORT; i++)
      cm::service::printv("fwd[%d] %08x\n",
                          i, unsigned(_config.swCfg.portfwd_table.forward[i]));
    for(unsigned i=0; i<FmSwitchConfig::Trunks; i++)
      cm::service::printv("trunk[%d] %08x\n",
                          i, unsigned(_config.swCfg.trunk[i]));
  }
}

void* poll_bsi(void* arg)
{
  DtmEnv* env = reinterpret_cast<DtmEnv*>(arg);

  uint32_t val;
  uint32_t addr;

  uint32_t* val_cache = new uint32_t[MAX_PORT];
  memset(val_cache, 0, sizeof(uint32_t)*MAX_PORT);

  while(1) {
    usleep(_bsi_poll_period);

    bool lHeader=false;

    unsigned port=0;
    while(1) {

      if (lfifo) {
        uint32_t data = BsiRead32(env->bsi,BSI_FIFO_OFFSET);
        if (!BSI_FIFO_GET_VALID(data)) break;
        
        addr = BSI_FIFO_GET_ADDR(data) >> 2;
        val  = BsiRead32 (env->bsi,addr);
        data = BsiRead32 (env->bsi,BSI_FIFO_OFFSET);
      }
      else {
        // Read from RAM
        // Mock up as if read from FIFO
        if (port==MAX_PORT) break;

        addr = port + BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET;
        val  = BsiRead32(env->bsi,addr);
        port++;
      }

      unsigned vspd  = val&0xff;
      unsigned trunk = (val>>16)&0xff;

      if (addr >= BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET &&
          addr <  BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET+MAX_PORT) {
        
        unsigned chan = addr - BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET;
        if (val == val_cache[chan]) continue;

        val_cache[chan] = val;
        FmPort sw_port(1+chan);
        cm::fm::PortSpeed speed = cm::fm::None;
        unsigned flip=0;

        if (vspd==CMB_SWITCH_PORT_TYPE_1000BASEX_LANE3) {
          flip = 1;
          speed = cm::fm::X1Gb;
        }
        else if (vspd==CMB_SWITCH_PORT_TYPE_1000BASEX_LANE0)
          speed = cm::fm::X1Gb;
        else if (vspd==CMB_SWITCH_PORT_TYPE_XAUI)
          speed = cm::fm::X10Gb;
        else if (vspd==CMB_SWITCH_PORT_TYPE_NC)
          continue;
        else
          speed = cm::fm::None;

        if (_config.swCfg.trunk[FmSwitchConfig::Trunks-1].contains(sw_port))
          trunk = FmSwitchConfig::Trunks-1;

        if (lverbose) {
          if (!lHeader) {
            lHeader=true;
            cm::service::printv("PORT  TYPE   GROUP\n");
          }
          cm::service::printv("%4d  %5.5s  %d\n",chan,speedstr[vspd],trunk);
        }

        if (_tm) {
          _tm->configurePort(sw_port, speed, flip, flip, 0, trunk);
        }
        else if (_bm) {
          _bm->configurePort(sw_port, speed, flip, flip, 0, trunk);
        }
      }
    }
  }
  return 0;
}

void sigintHandler(int iSignal)
{
  if (lverbose)
    cm::service::printv("fmd caught signal %d\n",iSignal);

  exit(0);
}

