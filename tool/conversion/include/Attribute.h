#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H


/* --------------------------------------------------------------------- *//*!
  
  \file   Attribute.h
  \brief  Defines the \e __attribute__ feature to disappear on non-GCC
          platforms
  \author JJRussell - russell@slac.stanford.edu

\verbatim
   CVS $Id: Attribute.h,v 1.3 2005/10/01 00:33:25 russell Exp $
\endverbatim


  \par SYNOPSIS
   The GCC compiler allows functions, variables and types to assume
   various attributes through the appropriately named compiler feature
   called \e __attribute__. They were kind enough to define its usage in
   such a way that it can be easily \#define'd into oblivion on non-GCC
   compilers. That's what this file does.
   
  \par USAGE
   The underlying directive can be used directly as in the following
   example \par

  \code
   static void my_unused_routine (void) __attribute__((unused));
  \endcode

  \par
   Note the use of two sets of parens in this macro. It is this trick that
   allows one to easily \#define this directive away on platforms that do
   not support it.

  \par 
   One can also use the short-hand symbol \e ATTR_UNUSED_OK as in,\par
  
  \code
   static void my_unused_routine (void) ATTR_UNUSED_OK;
  \endcode

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

  \def    __attribute__(x)
  \brief  With non-GCC compilers, the \e __attribute__ feature is redefined
          as a NOP macro. With GCC compilers, this macro is not defined.
                                                                         */
/* --------------------------------------------------------------------- */
#ifndef __GNUC__
#define __attribute__(x)
#undef  ATTRIBUTE_SUPPORTED
#else
#define ATTRIBUTE_SUPPORTED
#endif
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def    ATTR_UNUSED_OK
  \brief  Symbol defined when using GCC compilers indicating that a
          static variable or static function is okay to be unused.

   With compiler warnings at their highest level, static variables and
   functions that are declared/defined but not used result in a warning
   message. Usually this is exactly what you want, it warns one of
   dead code or possible typos. However an include file that defines
   many static inlines can cause needless noise. It is unlikely that the
   user will need all the defined inlines, so some are bound to be unused.

   Example:
  \code

   static inline int foo (int bar) ATTR_UNUSED_OK;

  \endcode
                                                                         */
/* --------------------------------------------------------------------- */
#define ATTR_UNUSED_OK __attribute__((unused))
/* --------------------------------------------------------------------- */

#endif





