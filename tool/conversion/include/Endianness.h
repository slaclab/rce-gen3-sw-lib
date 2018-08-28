#ifndef ENDIANNESS_H
#define ENDIANNESS_H



/* ---------------------------------------------------------------------- *//*!

  \file  Endianness.h
  \brief Provides definitions of the ENDIANNESS of the target machine. The
         symbols \a ENDIANNESS_IS_LITTLE and \a ENDIANNESS_IS_BIG should be
         used in preprocessing directives to determine the endianness of
         the machine, while the symbol \a ENDIANNESS can be more easily
         used in 'C' code.
  \author JJRussell - russell@slac.stanford.edu

\verbatim

   CVS $Id: Endianness.h,v 1.4 2005/10/01 00:33:25 russell Exp $
\endverbatim

  \par SYNOPSIS
   One of two symbols are defined
      - \a ENDIANNESS_IS_LITTLE
      - \a ENDIANNESS_IS_BIG.

  \par
   As a convenience to the programmer, an overall symbol, \a ENDIANNESS
   is also defined. It will take on one of the three values.

     - ENDIANNESS_K_UNKNOWN
     - ENDIANNESS_K_LITTLE
     - ENDIANNESS_K_BIG

  \par USAGE
   One should use this as follows \par

  \code
    #if   ENDIANNESS_IS_LITTLE

      / * Do little endian kind of things * /

    #elif ENDIANNESS_IS_BIG

      / * Do big endian kind of things * /

    #else

    #error _FILE_ Endianness not determined in this file

    #endif
   \endcode

   \par
    It is recommended that both BIG and LITTLE conditions are tested for.
    This can save one from embarrassing typographical errors. For example,
    consider the consequences of the following \par

   \code

    #if ENDIANESS_IS_LITTLE

       / * Do little endian kind of things * /

    #else

       / * Do big endian kind of things * /

    #endif

   \endcode

   \par
    It takes a keen eye to note the misspelling of \a ENDIANNESS_IS_LITTLE
    as \a ENDIANESS_IS_LITTLE. Since there is no such symbol as
    \a ENDIANESS_IS_LITTLE, the 'big' endian kind of things will always be
    done.

   \par
    It is also recommended that one use \e \#if preprocessor directive over
    the \#ifdef form. Either works with the current implementation, but
    future implementations reserves the right to support only the \#if
    form.

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




/* --------------------------------------------------------------------- *//*!

  \def    ENDIANNESS_IS_LITTLE
  \brief  Defined and set to a non-zero value, \a ENDIANNESS_K_LITTLE, if
          the target is a little endian machine.
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def    ENDIANNESS_IS_BIG
  \brief  Defined and set to a non-zero value, \a ENDIANNESS_BIG, if
          the target is a big endian machine.
                                                                         */
/* --------------------------------------------------------------------- */





/* --------------------------------------------------------------------- *//*!

  \def    ENDIANNESS_K_UNKNOWN
  \brief  Value of ENDIANNESS the endianness of the target is undefined.
                                                                         */
/* --------------------------------------------------------------------- */
#define ENDIANNESS_K_UNKNOWN    0
/* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- *//*!

  \def    ENDIANNESS_K_LITTLE
  \brief  Value of ENDIANNESS if the target is a little endian machine.
                                                                         */
/* --------------------------------------------------------------------- */
#define ENDIANNESS_K_LITTLE     1
/* --------------------------------------------------------------------- */


/* --------------------------------------------------------------------- *//*!

  \def    ENDIANNESS_K_BIG
  \brief  Value of ENDIANNESS if the target is a big endian machine
          machine.
                                                                         */
/* --------------------------------------------------------------------- */
#define ENDIANNESS_K_BIG        2
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def    ENDIANNESS
  \brief \a ENDINANESS assumes one of the values,
          \li \a ENDIANNESS_K_LITTLE
          \li \a ENDIANNESS_K_BIG
          \li \a ENDIANNESS_K_UNKNOWN.


  \note
   Since DOXYGEN does not have access to the full set of compile time
   preprocessor symbols, \a ENDIANNESS will appear to be defined as
   \a ENDIANNESS_K_UNKNOWN.

  \warning
   It is not recommended that this symbol be used in preprocessor
   directives to determine the endianness of the machine. Use
   \a ENDIANNESS_IS_LITTLE or \a ENDIANNESS_IS_BIG. Why? To do this
   correctly one must use the following long-winded style: \par

  \code
   if   defined (ENDIANNESS) && (ENDIANNESS == ENDIANNESS_K_LITTLE)
   {
      .
      .
   }
   elif defined (ENDIANNESS) && (ENDIANNESS == ENDIANNESS_K_BIG)
   {
      .
      .
   }
   else
   {
      .
      / * Endianness unknown * /
   }
  \endcode

  \par
   The check for \a ENDIANNESS being defined is mandatory for robustness.
   If the user forgets to include "PBI/Endiannness.h", all the \e ENDIANNESS
   symbols are, in fact, undefined. Without the check that \a ENDIANNESS
   is defined, the simple check of (\a ENDIANNESS == \a ENDIANNESS_K_LITTLE)
   will \b always succeed, causing the big endian if clause to always be
   taken. If the code is developed on a big endian machine, it is only
   when the code is ported to a little endian machine will the problem
   show up as malfunctioning code which the poor developer will eventually
   trace to the ill-constructed endianness determination.
                                                                         */
/* --------------------------------------------------------------------- */

#if   (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)


    #   define ENDIANNESS_IS_LITTLE ENDIANNESS_K_LITTLE
    #   define ENDIANNESS           ENDIANNESS_K_LITTLE


  #elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

    #    define ENDIANNESS_IS_BIG    ENDIANNESS_K_BIG
    #    define ENDIANNESS           ENDIANNESS_K_BIG

  #else

    #    define ENDIANNESS           ENDIANNESS_K_UNKNOWN

      /*
       | From a programming standpoint this not necessary, but since
       | DOXYGEN does not have access to the full set of compile time
       | preprocessor symbols, the first two clauses will not define
       | ENDIANNESS_IS_LITTLE or ENDIAN_IS_BIG, causing DOXYGEN to complain
       | about documented, but undefined symbols. In effect, these two
       | statements are just to keep DOXYGEN from spitting out compliants.
      */
    #    define ENDIANNESS_IS_LITTLE ENDIANNESS_K_UNKNOWN
    #    define ENDIANNESS_IS_BIG    ENDIANNESS_K_UNKNOWN

    #    undef ENDIANNESS_IS_LITTLE
    #    undef ENDIANNESS_IS_BIG

    #    error "Endianness.hh: Cannot determine the endianness of the machine"

   #endif
#endif
/* --------------------------------------------------------------------- */













