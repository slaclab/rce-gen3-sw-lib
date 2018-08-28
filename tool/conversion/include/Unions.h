#ifndef UNIONS_H
#define UNIONS_H


/* --------------------------------------------------------------------- *//*!


  \file   Unions.h
  \brief  Defines a set of useful unions between long long, ints, shorts
          and chars (bytes) which allow one to access subfields in a
          consistent fashion across big and little endian machines.
  \author JJRussell - russell@slac.stanford.edu

\verbatim
  CVS $Id: Unions.h,v 1.4 2005/10/01 00:33:26 russell Exp $
\endverbatim


  \par DESCRIPTION
   Many times one needs to access a subfield of large value. For example,
   extracting the most significant 32 bits of a 64 bit number. Unfortunately,
   this extraction is dependent of the endianness of the machine. The
   unions and macros defined in here allow one t uniformly do this extraction.

  \par USAGE
   The user has two choices in the packing and unpacking. The first is to
   use a set of unions to load the wider type or assess narrower types.
   The unions are completely general, allowing assess to any narrower type
   contained within the wider type. This generality comes at the expense
   of verbosity, which some would interpret as overly complex. In the
   simple and often needed case of assessing only the \e hi or \e lo halves,
   for example the \e hi or lo \e 32 bits of a long long int, a series of less
   general purpose, but easier to use macros are provided. These are of
   the form:

       \li U_UNPACK_SHORT_LO_BYTE(_us)
       \li U_UNPACK_SHORT_HI_BYTE(_us)
       \li U_UNPACK_INT_LO_SHORT(_ui)
       \li U_UNPACK_INT_HI_SHORT(_ui)
       \li U_UNPACK_LL_LO_INT(_ull)
       \li U_UNPACK_LL_HI_INT(_ull)

  \par
   One of the big problems here is one of language. There are two concepts
   of ordering; one by where the bits representing the least signficant
   digits (in a numerical sense) live and the other relating to their
   layout in memory. On little endian machines, these are the same. On big
   endian machines, the bits containing the least significant digits live
   in the highest addresses. Indeed, this is the problem these unions are
   trying to solve.

  \par TARGET PLATFORMS
   All FSW supported platforms.
                                                                         */
/* --------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *\
 *
 * HISTORY  WHO   WHAT
 * -------  ---   -------------------------------------------------------
 * 09.28.05 jjr   Corrected documentation formating for Doxygen 1.4.4
 * 09.28.05 jjr   Added history log
 *
\* ---------------------------------------------------------------------- */



#include "conversion/Endianness.h"

/* --------------------------------------------------------------------- *//*!

  \def    U_PACK_SHORT_WITH_BYTES(_hi, _lo)
  \brief  Packs a short with the numerically high and low bytes
  \param _hi The numerically  most significant byte
  \param _lo The numerically least significant byte
  \return    The packed short integer
                                                                         */
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \def    U_PACK_INT_WITH_SHORTS(_hi, _lo)
  \brief  Packs an int with the numerically high and low shorts
  \param _hi The numerically  most significant short
  \param _lo The numerically least significant short
  \return    The packed integer
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def    U_PACK_INT_WITH_BYTES(_3, _2, _1, _0)
  \brief  Packs an int with the 4 bytes, specified in order from the
          numerically most significant byte (\a _3) to the numerically
          least signicant byte (\e _0)
  \param _3 The numerically  most significant byte
  \param _2 The numericallly second most significant byte
  \param _1 The numericallly third  most significant byte
  \param _0 The numerically least significant byte
  \return    The packed integer
                                                                         */
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \def    U_PACK_LL_WITH_INTS(_hi, _lo)
  \brief  Packs a long long int with the numerically high and low ints
  \param _hi The numerically  most significant int
  \param _lo The numerically least significant int
  \return    The packed long long integer
                                                                         */
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \def    U_PACK_LL_WITH_SHORTS(_3, _2, _1, _0)
  \brief  Packs a long long int with 4 shorts, specified in order from
          the numerically most significant short (\a _3) to the
          numerically least signicant short (\e _0)
  \param _3 The numerically  most significant short
  \param _2 The numericallly second most significant short
  \param _1 The numericallly third  most significant short
  \param _0 The numerically least significant short
  \return   The packed long long integer
                                                                         */
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \def   U_PACK_LL_WITH_BYTES(_7, _6, _5, _4, _3, _2, _1, _0)
  \brief Packs a long long int with 8 bytes, specified in order from
         the numerically most significant byte (\a _7) to the
         numerically least signicant byte (\e _0)

  \param _7 The numerically  most significant byte
  \param _6 The numericallly second  most significant byte
  \param _5 The numericallly third   most significant byte
  \param _4 The numerically  fourth  most significant byte
  \param _3 The numerically  fifth   most significant byte
  \param _2 The numericallly sixth   most significant byte
  \param _1 The numericallly seventh most significant byte
  \param _0 The numerically least significant byte
  \return   The packed long long integer
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def   U_UNPACK_SHORT_LO_BYTE(_us)
  \brief Extracts the numerically least significant byte from a short

  \param _us The short to extract the byte from
  \returns   The numerically least significant byte.
                                                                         */
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \def   U_UNPACK_SHORT_HI_BYTE(_us)
  \brief Extracts the numerically most significant byte from a short

  \param  _us The short to extract the byte from
  \returns    The numerically most significant byte.
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def   U_UNPACK_INT_LO_SHORT(_ui)
  \brief Extracts the numerically least significant short from an int

  \param  _ui The int to extract the short from
  \returns    The  numerically least significant short.
                                                                         */
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \def   U_UNPACK_INT_HI_SHORT(_ui)
  \brief Extracts the numerically most significant short from an int

  \param  _ui The int to extract the short from
  \returns    The numerically most significant short.
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def   U_UNPACK_LL_LO_INT(_ull)
  \brief Extracts the numerically least significant int from a long long

  \param   _ull The long long to extract the int from
  \returns      The numerically least significant int.
                                                                         */
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \def   U_UNPACK_LL_HI_INT(_ull)
  \brief Extracts the numerically most significant int from a long long
         int

  \param   _ull The long long to extract the int from
  \returns      The numerically most significant int.
                                                                         */
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

  \union  _U_byte
  \brief   Accesses an 8 bit quantity as both a signed and unsigned byte.
                                                                         *//*!
  \typedef U_byte
  \brief   Typedef for union \e _U_byte
                                                                         */
/* --------------------------------------------------------------------- */
typedef union _U_byte
{
    signed char sb;  /*!< Interpretted as an   signed byte               */
  unsigned char ub;  /*!< Interpretted as an unsigned byte               */
}
U_byte;
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

  \union  _U_short
  \brief   Accesses a 16 bit quantity as narrower integer types (chars).

                                                                         *//*!
  \typedef U_short
  \brief   Typedef for union \e _U_short

   The 16 bits can be assessed as
     -# signed and unsigned shorts
     -# hi and lo signed and unsigned  8 bit chars within each of
         the hi and lo  16 bit shorts.


                                                                         */
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

  \union  _U_int
  \brief   Accesses a 32 bit quantity as narrower integer types (shorts
           and chars).
                                                                         *//*!
  \typedef U_int
  \brief   Typedef for union \e _U_int

  The 32 bits can be assessed as
     -# signed and unsigned ints
     -# hi and lo signed and unsigned 16 bit integers
     -# hi and lo signed and unsigned  8 bit chars within each of the
        hi and lo  16 bit shorts.

                                                                         */
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

  \union  _U_longlong
  \brief   Accesses a 64 bit quantity as narrower integer types (ints,
           shorts and chars).
                                                                         *//*!
  \typedef U_longlong
  \brief   Typedef for union \e _U_longlong

   The 64 bits can be assessed as
     -# signed and unsigned long longs
     -# hi and lo signed and unsigned 32 bit integers
     -# hi and lo signed and unsigned 16 bit shorts
        within each of the hi and lo  32 bit integers
     -# hi and lo signed and unsigned  8 bit chars
        within each of the hi and lo  16 bit shorts.
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \struct _U_short_as_bytes
  \brief   Internal structure which sets the ordering for assessing the
           bytes within a short.
                                                                         *//*!
  \typedef U_short_as_bytes
  \brief   Typedef for union \e _U_short_as_bytes

                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \struct _U_int_as_shorts
  \brief   Internal structure which sets the ordering for assessing the
           shorts within an int
                                                                         *//*!
  \typedef U_int_as_shorts
  \brief   Typedef for union \e _U_int_as_shorts

                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \struct _U_longlong_as_ints
  \brief   Internal structure which sets the ordering for assessing the
           ints within a long long int
                                                                         *//*!
  \typedef U_longlong_as_ints
  \brief   Typedef for union \e _U_longlong_as_ints
                                                                         */
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

   \def _UNIONS_SHORT_AS_BYTES
   \brief Internal use only
                                                                         *//*!
   \def _UNIONS_INT_AS_SHORTS
   \brief Internal use only
                                                                         *//*!
   \def _UNIONS_LONGLONG_AS_INTS
   \brief Internal use only
                                                                         */
/* --------------------------------------------------------------------- */



/* ===================================================================== *\
 |                                                                       |
 | This determines whether one picks up the big or little implementation |
 |                                                                       |
\* --------------------------------------------------------------------- */

#if     ENDIANNESS_IS_LITTLE

#       define UNIONS_BIG_LITTLE "conversion/impl/Unions_little.h"

#elif   ENDIANNESS_IS_BIG

#       define UNIONS_BIG_LITTLE "conversion/impl/Unions_big.h"

#else

/*
 |
 | !!! KLUDGE !!!
 | --------------
 | These are put here just to keep DOXYGEN happy. Tony has to change the
 | way include files in subdirectories are parsed to get the right
 | dependencies. When he gets it fixed, these should be removed
 |
*/
typedef struct _U_short_as_bytes
{
   union _U_byte  hi; /*!< Access the numerically least significant byte */
   union _U_byte  lo; /*!< Access the numerically most  significant byte */
}
U_short_as_bytes;


typedef struct _U_int_as_shorts
{
  union _U_short hi; /*!< Access the numerically least significant short */
  union _U_short lo; /*!< Access the numerically most  significant short */
}
U_int_as_shorts;


typedef struct _U_longlong_as_ints
{

 union _U_int    hi; /*!< Access the numerically least significant int   */
 union _U_int    lo; /*!< Access the numerically most  significant int   */
}
U_longlong_as_ints;


#error Unions.h cannot determine the endianness for

#endif
/* --------------------------------------------------------------------- */









/* ===================================================================== *\
 |                                                                       |
 |                          SHORTS                                       |
 |                                                                       |
\* --------------------------------------------------------------------- */
#define _UNIONS_SHORT_AS_BYTES
#include UNIONS_BIG_LITTLE
#undef  _UNIONS_SHORT_AS_BYTES


#define  U_PACK_SHORT_WITH_BYTES(_hi, _lo)                                \
         ((((_hi)&0xff) << 8) | (((_lo)&0xff) << 0))

#define  U_UNPACK_SHORT_LO_BYTE(_us) ((unsigned char)(_us))

#define  U_UNPACK_SHORT_HI_BYTE(_us) ((unsigned char)((_us) >> 8))


typedef union _U_short
{
     signed short     ss;  /*!< Interpretted as an   signed short        */
   unsigned short     us;  /*!< Interpretted as an unsigned short        */
   U_short_as_bytes    b;  /*!< Interpretted as bytes                    */
}
U_short;
/* ===================================================================== */









/* ===================================================================== *\
 |                                                                       |
 |                            INTS                                       |
 |                                                                       |
\* --------------------------------------------------------------------- */
#define _UNIONS_INT_AS_SHORTS
#include UNIONS_BIG_LITTLE
#undef  _UNIONS_INT_AS_SHORTS


#define  U_PACK_INT_WITH_SHORTS(_hi, _lo)                                 \
         ((((_hi)&0xffff) << 16) | (((_lo)&0xffff) << 0))


#define  U_PACK_INT_WITH_BYTES(_3, _2, _1, _0)                            \
         U_PACK_INT_WITH_SHORTS (U_PACK_SHORT_WITH_BYTES(_3, _2),         \
                                 U_PACK_SHORT_WITH_BYTES(_1, _0))

#define  U_UNPACK_INT_LO_SHORT(_ui) ((unsigned short)(_ui))
#define  U_UNPACK_INT_HI_SHORT(_ui) ((unsigned short)((_ui) >> 16))


typedef union _U_int
{
     signed int       si;  /*!< Interpretted as an   signed short        */
   uint32_t       ui;  /*!< Interpretted as an unsigned short        */
   U_int_as_shorts     s;  /*!< Interpretted as shorts                   */
}
U_int;
/* ===================================================================== */









/* ===================================================================== *\
 |                                                                       |
 |                          LONG LONG                                    |
 |                                                                       |
\* --------------------------------------------------------------------- */
#define _UNIONS_LONGLONG_AS_INTS
#include UNIONS_BIG_LITTLE
#undef  _UNIONS_LONGLONG_AS_INTS


/*
 | There is a bug in the GNUC 2.7 compiler.
 | It will not compile shifts on long longs.
 | However, there is an alternate method, which while not portable works on
 | the compiler. The GCC compilers allow one to put '(' ')' around any code
 | block, including one with '{' '}' and have the value of the block by
 | defined by the last value seen. Since this is the only compiler that we
 | need pull this trick on, there is on for this code to be portable. It
 | need only work for GNUC 2.7.
 |
 | There is one other sublity. When the extraction is done with the shift
 | operator, the sign extension of the extracted value follows the type
 | of the input variable. In this case, it does not. However, since the
 | likely destination of extracting an integer from a long long is an int
 | this should not case a problem. However, one should be careful on
 | how the output of this macro is treated, It always returns an unsigned
 | value.
*/
#if  defined (__GNUC__    )   && (__GNUC__       == 2) && \
     defined (__GNUC_MINOR__) && (__GNUC_MINOR__ == 7)


#define U_PACK_LL_WITH_INTS(_hi, _lo) \
({                                    \
   U_longlong ___ll;                  \
   ___ll.i.hi.ui = _hi;               \
   ___ll.i.lo.ui = _lo;               \
   ___ll.ull;                         \
})



#define U_UNPACK_LL_HI_INT(_ull) \
({                               \
   U_longlong ___ll;             \
   ___ll.ull = _ull;             \
   ___ll.i.hi.ui;                \
})

#else


#define U_PACK_LL_WITH_INTS(_hi, _lo)                                    \
        ((((unsigned long long)((_hi))&0xffffffffff) << 32) |            \
                             ((((_lo) &0xffffffffff) <<  0)))


#define U_UNPACK_LL_HI_INT(_ull) ((uint32_t)((_ull) >> 32))


#endif



#define U_PACK_LL_WITH_SHORTS(_3, _2, _1, _0)                            \
        U_PACK_LL_WITH_INTS (U_PACK_INT_WITH_SHORTS(_3, _2),             \
                             U_PACK_INT_WITH_SHORTS(_1, _0))



#define U_PACK_LL_WITH_BYTES(_7, _6, _5, _4, _3, _2, _1, _0)             \
        U_PACK_LL_WITH_INTS (U_PACK_INT_WITH_BYTES(_7, _6, _5, _4),      \
                             U_PACK_INT_WITH_BYTES(_3, _2, _1, _0))


#define U_UNPACK_LL_LO_INT(_ull) ((uint32_t)(_ull))


typedef union _U_longlong
{
   signed long long int sll;/*!< Interpretted as an   signed long long   */
 unsigned long long int ull;/*!< Interpretted as an unsigned long long   */
 U_longlong_as_ints       i;/*!< Interpretted as ints                    */

}
U_longlong;
/* ===================================================================== */



#endif

