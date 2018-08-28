#ifndef Bali_SwmEgressVlanTableEntry_hh
#define Bali_SwmEgressVlanTableEntry_hh

namespace Bali {
  class FmEgressVlanTableEntry;

  class SwmEgressVlanTableEntry {
  public:
    SwmEgressVlanTableEntry() {}
    SwmEgressVlanTableEntry(unsigned fid,
                            unsigned mtu_index,
                            unsigned port_membership_mask,
                            unsigned trig_id);
  
    SwmEgressVlanTableEntry& operator=(const SwmEgressVlanTableEntry&);

    bool          empty   () const;
    unsigned      members () const;
    unsigned      portIsMember (unsigned) const;
    unsigned      mtuIndex() const;
    unsigned      fid     () const;
    unsigned      trigger () const;

    void          print() const;
  private:
    unsigned _data[2];
    friend class FmEgressVlanTableEntry;
  };
};

inline bool Bali::SwmEgressVlanTableEntry::empty() const
{
  return (_data[0]&0x1ffffff0)==0;
}

inline unsigned Bali::SwmEgressVlanTableEntry::members() const
{
  return (_data[0] >> 4) & 0x1ffffff;
}

inline unsigned Bali::SwmEgressVlanTableEntry::portIsMember(unsigned iport) const
{
  return (_data[0] >> (iport+4))&1;
}

inline unsigned Bali::SwmEgressVlanTableEntry::mtuIndex() const
{
  return (_data[0]>>1)&0x7;
}

inline unsigned Bali::SwmEgressVlanTableEntry::fid() const
{
  return _data[1]&0xfff;
}

inline unsigned Bali::SwmEgressVlanTableEntry::trigger() const
{
  return (_data[1]>>12)&0x3f;
}

#endif
