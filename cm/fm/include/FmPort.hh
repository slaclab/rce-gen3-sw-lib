#ifndef FmPort_hh
#define FmPort_hh

//
//  These classes are defined to clarify the numbering convention
//  used for port ids in function calls.
//

struct ether_addr_s {
  unsigned char ether_addr_octet[6];
};

typedef ether_addr_s ether_addr_t;

namespace cm {
  namespace fm {
    enum PortSpeed { None, X10Mb, X100Mb, X1Gb, X2_5Gb, X4Gb, X10Gb };
  };
};

//
//  class FmPort
//    Port number with the convention that the logical CPU port
//  is at index 0.
//
class FmPort {
public:
  enum { CPU_PORT = 0 };
  enum { MAX_PORT = 24 };
public:
  explicit FmPort(unsigned port) : _port(port) {}

  operator unsigned() const { return _port; }
private:
  unsigned _port;
};

//
//  class FmPortMask
//    Mask of FmPort indices.
//
class FmPortMask {
public:
  explicit FmPortMask(int      mask) : _mask(mask) {}
  explicit FmPortMask(unsigned mask) : _mask(mask) {}
  FmPortMask() : _mask(0) {}
  FmPortMask(const FmPortMask& o) : _mask(o._mask) {}

  operator unsigned() const { return _mask; }

  FmPortMask& operator &=(unsigned p);
  FmPortMask& operator |=(unsigned p);
  FmPortMask& operator |=(const FmPortMask& p);

  bool     contains(FmPort p) const;
public:
  static FmPortMask allPorts() { return FmPortMask((2<<FmPort::MAX_PORT)-1); }
  static FmPortMask phyPorts() { return FmPortMask((2<<FmPort::MAX_PORT)-1-(1<<FmPort::CPU_PORT)); }
private:
  unsigned _mask;
};

inline bool FmPortMask::contains(FmPort p) const 
{
  return _mask & (1<<unsigned(p)); 
}

inline FmPortMask& FmPortMask::operator&=(unsigned p)
{
  _mask &= p;
  return *this;
}

inline FmPortMask& FmPortMask::operator|=(unsigned p)
{
  _mask |= p;
  return *this;
}

inline FmPortMask& FmPortMask::operator|=(const FmPortMask& p)
{
  _mask |= p._mask;
  return *this;
}

inline FmPortMask operator|(const FmPortMask& a, const FmPortMask& b)
{
  return FmPortMask(unsigned(a) | unsigned(b));
}

#endif
