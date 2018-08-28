#ifndef INLINE_H
#define INLINE_H

/* ---------------------------------------------------------------------- *//*!

  \file  Inline.h
  \brief Defines a set of macros used to be used when defining inlines
  \author JJRussell - russell@slac.stanford.edu


\verbatim
    CVS $Id: Inline.h,v 1.4 2005/10/01 00:33:25 russell Exp $
\endverbatim

  \par SYNOPSIS
   Unfortunately there does not seem to be agreement on how to specify
   inline functions and their prototypes across the various compilers. In
   addition, compilers treat inlines differently depending on whether
   optimization is enabled or not.

  \par
   These files define 8 symbols to aid the user in supporting inline
   functions over multiple targets.

  \par USAGE
   There are two set of definitions. One for the prototype declarations
   and one for the actual function definitions. These definitions are
   definitions are dependent on whether this is included in the
   implementation file or a user file. Here implementation file is meant
   to be a file whose sole existence is to create a callable version of
   the inlines. This is necessary because some compilers (gcc, for example)
   will not inline under some circumstances (for example, when the
   optimization level is set to 0). Instead, an external call to the
   'inlined' routine is generated. The 'implementation file is used to
   generate a callable copy of these routine in this case. Unfortunately,
   the declarations of the inlines is different than in normal user code.
   Hence, one has two sets of definitions, one for normal usage and one
   used only in the implementation file.

  \par
   See the example below for procedure that works even when one is mixing
   inlines with other inlines, whether in implementation files or in
   user code.

  \par SYMBOLS DEFINED

  \par SET1 - USED in implementation files.
\verbatim
 INLINE_IMP_EXT_PROTO - Declares prototypes, inlines with public interfaces.
 INLINE_IMP_EXT_FNC   - Declares public inline functions.
 INLINE_IMP_EXP_PROTO - Declares prototypes, inlines with public interfaces.
 INLINE_IMP_EXP_FNC   - Declares public inline functions.
 INLINE_IMP_LCL_PROTO - Declares prototypes, inlines with private interfaces.
 INLINE_IMP_LCL_FNC   - Declares private inline functions.
\endverbatim

  \par SET 2 - USED in USER programs.
\verbatim
 INLINE_USR_EXT_PROTO - Declares prototypes, inlines with public interfaces.
 INLINE_USR_EXT_FNC   - Declares public inline functions.
 INLINE_USR_EXP_PROTO - Declares prototypes, inlines with public interfaces.
 INLINE_USR_EXP_FNC   - Declares public inline functions.
 INLINE_USR_LCL_PROTO - Declares prototypes, inlines with private interfaces.
 INLINE_USR_LCL_FNC   - Declares private inline functions.
\endverbatim

  \par  EXAMPLE
   Suppose one wishes to create an inline version of a routine which
   adds or subtracts 2 integers. One would create the interface include
   file MTH.ih.

  \par
   In order to be able to include one set of inlines in another set, one
   must make sure that each has the proper declarations. The following
   procedure works. \par

  \code

      #ifndef   MTH_IH
      #define   MTH_IH
      .
      #include "conversion/inline.h"

  \endcode

  \par
   The following correctly defines the symbols properly for both USER
   includes and implementation file includes. The convention adopted here
   is that the implementation file will define the symbol
   \a MTH_IMPLEMENTATION_FILE. \par

  \code

      #ifdef  MTH__IMPLEMENTATION_FILE

      #define MTH__EXT_PROTO INLINE_IMP_EXT_PROTO
      #define MTH__EXT_FNC   INLINE_IMP_EXT_FNC
      #define MTH__LCL_PROTO INLINE_IMP_LCL_PROTO
      #define MTH__LCL_FNC   INLINE_IMP_LCL_FNC

      #else

      #define MTH__EXT_PROTO INLINE_USR_EXT_PROTO
      #define MTH__EXT_FNC   INLINE_USR_EXT_FNC
      #define MTH__LCL_PROTO INLINE_USR_LCL_PROTO
      #define MTH__LCL_FNC   INLINE_USR_LCL_FNC

      #endif


      / * Declare the prototypes  * /
      MTH__EXT_PROTO int MTH__add (int a, int b);
      MTH__EXT_PROTO int MTH__sub (int a, int b);
      MTH__LCL_PROTO int mth__add (int a, int b);


      / * Drop the implementations * /
      MTH__EXT_FNC   int MTH_add (int a, int b)
      {
         return mth__add (a, b);
      }



      MTH__EXT_FNC   int MTH_sub (int a, int b)
      {
      return mth__add (a, -b);
      }



      MTH__LCL_FNC   int mth__add (int a, int b)
      {
         return a + b;
      }

  \endcode

  \par
      The user of this facility would then do \par

  \code

      #include MTH.ih

      int myTest (a, b)
      {
          printf ("The sum of %d + %d = %d\n", a, b, MTH__add (a, b));
          printf ("The dif of %d - %d = %d\n", a, b, MTH__sub (a, b));
      }

  \endcode

  \par
   If one wishes to produce a more traditional callable interface, then
   define the interface file MTH.h \par

  \code

      #ifndef MTH_H
      #define MTH_H

      / * Define the prototypes * /
      int MTH_add (int a, int b);
      int MTH_sub (int a, int b);


      #endif

  \endcode

  \par
   and the implementation file MTH.c \par

  \code

      #include "conversion/alias.h"
      #include "conversion/inline.h"

      #define   MTH__IMPLEMENTATION_FILE   -- This is the implementation file
      #include "conversion/MTH.ih"                -- Makes callable versions of
                                           -- MTH__add and MTH__sub

      ALIAS_BEG (MTH)                      -- This aliases ...
      ALIAS_FNC (int, MTH__add, MTH_add)   -- ...  MTH_add with MTH__add
      ALIAS_FNC (int, MTH__sub, MTH_sub)   -- ...  MTH_sub with MTH__sub
      ALIAS_END (MTH)

  \endcode

  \par
   This scheme allows the user to mix and match between callable and
   inline versions. In this example the MTH__xxx routines are the inlines
   and the MTH_xxx are the callable versions.

  \par
   Okay, one final sublity. Note that there are two versions of defining
   external routines, the \a _EXT_ and \a _EXP_ symbols. The _EXP_ symbols
   always define the inlines as static routines. This avoids the problem
   of having to provide an implementation file for the inlines when the
   compiler optimization is turned off. The downside of this is that a
   version of \a all the inlines is included in the including file. This
   fact leads to two undesirable consequences. The first is obvious, just
   the code bloat associated with having a bunch of unused/unneeded code
   dropped down. The second is that with the compiler warning flags set
   at their maximum noise level, these unused static routines get reported
   as warnings.

  \par
   The best solution I have come up with (and it is not great) is that
   to include a \e ATTR_UNUSED_OK with the definition of the routine. The
   reason that this is not great, is that this is a GCC feature only.
   (Note that the \e attribute feature will not cause to the code to break
   on non-GCC compilers, but then it will not do it's assigned job.) So far
   this has accidentally worked out okay. The SUN scc compiler does not
   issue warnings for unused statics.

  \par
   The equivalent exmample using the EXP symbols in the implementation
   file is \par

  \code

   #include "PBI/Inline.h"

     #define MTH__IMPLEMENTATION_FILE
     #define MTH__add MTH_add
     #define MTH__sub MTH_sub

     # include "MTH.ih

   #endif

 \endcode

 \par
  This bit of pre-processor syntax-sugar causes the _EXP_ symbols to
  be redefined as regular callable routines (no inlining). The names
  of the routines are then changed to the callable versions.

 \par
  Which should one use? I am recommending the second, but there are
  situations where the first may be more appropriate.

 \warning
  Finally, there are of the limitations of this method. If an inline
  routine in a package actually calls another inline in the same
  package and one uses the trick above to create callable versions,
  be aware that the called inlined will in fact no longer be dropped
  inline, but will generate a real call. Afterall that's what the
  \#defines and xxx_IMPLEMENTATION_FILE symbol did, change the inlines
  into regular old callable routines.

 \par
  In this case, I think the only recourse is to define a truly static
  inline which both routines call. This underlying routine will always
  be an inline. \par

 \note
  Sorry that inlines are so confusing. Unfortunately the inventors of
  inlines did not define it rigorously enough or think through all the
  consequences.
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

    \def   INLINE_IMP_EXT_PROTO
    \brief External function prototype, used in the
           implementation file
                                                                         *//*!
    \def   INLINE_IMP_EXT_FNC
    \brief External function declaration, used in the
           implementation file
                                                                         *//*!
    \def   INLINE_IMP_EXP_PROTO
    \brief External function prototype, used in the
           implementation file. This differs from the EXT in that it
           declares the routines static. This means one may get
           multiple copies, but ensures that the copy stays with
           the generated code.
                                                                         *//*!
    \def   INLINE_IMP_EXP_FNC
    \brief External function declaration, used in the
           implementation file. This differs from the EXT in that it
           declares the routines static. This means one may get
           multiple copies, but ensures that the copy stays with
           the generated code.
                                                                         *//*!
    \def   INLINE_IMP_LCL_PROTO
    \brief Internal (static) function prototype, used in the
           implementation file
                                                                         *//*!
    \def   INLINE_IMP_LCL_FNC
    \brief Internal (static) function declaration, used in the
           implementation file
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

    \def   INLINE_USR_EXT_PROTO
    \brief External function prototype, used in the user code.
                                                                         *//*!
    \def   INLINE_USR_EXT_FNC
    \brief External function declaration, used in the user code
                                                                         *//*!
    \def   INLINE_USR_EXP_PROTO
    \brief Exportable function prototype, used in the user code.
           This declares the routine to be an inline static.
                                                                         *//*!
    \def   INLINE_USR_EXP_FNC
    \brief Exportable function declaration, used in the user code
           This declares the routine to be an inline static.
                                                                         *//*!
    \def   INLINE_USR_LCL_PROTO
    \brief Internal (static) function prototype, used in the user code
                                                                         *//*!
    \def   INLINE_USR_LCL_FNC
    \brief Internal (static) function declaration, used in the user code
                                                                         */
/* --------------------------------------------------------------------- */




#if   defined(__GNUC__)

#   include "conversion/impl/Inline.h.xx-xxx-gcc"

#elif defined(__SUNPRO_CC)

#   include "conversion/impl/Inline.h.xx-xxx-scc"

#else

#   error __FILE__, unsupported platform

#endif

#endif
