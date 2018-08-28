#ifndef shelf_ping_hh
#define shelf_ping_hh

#include "atca/Attributes.hh"

#include <string.h>
#include <new>
#include <arpa/inet.h>

const unsigned mcast=0xefff0001;
const unsigned short port_command   = 10001;
const unsigned short port_discover  = 10002;

static const int _dpm_elem[] = {0,2,-1};
static const int _dtm_elem[] = {0,-1};
static const int* _elem[] = { _dpm_elem, _dpm_elem, _dpm_elem, _dpm_elem, _dtm_elem };

namespace shelf {
  namespace ping {
    class Command {
    public:
      Command(const char* shelf, unsigned slotmask, unsigned echoes) :
        _slotmask(slotmask), _echoes(echoes)
      {
        memcpy(_shelf, shelf, 8);
        memset(_reserved, 0, 2);
      }
    public:
      uint8_t  _shelf[8];
      uint8_t  _reserved[2];
      uint16_t _slotmask;
      uint32_t _echoes;
    };

    class Tag {
    public:
      enum { nslots=15, nbays=5, nelements=4 };
    public:
      Tag() {}
      Tag(const service::atca::Attributes& attr) :
        _reserved(0),
        _slot   (attr.cluster()),
        _bay    (attr.bay()),
        _element(attr.element())
      { memcpy(_shelf, attr.group(), 8); }
      Tag(const char* shelf, unsigned mask) :
        _reserved((mask>>8)&0xff),
        _slot    ((mask>>0)&0xff),
        _bay     (0),
        _element (0)
      { memcpy(_shelf, shelf, 8); }
      Tag(const Tag& o) { memcpy(this,&o, sizeof(o)); }
    public:
      bool operator==(const Tag& o) const
      { return memcmp(this, &o, sizeof(Tag))==0; }
      Tag& operator=(const Tag& o) { memcpy(this,&o,sizeof(o)); return *this; }
    public:
      unsigned slots() const { unsigned r=_reserved; r = (r<<8)|_slot; return r; }
    public:
      uint8_t           _shelf[8];
      uint8_t           _reserved;
      uint8_t           _slot;
      uint8_t           _bay;
      uint8_t           _element;
    };

    class Report {
    public:
      Report(const Tag& dst, unsigned n) : _dst(dst), _n(n) {}
    public:
      Tag      _dst;
      uint32_t _n;
    };

    class Response {
    public:
      Response() {}
      Response(const Tag& tag) :
        _src (tag),
        _ndst(0) {}
    public:
      const Report& report(unsigned i) const
      { return reinterpret_cast<const Report*>(this+1)[i]; }
      size_t _sizeof() const
      { return sizeof(*this)+_ndst*sizeof(Report); }
    public:
      void append(const Report& r)
      { reinterpret_cast<Report*>(this+1)[_ndst++] = r; }
    public:
      Tag               _src;
      uint32_t          _ndst;
    };

    class Service {
    public:
      Tag               _src;
      Tag               _dst;
      uint32_t          _reply;
    };

    class Results {
    public:
      Results() : _reply(0) {}
      Results(const Response& r)
      {
        _reply++;
        for(unsigned i=0; i<Tag::nslots*Tag::nbays*Tag::nelements; i++)
          _result[i] = -1U;
        for(unsigned i=0; i<r._ndst; i++) {
          const Report& p = r.report(i);
          const Tag& dst = p._dst;
          // Sparse mapping
          unsigned index = (dst._slot*Tag::nbays + dst._bay)*Tag::nelements + 
            dst._element;
          _result[index] = p._n;
        }
      }
    public:
      uint32_t _reply;
      uint32_t _result[Tag::nslots*Tag::nbays*Tag::nelements];
    };
  };
};

#endif
