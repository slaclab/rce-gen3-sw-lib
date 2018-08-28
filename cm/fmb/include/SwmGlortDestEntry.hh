#ifndef Bali_SwmGlortDestEntry_hh
#define Bali_SwmGlortDestEntry_hh

#include "fm/FmPort.hh"

namespace Bali {
  class FmGlortDestEntry;

  class SwmGlortDestEntry {
  public:
    SwmGlortDestEntry();
    SwmGlortDestEntry(FmPortMask dest_mask, unsigned ip_mcast=0);
    //  SwmGlortDestEntry( const FmGlortDestEntry& );
    ~SwmGlortDestEntry();

    SwmGlortDestEntry& operator=(const SwmGlortDestEntry&);

  public:
    bool       parityError   () const;
    FmPortMask destMask      () const;
    unsigned   ip_mcast_index() const;

  public:
    unsigned a;
    unsigned b;
  };
};

#endif
