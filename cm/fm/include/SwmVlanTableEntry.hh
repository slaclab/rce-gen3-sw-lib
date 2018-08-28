#ifndef SwmVlanTableEntry_hh
#define SwmVlanTableEntry_hh

class FmVlanTableEntry;
namespace Bali { class FmIngressVlanTableEntry; };

class SwmVlanTableEntry {
public:
  SwmVlanTableEntry() {}
  SwmVlanTableEntry(unsigned port_membership_mask, unsigned port_tag_mask,
		    unsigned trig_id, unsigned counter, unsigned reflect);
  
  SwmVlanTableEntry& operator=(const SwmVlanTableEntry&);

  bool          empty  () const;
  unsigned      members() const;
  unsigned      tagged () const;
  unsigned      portIsMember (unsigned) const;
  unsigned      portIsTagging(unsigned) const;
  unsigned      trigger() const;
  unsigned      counter() const;
  unsigned      reflect() const;

  void          print() const;
private:
  unsigned _data[2];
  friend class FmVlanTableEntry;
  friend class Bali::FmIngressVlanTableEntry;
};

inline bool SwmVlanTableEntry::empty() const
{
  return (_data[0]&0xaaaaaaaa)==0 && (_data[1]&0x2aaaa)==0;
}

inline unsigned SwmVlanTableEntry::portIsMember(unsigned iport) const
{
  return ((iport<9) ? (_data[0] >> (iport*2+14)) : _data[1] >> (iport*2-18))&2;
}

inline unsigned SwmVlanTableEntry::portIsTagging(unsigned iport) const
{
  return ((iport<9) ? (_data[0] >> (iport*2+14)) : _data[1] >> (iport*2-18))&1;
}

inline unsigned SwmVlanTableEntry::trigger() const
{
  return (_data[0]>>8)&0x1f;
}

inline unsigned SwmVlanTableEntry::counter() const
{
  return (_data[0]>>2)&0x3f;
}

inline unsigned SwmVlanTableEntry::reflect() const
{
  return _data[0]&2;
}

#endif
