

#include <string.h>
#include <malloc.h>

#include "../FmConfig.h"

void SwmVlanTableEntry_init(SwmVlanTableEntry* this,
                            unsigned port_membership_mask, unsigned port_tag_mask,
                            unsigned trig_id, unsigned counter, unsigned reflect)
{
  unsigned j,p,parity;

  this->_data[0] = (reflect!=0) ? 2 : 0;
  this->_data[0] |= (counter&0x3f)<<2;
  this->_data[0] |= (trig_id&0x1f)<<8;
  this->_data[1] = 0;

  j=0;
  while( j < 9 ) {
    if (port_tag_mask & (1<<j))
      this->_data[0] |= 1<<(2*j+14);
    if (port_membership_mask & (1<<j))
      this->_data[0] |= 1<<(2*j+15);
    j++;
  }
  while( j < 25 ) {
    if (port_tag_mask & (1<<j))
      this->_data[1] |= 1<<(2*j-18);
    if (port_membership_mask & (1<<j))
      this->_data[1] |= 1<<(2*j-17);
    j++;
  }

  parity = reflect ? 1 : 0;
  p = port_membership_mask ^ port_tag_mask ^ (trig_id&0x1f) ^ (counter&0x3f);
  while( p ) {
    if (p&1) parity++;
    p >>= 1;
  }
  this->_data[0] |= parity&1;
}

FmConfig* FmConfig_init(void)
{
  unsigned k;
  FmPortMask empty = (0);
  FmPortMask full  = ((1<<24)-1)<<1;
  FmConfig* config = malloc(sizeof(FmConfig));

  memset(&config->portCfg,0,sizeof(FmPortConfig));
  memset(&config->hostCfg,0,sizeof(FmHostIfConfig));

  for(k=0; k<Entries; k++) {
    config->swCfg.fid_table[k].listening  = empty;
    config->swCfg.fid_table[k].learning   = empty;
    config->swCfg.fid_table[k].forwarding = full;
  }

  for(k=0; k<Entries; k++)
    SwmVlanTableEntry_init(&config->swCfg.vid_table[k], 0, 0, 0, 0, 0);
  SwmVlanTableEntry_init(&config->swCfg.vid_table[1], -1UL, 0, -1UL, -1UL, 0);

  for(k=0; k<0x4000; k++)
    config->swCfg.mac_table.entry[k]._data[2] = 0;

  for(k=0; k<MAX_PORT; k++)
    config->swCfg.portfwd_table.forward[k] = full | 1;

  config->swCfg.port_vlan.strict = 0;
  config->swCfg.port_vlan.tagged = 0;
  config->swCfg.port_vlan.untagged = 0;
  for(k=0; k<MAX_PORT; k++)
    config->swCfg.port_vlan.defaultVlan[k] = 1;

  for(k=0; k<Trunks; k++)
    config->swCfg.trunk[k] = 0;

  memset(&config->trigCfg[0], 0, 16*sizeof(FmTriggerConfig));

  config->protocolCfg.igmpVsn  = NoIGMP;
  config->protocolCfg.garpImpl = NoGARP;
  config->protocolCfg.stpImpl  = NoSTP;

  return config;
}

unsigned SwmVlanTableEntry_members(SwmVlanTableEntry* this)
{
  unsigned j;
  unsigned m = 0;
  unsigned n = 0;
  unsigned v = this->_data[0]>>15;
  for(j=0; j<9; j++,v>>=1)
    m |= v&(1<<j);
  v = this->_data[1]>>1;
  for(j=9; j<=24; j++,v>>=1)
    n |= v&(1<<(j-9));
  return (m|(n<<9));
}
