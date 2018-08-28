#ifndef shelf_ping_hh
#define shelf_ping_hh

#include "atca/Attributes.hh"

#include <string.h>
#include <new>
#include <arpa/inet.h>

const unsigned mcast=0xefff0001;
const unsigned short port_remotesvc   = 10003;

static const int _dpm_elem[] = {0,2,-1};
static const int _dtm_elem[] = {0,-1};
static const int* _elem[] = { _dpm_elem, _dpm_elem, _dpm_elem, _dpm_elem, _dtm_elem };

namespace shelf {
  namespace remote_service {
    class Command {
    public:
      enum Type { FileTransfer,
                  ShellCommand };
    public:
      Command() {}
      Command(unsigned    id,
              const char* shelf, 
              unsigned    slotmask, 
              unsigned    elements,
              const char* fname,
              size_t      offset) :
        _type    (FileTransfer),
        _id      (id),
        _slotmask(slotmask), 
        _elements(elements), 
        _size    (0),
        _offset  (offset)
      {
        memcpy(_shelf, shelf, 8);
        strcpy(reinterpret_cast<char*>(this+1), fname);
      }
      Command(unsigned    id,
              const char* shelf, 
              unsigned    slotmask, 
              unsigned    elements, 
              const char* command) :
        _type    (ShellCommand), 
        _id      (id),
        _slotmask(slotmask),
        _elements(elements),
        _size    (0),
        _offset  (0)
      {
        memcpy(_shelf, shelf, 8);
        strcpy(reinterpret_cast<char*>(this+1), command);
      }
      Command(const Command& c) :
        _type    (c._type),
        _id      (c._id),
        _slotmask(c._slotmask),
        _elements(c._elements),
        _size    (c._size),
        _offset  (c._offset)
      { memcpy(_shelf,c._shelf,8); }
    public:
      void* operator new   (size_t,void* p) { return p; }
      void  operator delete(void* p)        {}
    public:
      Type type() const { return Type(_type); }
      const char* command() const { return reinterpret_cast<const char*>(this+1); }
      const char* fname  () const { return reinterpret_cast<const char*>(this+1); }
      const char* payload() const { return fname()+strlen(fname())+1; }
      unsigned    _sizeof() const { return payload()+_size-(const char*)this; }
    public:
      char* payload() { return const_cast<char*>(fname())+strlen(fname())+1; }
    public:
      uint16_t _type;
      uint16_t _id;
      uint8_t  _shelf[8];
      uint32_t _slotmask;
      uint32_t _elements;
      uint32_t _size;
      uint64_t _offset;
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

    class Ack {
    public:
      Ack() {}
      Ack(const Command& cmd,
          const Tag&     src) :
        _cmd(cmd),
        _src(src) {}
    public:
      Command _cmd;
      Tag     _src;
    };
  };
};

#endif
