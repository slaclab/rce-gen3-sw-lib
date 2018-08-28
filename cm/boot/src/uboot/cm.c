//#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include "bsi/Bsi_Cfg.h"
#include "bsi/CMB_switch_cfg.h"
#include "bsi/Bsi.h"
#include "boot/cm.h"
#include "../FmTahoe_init.h"
#include "../FmBali_init.h"
#include "../ClusterIpInfo.h"

extern unsigned long get_timer(unsigned long);
extern int printf(const char* fmt,...);

/* control verbosity here */
//#define DBUG
//#define PROFILE

#define PLX_BASEADDR  0xBC001000

//  BSI Fabric Map bit shifts,masks
enum { s_slot=0, s_chan=4, s_port=8, s_stat=13 };
enum { m_slot=(1<<(s_chan-s_slot))-1 };
enum { m_chan=(1<<(s_port-s_chan))-1 };
enum { m_port=(1<<(s_stat-s_port))-1 };
enum { m_stat=(1<<(16-s_stat))-1 };
enum { ok_stat=0x4 };
enum { FABRIC_MAP_WORDS=8 };  // 32b words per slot
enum { FABRIC_MAP_SLOTS=16 };

void cm_net_init(Bsi bsi)
{
  uint32_t data;
  unsigned i,j;

#ifdef PROFILE
  unsigned long time = get_timer(0);
  FmConfig* config = FmConfig_init();
  time = get_timer(time);
  printf("FmConfig_init %ld ms\n",time);

  time = get_timer(0);
  plx* _plx = plx_init(PLX_BASEADDR);
  time = get_timer(time);
  printf("plx_init %ld ms\n",time);

  time = get_timer(0);
#else
  FmConfig* config = FmConfig_init();
  plx* _plx = plx_init(PLX_BASEADDR);
#endif
  const unsigned _fp = ((1<<23)|(1<<24));
  //  unsigned remaining = ((1<<MAX_PORT)-1)<<1;
  unsigned remaining = _fp | (1<<1); // Add the DTM port
  do {
    data = BsiRead32(bsi,BSI_FIFO_OFFSET);
    if(!BSI_FIFO_GET_VALID(data)) continue;

    uint32_t addr = BSI_FIFO_GET_ADDR(data)>>2; /* word offset */
    uint32_t val  = BsiRead32(bsi,addr);

    if (!(addr >= BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET &&
          addr <  BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET+MAX_PORT))
      continue;

    unsigned port = addr - BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET;
    unsigned portMask = 1<<(port+1);
    unsigned speed = val&0xff;
    unsigned trunk = (val>>16)&0xff;

    remaining &= ~portMask;

#ifdef DBUG
    if (val)
      printf("port %d  val 0x%x addr 0x%x\n",(int)port,(int)val,(int)addr);
#endif

    switch(speed) {
    case CMB_SWITCH_PORT_TYPE_1000BASEX_LANE3 :
      config->portCfg.portsRxFlip |= portMask;
      config->portCfg.portsTxFlip |= portMask;
    case CMB_SWITCH_PORT_TYPE_1000BASEX_LANE0 :
      config->portCfg.ports1Gb |= portMask;
      break;
    case CMB_SWITCH_PORT_TYPE_XAUI :
      config->portCfg.ports10Gb |= portMask;
      break;
    default:
      break;
    }

    if (trunk)
      config->swCfg.trunk[trunk-1] |= portMask;

  } while (remaining);

  //  Get Fabric map to calculate port forwarding
  //    Need to get channel map
  //    Need to determine if we are a hub in dual star or not

  unsigned lslot = 0;
  while(!(lslot&0x80000000))
    lslot = BsiRead32(bsi,BSI_FABRIC_MAP_OFFSET + 
                      FABRIC_MAP_WORDS*FABRIC_MAP_SLOTS);

  unsigned fp_hub = lslot&0x40000000;

  lslot &= 0xf;

  unsigned node_slots=0, hub_slots=0;
  unsigned zone2=0;

  { Offset offset = BSI_FABRIC_MAP_OFFSET;
    for(i=0; i<FABRIC_MAP_SLOTS; i++) {
      unsigned mslot=0;
      for(j=0; j<FABRIC_MAP_WORDS; j++,offset++) {
        uint32_t v = BsiRead32(bsi, offset);
        if (((v>>13)&0x7)==0x4) { // Done|Error|MBZ
          unsigned port  = (v>>8)&0x1f;
          unsigned dslot = (v>>0)&0xf;
          if (port < MAX_PORT) {
            mslot |= (1<<dslot);
            if (i==lslot)
              zone2 |= (2<<port); // 1-based port numbers
          }
        }
        v >>= 16;
        if (((v>>13)&0x7)==0x4) { // Done|Error|MBZ
          unsigned port  = (v>>8)&0x1f;
          unsigned dslot = (v>>0)&0xf;
          if (port < MAX_PORT) {
            mslot |= (1<<dslot);
            if (i==lslot)
              zone2 |= (2<<port); // 1-based port numbers
          }
        }
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

  //  bool lmesh = node_slots==0 || hub_slots==0;
  //  if (lmesh) {
  if (node_slots==0 || hub_slots==0) {
    //
    //  Don't forward from zone2 to zone2 in a full mesh
    //
    for(i=0; i<MAX_PORT; i++)
      if (zone2&(1<<(i+1)))
        config->swCfg.portfwd_table.forward[i] = ALL_PORTS&~zone2;
  }
  else if (hub_slots & (1<<lslot)) {

    for(i=0; i<MAX_PORT; i++)
      config->swCfg.portfwd_table.forward[i] = ALL_PORTS;

    //
    //  Don't forward from (other) hub to zone2
    //
    Offset offset = BSI_FABRIC_MAP_OFFSET + FABRIC_MAP_WORDS*lslot;
    for(j=0; j<FABRIC_MAP_WORDS; j++, offset++) {
      uint32_t v = BsiRead32(bsi, offset);
      if (((v>>s_stat)&m_stat)==ok_stat) { // Done|Error|MBZ
        unsigned port  = (v>>s_port)&m_port;
        unsigned dslot = (v>>s_slot)&m_slot;
        if (hub_slots & (1<<dslot))
          config->swCfg.portfwd_table.forward[port] &= ~zone2;
      }
      v >>= 16;
      if (((v>>s_stat)&m_stat)==ok_stat) { // Done|Error|MBZ
        unsigned port  = (v>>s_port)&m_port;
        unsigned dslot = (v>>s_slot)&m_slot;
        if (hub_slots & (1<<dslot))
          config->swCfg.portfwd_table.forward[port] &= ~zone2;
      }
    }
  }
  else {
    //
    //  For nodes in a dual star, trunk together the hub connections
    //
    memset(config->swCfg.trunk, 0, Trunks*sizeof(uint32_t));
    config->swCfg.trunk[Trunks-1] |= zone2;
  }

  //
  //  Front panel forwarding
  //
  for(i=0; i<MAX_PORT; i++) {
    //
    //  Don't forward from fp to fp
    //
    if (_fp&(1<<(i+1)))
      config->swCfg.portfwd_table.forward[i] &= ~_fp;
    if (!fp_hub) {
      //
      // Don't forward from zone2 to fp unless a hub
      //
      if (zone2&(1<<(i+1)))
        config->swCfg.portfwd_table.forward[i] &= ~_fp;
      //
      //  Don't forward from fp to zone2 unless a hub
      //
      if (_fp&(1<<(i+1)))
        config->swCfg.portfwd_table.forward[i] &= ~zone2;
    }
  }

  //
  //  VLAN setup (for ingress/egress frames on external ports)
  //
  ClusterIpInfo* info = ClusterIpInfo_init(bsi);
  
  //
  //  Per port configuration
  //
  if (info->_vlan & Vlan_use) {
    unsigned vlanId = info->_vlan & VlanID_mask;
    if (info->_ingress_rules & RejectUntagged)     // require tagged
      config->swCfg.port_vlan.tagged     |=  _fp;
    if (info->_ingress_rules & RejectTagged)       // require untagged
      config->swCfg.port_vlan.untagged   |=  _fp;
    if (info->_ingress_rules & RejectBV)     // filter ingress boundary violations
      config->swCfg.port_vlan.strict     |=  _fp;

    for(i=0; i<MAX_PORT; i++)
      config->swCfg.port_vlan.defaultVlan[i] = vlanId;

    //
    //  Apply ingress tag rule to egress policy
    //
    for(i=0; i<Entries; i++)
      SwmVlanTableEntry_init(&config->swCfg.vid_table[i],0,0,0,0,0);

    SwmVlanTableEntry_init(&config->swCfg.vid_table[vlanId],
                           ALL_PORTS,
                           (info->_ingress_rules&RejectUntagged) ? _fp : 0,
                           0, vlanId&0x1f, 0);
    free(info);
  }

#ifdef PROFILE
  time = get_timer(time);
  printf("bsi fetch %ld ms\n",time);
#endif

  if (!FmTahoe_init(_plx,config))
    if (!FmBali_init(_plx,config))
      ;
}
