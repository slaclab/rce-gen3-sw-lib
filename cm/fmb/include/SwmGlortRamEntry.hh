#ifndef Bali_SwmGlortRamEntry_hh
#define Bali_SwmGlortRamEntry_hh

#include "fm/FmPort.hh"

namespace Bali {
  class FmGlortRamEntry;

  class SwmGlortRamEntry {
  public:
    SwmGlortRamEntry();
    SwmGlortRamEntry(unsigned dest_index);
    SwmGlortRamEntry(unsigned dest_index,
                     unsigned sub_indexa_pos,
                     unsigned sub_indexa_nb,
                     unsigned sub_indexb_pos,
                     unsigned sub_indexb_nb,
                     unsigned dest_count,
                     unsigned rotation);
    //  SwmGlortRamEntry( const FmGlortRamEntry& );
    ~SwmGlortRamEntry();

    SwmGlortRamEntry& operator=(const SwmGlortRamEntry&);

  public:
    bool       parityError   () const;
    unsigned   strict        () const;
    unsigned   destIndex     () const;
    unsigned   subIndexA_pos () const;
    unsigned   subIndexA_nb  () const;
    unsigned   subIndexB_pos () const;
    unsigned   subIndexB_nb  () const;
    unsigned   destCount     () const;
    unsigned   hashRotation  () const;

  public:
    unsigned a;
    unsigned b;
  };
};

#endif
