/* --------------------------------------------------------------------- *//*!

   \file
   \brief  Byteswapping, callable implementation.
   \author JJRussell - russell@slac.stanford.edu

\verbatim

    CVS $Id: BSWP.c,v 1.4 2005/10/01 01:00:13 russell Exp $
\endverbatim
                                                                         */
/* --------------------------------------------------------------------- */

/*
 | Unfortunately the sun puts the bcopy routines in strings.h....
*/
#ifdef   __sun__
#include <strings.h>
#else
#include <string.h>
#endif



#include "conversion/Alias.h"
#include "conversion/Attribute.h"
#include "conversion/BSWP.h"
#include "conversion/impl/BSWP.ih"

#ifdef __cplusplus
extern "C"
{
#endif


/* --------------------------------------------------------------------- *//*!

    \fn        void BSWP_swap16N (      uint16_t *dst,
                                  const uint16_t *src,
                                                       int  cnt)
    \brief     Byte swaps 16 bit words
    \param dst The destination array to receive the swapped words
    \param src The source array of 16 bit words to swap
    \param cnt The number of 16 bit words to swap

     This routine byte swaps the source array of 16 bit integers into
     the destination array.
                                                                         */
/* --------------------------------------------------------------------- */






/* --------------------------------------------------------------------- *//*!

    \fn        void BSWP_swap16lN (      uint16_t *dst,
                                   const uint16_t *src,
                                                        int  cnt)
    \brief     Byte swaps 16 bit words; local representation <=> little
    \param src The source array of 16 bit words to swap
    \param dst The destination array to receive the swapped words
    \param cnt The number of 16 bit words to swap

     This routine transforms the source array of 16 bit integer to/from
     local/little endian representation from/to little endian/local
     representation. If the local representation is little endian, then
     this operation is a copy. If the local representation is little endian
     and the source and destination addresses are identical, then this
     operation is a NOOP.
                                                                         */
/* --------------------------------------------------------------------- */






/* --------------------------------------------------------------------- *//*!

    \fn        void BSWP_swap16bN (      uint16_t *dst,
                                   const uint16_t *src,
                                                        int  cnt)
    \brief     Byte swaps    16 bit words; local representation <=> big
    \param dst The destination array to receive the swapped words
    \param src The source      array of 16 bit words to swap
    \param cnt The number of 16 bit words to swap

     This routine transforms the source array of 16 bit integer to/from
     local/big endian representation from to/big endian/local representation.
     If the local representation is big endian, then this operation is
     a copy. If the local representation is big endian and the source
     and destination addresses are identical, then this operation is a
     NOOP.

   \warning
     This function name is being deprecated. The new name is
      BSWP_swap16bN().

                                                                         */
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

    \fn        void BSWP_swap32N (      uint32_t *dst,
                                  const uint32_t *src,
                                                 int  cnt)
    \brief     Byte swaps    32 bit words
    \param src The source      array of 32 bit words to swap
    \param dst The destination array to receive the swapped words
    \param cnt The number of 32 bit words to swap

     This routine byte swaps the source array of 32 bit integers into
     the destination array.
                                                                         */
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

    \fn        void BSWP_swap32lN (      uint32_t *dst,
                                   const uint32_t *src,
                                                  int  cnt)
    \brief     Byte swaps an array 32 bit words;
               local representation <=> little
    \param dst The destination array to receive the swapped words
    \param src The source      array of 32 bit words to swap
    \param cnt The number of 32 bit words to swap

     This routine transforms the source array of 32 bit integer to/from
     local/little endian representation from to little endian/local
     representation. If the local representation is little endian, then
     this operation is a copy. If the local representation is little endian
     and the source and destination addresses are identical, then this
     operation is a NOOP.
                                                                         */
/* --------------------------------------------------------------------- */






/* --------------------------------------------------------------------- *//*!

    \fn        void BSWP_swap32bN (      uint32_t *dst,
                                   const uint32_t *src,
                                                  int  cnt)
    \brief     Byte swaps 32 bit words; local representation <=> big
    \param dst The destination array to receive the swapped words
    \param src The source array of 32 bit words to swap
    \param cnt The number of 32 bit words to swap

     This routine transforms the source array of 32 bit integer to/from
     local/big endian representation from/to big endian/local representation.
     If the local representation is big endian, then this operation is
     a copy. If the local representation is big endian and the source
     and destination addresses are identical, then this operation is a
     NOOP.

   \warning
     This function name is being deprecated. The new name is
     BSWP_swap32bN().
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

    \fn        void BSWP_swap64N (      uint64_t *dst,
                                  const uint64_t *src,
                                                           int  cnt)
    \brief     Byte swaps    64 bit words
    \param src The source      array of 64 bit words to swap
    \param dst The destination array to receive the swapped words
    \param cnt The number of 64 bit words to swap

     This routine byte swaps the source array of 64 bit integers into
     the destination array.
                                                                         */
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

    \fn        void BSWP_swap64lN (      uint64_t *dst,
                                   const uint64_t *src,
                                                            int  cnt)
    \brief     Byte swaps an array 64 bit words;
               local representation <=> little
    \param dst The destination array to receive the swapped words
    \param src The source      array of 64 bit words to swap
    \param cnt The number of 64 bit words to swap

     This routine transforms the source array of 64 bit integer to/from
     local/little endian representation from to little endian/local
     representation. If the local representation is little endian, then
     this operation is a copy. If the local representation is little endian
     and the source and destination addresses are identical, then this
     operation is a NOOP.
                                                                         */
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

    \fn        void BSWP_swap64bN (      uint64_t *dst,
                                   const uint64_t *src,
                                                            int  cnt)
    \brief     Byte swaps 64 bit words; local representation <=> big
    \param dst The destination array to receive the swapped words
    \param src The source array of 64 bit words to swap
    \param cnt The number of 64 bit words to swap

     This routine transforms the source array of 64 bit integer to/from
     local/big endian representation from/to big endian/local representation.
     If the local representation is big endian, then this operation is
     a copy. If the local representation is big endian and the source
     and destination addresses are identical, then this operation is a
     NOOP.
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- */
#ifndef CMX_DOXYGEN   /* Internal routines only, no public documentation */
/* --------------------------------------------------------------------- */
 void bswp_copy16N (      uint16_t *dst,
                          const uint16_t *src,
                                                   int  cnt) ATTR_UNUSED_OK;

 void bswp_copy32N (      uint32_t *dst,
                          const uint32_t *src,
                                                   int  cnt) ATTR_UNUSED_OK;

 void bswp_copy64N (      uint64_t *dst,
                          const uint64_t *src,
                                                   int  cnt) ATTR_UNUSED_OK;

/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!
   \internal
   \fn    static void bswp_copy16N (      uint16_t *dst,
                                    const uint16_t *src,
                                                         int  cnt)
   \brief Internal routine used to implement an array copy

   \param dst The destination array to receive the copied words
   \param src The source array of 16 bit words to copy
   \param cnt The number of 16 bit words to copy

    BSWP_lclXbig16N() or BSWP_lclXlittle16N() are aliased to this routine
    when the local representation matches the target representation and
    the source and destination arrays are not identical.
                                                                         */
/* --------------------------------------------------------------------- */
 void bswp_copy16N (      uint16_t *dst,
                          const uint16_t *src,
                                               int  cnt)
{
  //bcopy ((char *)src, (char *)dst, cnt * sizeof (*dst));
    memmove ((char *)dst, (char *)src, cnt * sizeof (*dst));
    return;
}
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!
   \internal
   \fn    static void bswp_copy32N (      uint32_t *dst,
                                    const uint32_t *src,
                                                   int  cnt)
   \brief Internal routine used to implement an array copy

   \param dst The destination array to receive the copied words
   \param src The source array of 32 bit words to copy
   \param cnt The number of 32 bit words to copy

    BSWP_lclXbig32N() or BSWP_lclXlittle32N() are aliased to this routine
    when the local representation matches the target representation and
    the source and destination arrays are not identical.
                                                                         */
/* --------------------------------------------------------------------- */
 void bswp_copy32N (      uint32_t *dst,
                          const uint32_t *src,
                                         int  cnt)
{
  //bcopy ((char *)src, (char *)dst, cnt * sizeof (*dst));
    memmove ((char *)dst, (char *)src, cnt * sizeof (*dst));
    return;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!
   \internal
   \fn    static void bswp_copy64N (      uint64_t *dst,
                                    const uint64_t *src,
                                                             int  cnt)
   \brief Internal routine used to implement an array copy

   \param dst The destination array to receive the copied words
   \param src The source array of 64 bit words to copy
   \param cnt The number of 64 bit words to copy

    BSWP_lclXbig64N() or BSWP_lclXlittle64N() are aliased to this routine
    when the local representation matches the target representation and
    the source and destination arrays are not identical.
                                                                         */
/* --------------------------------------------------------------------- */
 void bswp_copy64N (      uint64_t *dst,
                          const uint64_t *src,
                                                   int  cnt)
{
  //bcopy ((char *)src, (char *)dst, cnt * sizeof (*dst));
    memmove ((char *)dst, (char *)src, cnt * sizeof (*dst));
    return;
}
/* --------------------------------------------------------------------- */
#endif
/* --------------------------------------------------------------------- */






/* --------------------------------------------------------------------- */
void BSWP_swap16N (      uint16_t *dst,
                   const uint16_t *src,
                                        int  cnt)
{
    /* Determine if the copy should go forward or backwards */
    if ((uintptr_t)src >= (uintptr_t)(dst))
    {
        int idx = 0;
        while (--cnt >= 0)
        {
            dst[idx] = BSWP__load16x (src, sizeof (typeof (*src)) * idx);
            idx++;
        }
    }
    else
    {
        while (--cnt >= 0)
        {
            dst[cnt] = BSWP__load16x (src, sizeof (typeof (*src)) * cnt);
        }
    }

    return;
}
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- */
void BSWP_swap32N (      uint32_t *dst,
                   const uint32_t *src,
                                  int  cnt)
{
    /* Determine if the copy should go forward or backwards */
    if ((uintptr_t)src >= (uintptr_t)(dst))
    {
        int idx = 0;
        while (--cnt >= 0)
        {
            dst[idx] = BSWP__load32x (src, sizeof (typeof (*src)) * idx);
            idx++;
        }
    }
    else
    {
        while (--cnt >= 0)
        {
            dst[cnt] = BSWP__load32x (src, sizeof (typeof (*src)) * cnt);
        }
    }

    return;
}
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- */
void BSWP_swap64N (      uint64_t *dst,
                   const uint64_t *src,
                                            int  cnt)
{
    /* Determine if the copy should go forward or backwards */
    if ((uintptr_t)src >= (uintptr_t)(dst))
    {
        int idx = 0;
        while (--cnt >= 0)
        {
            dst[idx] = BSWP__load64x (src, sizeof (typeof (*src)) * idx);
            idx++;
        }
    }
    else
    {
        while (--cnt >= 0)
        {
            dst[cnt] = BSWP__load64x (src, sizeof (typeof (*src)) * cnt);
        }
    }

    return;
}
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *\
 |                                                                       |
 | The following routines exist only as aliases. DOXYGEN demands to see  |
 | an implementation, so a fake one is given, just keep DOXYGEN happy.   |
 | The implementation is deliberately uncompilable, just to make sure    |
 | it does not accidentally sneak into real code.                        |
 |                                                                       |
\* --------------------------------------------------------------------- */
#ifdef CMX_DOXYGEN
/* --------------------------------------------------------------------- */

void BSWP_swap16lN (      unsigned short    int *dst,
		    const unsigned short    int *src,
                                            int  cnt)   { return ?; }

void BSWP_swap16bN (      unsigned short    int *dst,
                    const unsigned short    int *src,
                                            int  cnt)   { return ?; }



void BSWP_swap32lN (      unsigned          int *dst,
                    const unsigned          int *src,
                                            int  cnt)   { return ?; }

void BSWP_swap32bN (      unsigned          int *dst,
                    const unsigned          int *src,
                                            int  cnt)   { return ?; }



void BSWP_swap64lN (      uint64_t *dst,
                    const uint64_t *src,
                                             int  cnt)  { return ?; }

void BSWP_swap64bN (      uint64_t *dst,
                    const uint64_t *src,
                                             int  cnt)  { return ?; }


/* --------------------------------------------------------------------- */
#endif
/* --------------------------------------------------------------------- */




//JHP:  these need to go outside of the extern "C" block

#if ENDIANNESS_IS_BIG

ALIAS_BEG (BSWP_)


ALIAS_FNC (void, bswp_copy16N, BSWP_swap16bN,(uint16_t*, const uint16_t*, int));
ALIAS_FNC (void, BSWP_swap16N, BSWP_swap16lN,(uint16_t*, const uint16_t*, int));
ALIAS_FNC (void, bswp_copy32N, BSWP_swap32bN,(uint32_t*, const uint32_t*, int));
ALIAS_FNC (void, BSWP_swap32N, BSWP_swap32lN,(uint32_t*, const uint32_t*, int));
ALIAS_FNC (void, bswp_copy64N, BSWP_swap64bN,(uint64_t*, const uint64_t*, int));
ALIAS_FNC (void, BSWP_swap64N, BSWP_swap64lN,(uint64_t*, const uint64_t*, int));


ALIAS_END (BSWP_)

#elif ENDIANNESS_IS_LITTLE

ALIAS_BEG (BSWP_)


ALIAS_FNC (void, BSWP_swap16N, BSWP_swap16bN,(uint16_t*, const uint16_t*, int));
ALIAS_FNC (void, bswp_copy16N, BSWP_swap16lN,(uint16_t*, const uint16_t*, int));
ALIAS_FNC (void, BSWP_swap32N, BSWP_swap32bN,(uint32_t*, const uint32_t*, int));
ALIAS_FNC (void, bswp_copy32N, BSWP_swap32lN,(uint32_t*, const uint32_t*, int));
ALIAS_FNC (void, BSWP_swap64N, BSWP_swap64bN,(uint64_t*, const uint64_t*, int));
ALIAS_FNC (void, bswp_copy64N, BSWP_swap64lN,(uint64_t*, const uint64_t*, int));



ALIAS_END (BSWP_)

#endif




#ifdef __cplusplus
}
#endif
