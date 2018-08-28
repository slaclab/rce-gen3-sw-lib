// -*-Mode: C++;-*-
/**
@file
@brief Brief description of file contents and purpose here.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(EXAMPLE_PSEUDOARP_HH)
#define      EXAMPLE_PSEUDOARP_HH

#include <inttypes.h>
#include "kvt/Kvt.h"

namespace examples {
  class PseudoARP {
  public:
    PseudoARP();
    ~PseudoARP();

  public:
    uint64_t lookup(uint32_t);

  public:
    int refresh();
    
  private:
    KvTable _table;
  };
}
#endif
