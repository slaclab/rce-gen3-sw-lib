#ifndef Bali_SwmIngressVlanTableEntry_hh
#define Bali_SwmIngressVlanTableEntry_hh

namespace Bali {
  class FmIngressVlanTableEntry;

  class SwmIngressVlanTableEntry {
  public:
    SwmIngressVlanTableEntry() {}
    SwmIngressVlanTableEntry(unsigned fid,
                             unsigned port_membership_mask,
                             unsigned trig_id,
                             unsigned counter, 
                             unsigned reflect,
                             unsigned trap_igmp);
  
    SwmIngressVlanTableEntry& operator=(const SwmIngressVlanTableEntry&);

    bool          empty  () const;
    unsigned      members() const;
    unsigned      tagged () const;
    unsigned      portIsMember (unsigned) const;
    unsigned      counter() const;
    unsigned      reflect() const;

    void          print() const;
  private:
    unsigned _data[4];
    friend class FmIngressVlanTableEntry;
  };
};

inline bool Bali::SwmIngressVlanTableEntry::empty() const
{
  return (_data[1]&0xaaaaaaaa)==0 && (_data[0]&0xaaaa8000)==0;
}

inline unsigned Bali::SwmIngressVlanTableEntry::portIsMember(unsigned iport) const
{
  return ((iport<9) ? (_data[0] >> (iport*2+14)) : _data[1] >> (iport*2-18))&2;
}

inline unsigned Bali::SwmIngressVlanTableEntry::counter() const
{
  return (_data[0]>>2)&0x3f;
}

inline unsigned Bali::SwmIngressVlanTableEntry::reflect() const
{
  return _data[0]&2;
}

#endif
