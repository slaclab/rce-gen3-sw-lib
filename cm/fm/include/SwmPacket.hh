#ifndef SwmPacket_hh
#define SwmPacket_hh


#include "fm/FmPort.hh"

class FmTahoe;

class SwmPacket {
public:
  SwmPacket( unsigned );
  SwmPacket( FmTahoe& );
  ~SwmPacket();

  bool operator==(const SwmPacket&) const;

  unsigned   bytes() const { return _len<<2; }
  unsigned   len()  const { return _len; }  // in words
  unsigned   dst()  const { return _dst; }
  const unsigned* data() const { return _data; }
  //  unsigned* data() const { return (unsigned*)(this+1); }

  unsigned size() const { return sizeof(_dev)+sizeof(unsigned)*(_len+2); }

  // rx info
  FmPort   srcPort() const { return FmPort((_dst>>18)&0x1f); }
  unsigned rxPri  () const { return (_dst>>12)&0xf; }
  unsigned vlanId () const { return (_dst>> 0)&0x7ff; }
  unsigned vlanAct() const { return (_dst>>16)&0x3; }
  unsigned rxPad  () const { return (_data[_len+1]>>3)&7; }
  unsigned uflow  () const { return (_data[_len+1]>>2)&1; }
  unsigned tailErr() const { return (_data[_len+1]>>1)&1; }
  unsigned crcErr () const { return (_data[_len+1]>>0)&1; }

  void append(unsigned);

private:
  //  enum { MTU=1514, PREAMBLE=8, FCSLEN=4 };
  enum { MTU=2048, PREAMBLE=8, FCSLEN=4 };
  FmTahoe* _dev;
  unsigned _len;
  unsigned _dst;
  unsigned _data[(MTU+PREAMBLE+FCSLEN+3)>>2];
};

#endif
