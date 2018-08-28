// -*-Mode: C++;-*-
/**
@file
@brief Declare and implement BitRota (generic implementation).

@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2011/07/21

@par Credits:
SLAC
*/
#if !defined(TOOL_DATASTRUCTURE_BITROTA_HH)
#define      TOOL_DATASTRUCTURE_BITROTA_HH


#include "dataStructure/Bits.hh"

namespace tool {

  namespace dataStructure {

      /** @brief Round-robin scheduling using one bit per object scheduled.

          The implementation is generic in that uses the functions in
          dataStructure::Bits.

          A BitRota is normally updated by several threads, hence the
          volatile qualifiers on the member functions.
      */
      class BitRota {
      public:

        /** @brief Set the rota places selected by ones in the given mask. */
        void add(unsigned mask) volatile;

        enum {
          /** @brief Signifies that there is no place ready in a rota. */
          NOBODY = -1
        };

        /** @brief Find the next rota place that is ready. Return NOBODY if
            there is no such place. The highest-order bit is called zero,
	    the next highest one, etc.
        */
        int removeNext() volatile;

        /** @brief Set up a rota with no positions assigned (or ready). */
        BitRota();

        void clear() volatile;

      private:

        int m_freePlace; ///< The next unassigned place.

        unsigned m_rota; ///< The ready bits in the original (unrotated) order.

        int m_nextPlace; ///< Mod 32, the next place up for servicing.
      };


      inline BitRota::BitRota(): m_freePlace(0), m_rota(0), m_nextPlace(0) {}

      inline void BitRota::add(unsigned mask) volatile {m_rota |= mask;}

      namespace local {
        inline unsigned rotate(unsigned val, unsigned places) {
          places &= 0x1f;
          return (val << places) | (val >> (32 - places));
        }
      }

      inline int BitRota::removeNext() volatile {
        // Bring to the top the next place to be examined.
        register int place(m_nextPlace);
        register unsigned rota(local::rotate(m_rota, place));
        register int result;
        if (rota) {
          // Somebody is ready. Find who it is and remove it. On the
          // next call start looking at the next place after the one
          // we found.
          place += Bits::clz(rota);
          m_rota &= ~(0x80000000U >> place);
          m_nextPlace = place + 1;
          result = place & 0x1f;
          
        }
        else {
          result = NOBODY;
        }
        return result;
      }

  } // dataStructure
} // tool
#endif
