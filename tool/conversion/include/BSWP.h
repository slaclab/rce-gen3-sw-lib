#ifndef BSWP_H
#define BSWP_H

/* --------------------------------------------------------------------- *//*!
  
   \file  BSWP.h
   \brief Byteswapping, function prototypes, callable versions
   \author JJRussell - russell@slac.stanford.edu

\verbatim

    CVS $Id: BSWP.h,v 1.3 2005/10/01 01:00:11 russell Exp $
\endverbatim 

   \b SYNOPSIS \n
    The BSWP routines provide efficient and uniform methods of performing
    various byte swap operations on arrays of 16, 32 and 64 bit integers.
    These routines use the inline routines of BSWP.ih to implement the
    byteswapping. When possible (currently the PPC  platforms), these
    inline routines map directly onto the hardware instructions used to
    load and store the 16, 32 and 64 bit byteswapped quantities to and from
    memory.

    The routines come in 3 sets of 3. The first dimension is for operating
    on one of 16, 32 or 64 bit quantities, while the \e 3 dimension is for
    operating on these quantities when they are

       - unconditionally byteswapped
       - translating to/from the local representation from/to big endian
       - translating to/from the local representation from/to little endian

    The single word word swapping routines are so simple and efficiency
    is such a motivating design factor that these single word versions of
    the byteswapping routines are provided only as inlines; see BSWP.ih The
    vector routines are provided only as callable functions.

   \note 
    The vector swapping routines are safe independent of whether or how
    the source and destination buffers overlap.
                                                                         */
/* --------------------------------------------------------------------- */




/* ---------------------------------------------------------------------- *\
 *
 * HISTORY  WHO   WHAT
 * -------  ---   -------------------------------------------------------
 * 09.27.05 jjr   Added return type argument to ALIAS_FNC macro
 * 09.27.05 jjr   Added history log
 *
\* ---------------------------------------------------------------------- */

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif



    
/* ------------------------------------------------------------------------- *\
 |                                                                           |
 |              ARRAY  16  BIT SWAPs, STOREs and LOADs                       |
 |                                                                           | 
\* ------------------------------------------------------------------------- */
extern void BSWP_swap16N            (      uint16_t *dst,
                                     const uint16_t *src,
                                                          int  cnt);


extern void BSWP_swap16lN           (      uint16_t *dst,
                                     const uint16_t *src,
                                                          int  cnt);


extern void BSWP_swap16bN           (      uint16_t *dst,
                                     const uint16_t *src,
                                                          int  cnt);

/* ------------------------------------------------------------------------- */



    
/* ------------------------------------------------------------------------- *\
 |                                                                           |
 |              ARRAY  32 BIT SWAPs, STOREs and LOADs                        |
 |                                                                           | 
\* ------------------------------------------------------------------------- */
extern void BSWP_swap32N            (      uint32_t *dst,
                                     const uint32_t *src,
                                                    int  cnt);


extern void BSWP_swap32lN           (      uint32_t *dst,
                                     const uint32_t *src, 
                                                    int  cnt);

extern void BSWP_swap32bN           (      uint32_t *dst,
                                     const uint32_t *src,
                                                    int  cnt);

/* ------------------------------------------------------------------------- */




/* ------------------------------------------------------------------------- *\
 |                                                                           |
 |              ARRAY  64 BIT SWAPs, STOREs and LOADs                        |
 |                                                                           | 
\* ------------------------------------------------------------------------- */
extern void BSWP_swap64N            (      uint64_t *dst,
                                     const uint64_t *src,
                                                              int  cnt);

extern void BSWP_swap64lN           (      uint64_t *dst,
                                     const uint64_t *src, 
                                                              int  cnt);

extern void BSWP_swap64bN           (      uint64_t *dst,
                                     const uint64_t *src,
                                                              int  cnt);

/* ------------------------------------------------------------------------- */


    
#ifdef __cplusplus
}
#endif

#endif

