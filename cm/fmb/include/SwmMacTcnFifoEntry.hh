#ifndef Bali_SwmMacTcnFifoEntry_hh
#define Bali_SwmMacTcnFifoEntry_hh


#include "fm/FmPort.hh"
#include "fmb/SwmMacTableEntry.hh"

namespace Bali {

  class SwmMacTcnFifoEntry {
  public:
    SwmMacTcnFifoEntry(const SwmMacTableEntry&,
                       unsigned index, 
                       unsigned set,
                       unsigned type,
                       unsigned parity);

    //SwmMacTcnFifoEntry& operator=(const SwmMacTcnFifoEntry&);

    const SwmMacTableEntry& entry() const;
    unsigned         index () const;
    unsigned         set   () const;
    unsigned         type  () const;
    unsigned         parity() const;

  private:
    SwmMacTableEntry _entry;
    unsigned         _index;
    unsigned         _set;
    unsigned         _type;
    unsigned         _parity;
  };
};

inline Bali::SwmMacTcnFifoEntry::SwmMacTcnFifoEntry(const Bali::SwmMacTableEntry& e,
                                                    unsigned index,
                                                    unsigned set,
                                                    unsigned type,
                                                    unsigned parity) :
  _entry(e),
  _index(index),
  _set  (set),
  _type (type),
  _parity(parity)
{
}

inline const Bali::SwmMacTableEntry& Bali::SwmMacTcnFifoEntry::entry() const
{ return _entry; }
inline unsigned         Bali::SwmMacTcnFifoEntry::index () const
{ return _index; }
inline unsigned         Bali::SwmMacTcnFifoEntry::set   () const
{ return _set; }
inline unsigned         Bali::SwmMacTcnFifoEntry::type  () const
{ return _type; }
inline unsigned         Bali::SwmMacTcnFifoEntry::parity() const
{ return _parity; }

#endif
