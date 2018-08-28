#ifndef FmLCIPacket_hh
#define FmLCIPacket_hh

//  Class FmLCIPacket
//
//  A packet received on the FM2224 Logical CPU Interface.
//  First 4 bytes are LCI_RX_FRAME_EXTRA_INFO
//  [23:0] = SrcPort (6b) | VLAN Action (2b) | Priority (4b) | VLAN Id (12b)
//  Last 4 bytes are LCI_RX_FRAME_STATUS
//  [5:0] = Padding (3b) | Uflow (1b) | TailErr (1b) | BadCRC (1b)


#include "fm/FmPort.hh"

class FmLCIPacket {
public:
  FmLCIPacket(const void* data, unsigned len);
  ~FmLCIPacket();

public:
  // rx frame extra info
  FmPort   source_port() const;
  unsigned priority() const;
  unsigned vlanID() const;
  unsigned vlanAction() const;

  // rx frame status
  unsigned padding() const;
  bool underflow() const;
  bool tailError() const;
  bool badCRC() const;

  char* payload() const;
  unsigned payload_size() const;

private:
  const char* _data;
  unsigned    _words;
};


inline FmLCIPacket::FmLCIPacket(const void* data, unsigned len) :
  _data ((const char*)data),
  _words(len)
{
}

inline FmLCIPacket::~FmLCIPacket()
{
}

inline FmPort FmLCIPacket::source_port() const
{
  return FmPort((*reinterpret_cast<const unsigned*>(_data) >> 18) & 0x3f);
}

inline unsigned FmLCIPacket::priority() const
{
  return (*reinterpret_cast<const unsigned*>(_data) >> 12) & 0xf;
}

inline unsigned FmLCIPacket::vlanID() const
{
  return (*reinterpret_cast<const unsigned*>(_data) >> 0) & 0xfff;
}

inline unsigned FmLCIPacket::vlanAction() const
{
  return (*reinterpret_cast<const unsigned*>(_data) >> 16) & 0x3;
}

  // rx frame status
inline unsigned FmLCIPacket::padding() const
{
  return (reinterpret_cast<const unsigned*>(_data)[_words-1]>>3)&0x7;
}

inline bool FmLCIPacket::underflow() const
{
  return reinterpret_cast<const unsigned*>(_data)[_words-1]&0x4;
}

inline bool FmLCIPacket::tailError() const
{
  return reinterpret_cast<const unsigned*>(_data)[_words-1]&0x2;
}

inline bool FmLCIPacket::badCRC() const
{
  return reinterpret_cast<const unsigned*>(_data)[_words-1]&0x1;
}

inline char* FmLCIPacket::payload() const
{
  return const_cast<char*>(_data) + 4;
}

inline unsigned FmLCIPacket::payload_size() const
{
  return 4*_words - 4 - padding();
}

#endif
