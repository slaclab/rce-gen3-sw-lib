
#include "fm/SwmTrigger.hh"

#include "fm/FmTahoe.hh"

#include <stdio.h>
#include <new>

SwmTrigger::SwmTrigger(unsigned w0,
                       unsigned w1,
                       unsigned w2,
                       unsigned w3)
{
  _data[0] = w0;
  _data[1] = w1;
  _data[2] = w2;
  _data[3] = w3;
}

unsigned SwmTrigger::id          () const { return (_data[0]>>28)&0x0f; }
unsigned SwmTrigger::new_priority() const { return (_data[0]>>24)&0x0f; }
unsigned SwmTrigger::mirror_port () const { return (_data[0]>>19)&0x1f; }
unsigned SwmTrigger::action      () const { return (_data[0]>>16)&0x07; }

bool SwmTrigger::enabled         () const { return !(src_miss()&&src_hit()); }
bool SwmTrigger::src_dst_match () const { return _data[0]&(1<<11); }
bool SwmTrigger::priority_match() const { return _data[0]&(1<<10); }
bool SwmTrigger::mcast         () const { return _data[0]&(1<< 9); }
bool SwmTrigger::bcast         () const { return _data[0]&(1<< 8); }
bool SwmTrigger::ucast         () const { return _data[0]&(1<< 7); }
bool SwmTrigger::vlan          () const { return _data[0]&(1<< 6); }
bool SwmTrigger::dst_port      () const { return _data[0]&(1<< 5); }
bool SwmTrigger::src_port      () const { return _data[0]&(1<< 4); }
bool SwmTrigger::dst_miss      () const { return _data[0]&(1<< 3); }
bool SwmTrigger::dst_hit       () const { return _data[0]&(1<< 2); }
bool SwmTrigger::src_miss      () const { return _data[0]&(1<< 1); }
bool SwmTrigger::src_hit       () const { return _data[0]&(1<< 0); }

unsigned SwmTrigger::priority  () const { return (_data[1]>>0)&0xffff; }
unsigned SwmTrigger::src_ports () const { return (_data[2]>>0)&0x1fffffe; }
unsigned SwmTrigger::dst_ports () const { return (_data[3]>>0)&0x1fffffe; }

static const char* headers[] = {"Id",
                                "New Priority",
                                "Mirror Port",
                                "Action",
                                "Src/Dst Match",
                                "Prio Match",
                                "McastOnly",
                                "BcastOnly",
                                "UcastOnly",
                                "VLAN",
                                "Dst Port",
                                "Src Port",
                                "Dst Miss",
                                "Dst Hit",
                                "Src Miss",
                                "Src Hit",
                                "Priority",
                                "Src Ports",
                                "Dst Ports",
                                0 };

const char* SwmTrigger::header(unsigned i) { return headers[i]; }


typedef const char* (FieldFunc)(const SwmTrigger&);

static char _buff[32];

#define _PUT(fname,fmt,f) \
  static const char* _##fname(const SwmTrigger& t) \
  { sprintf(_buff,fmt,t.f()); return _buff; }

_PUT(id        ,"%02d",id)
_PUT(new_prio  ,"%02d",new_priority)
_PUT(mirr_port ,"%02d",mirror_port)
_PUT(priority  ,"%02d",priority)
_PUT(src_ports ,"%07x",src_ports)
_PUT(dst_ports ,"%07x",dst_ports)

#undef _PUT
#define _PUT(fname,func) \
  static const char* _##fname(const SwmTrigger& t)       \
  { sprintf(_buff,"%c",t.func()?'Y':'N'); return _buff; }

_PUT(src_dst,src_dst_match)
_PUT(prio_match,priority_match)
_PUT(mcast,mcast)
_PUT(bcast,bcast)
_PUT(ucast,ucast)
_PUT(vlan ,vlan)
_PUT(dst_port,dst_port)
_PUT(src_port,src_port)
_PUT(dst_miss,dst_miss)
_PUT(dst_hit ,dst_hit )
_PUT(src_miss,src_miss)
_PUT(src_hit ,src_hit )

#undef _PUT

  static const char* _actions[] = { "Fwd",
                                    "Redir",
                                    "Mirr",
                                    "Dsc",
                                    "FwdP",
                                    "RedirP",
                                    "MirrP",
                                    NULL };

  static const char* _action(const SwmTrigger& t)
{ sprintf(_buff,"%6.6s",_actions[t.action()]); return _buff; }

static FieldFunc* fields[] = {
  _id,
  _new_prio,
  _mirr_port,
  _action,
  _src_dst,
  _prio_match,
  _mcast,
  _bcast,
  _ucast,
  _vlan,
  _dst_port,
  _src_port,
  _dst_miss,
  _dst_hit,
  _src_miss,
  _src_hit,
  _priority,
  _src_ports,
  _dst_ports,
  NULL };

const char* SwmTrigger::field(unsigned i) const
{ return fields[i](*this); }

void SwmTriggerSet::print() const
{
  for(unsigned i=0; headers[i]!=NULL; i++) {
    printf("%14.14s",headers[i]);
    for(unsigned j=0; j<16; j++)
      printf("%s",fields[i](_triggers[j]));
    printf("\n");
  }
}

SwmTriggerSet::SwmTriggerSet(FmTahoe& t)
{
  for(unsigned i=0; i<16; i++)
    new(&_triggers[i]) SwmTrigger(t.trigger_cfg[i],
                                  t.trigger_pri[i],
                                  t.trigger_rx [i],
                                  t.trigger_tx [i]);
}
