#ifndef ALIAS_H
#define ALIAS_H


/* --------------------------------------------------------------------- *//*!

  \file  Alias.h
  \brief Defines a set of macros used to create function aliases
  \author JJRussell - russell@slac.stanford.edu

\verbatim
  CVS $Id: Alias.h,v 1.5 2005/10/01 00:33:25 russell Exp $
\endverbatim

  \par SYNOPSIS
   Provides a uniform way to declare ALIASs or SYNONYMS for global
   symbols.  The implementation is platform dependant, however the macros
   are universal.

  \par USAGE
   Suppose one has a facility \e ORG_ containing the routines \e ORG_get,
   \e ORG_set. Aliases \e ALIAS_get and \e ALIAS_set can be created using
   the following prescription. \par

  \code
   ALIAS_BEG(ORG_)
   ALIAS_FNC(int, ORG_get, ALIAS_get)
   ALIAS_FNC(int, ORG_set, ALIAS_set)
   ALIAS_END(ORG_)
  \endcode

  \par TARGET PLATFORMS
   All FSW supported platforms.

  \par LIMITATIONS
   Some platforms demand that the symbol being aliased be defined in the
   same file where the aliasing is occurring. This is an unfortunate
   limitation. One cannot, for example, alias a symbol to a system
   routine.
                                                                         */
/* --------------------------------------------------------------------- */





/* ---------------------------------------------------------------------- *\
 *
 * HISTORY
 * -------
 *
 * DATE     WHO WHAT
 * -------- --- ---------------------------------------------------------
 * 09.28.05 jjr Corrected documentation formating for Doxygen 1.4.4
 * 09.27.05 jjr Had to add the type of the returned value to the macro
 *              for the new GCC compiler (>3.4). This is a non-backwardly
 *              compatiable change, but, at least for GCC compilation,
 *              the declaring of aliased symbols now uses the compiler
 *              supported attribute specification.
 * 09.27.05 jjr Added history log
 *
\* ---------------------------------------------------------------------- */





/* --------------------------------------------------------------------- */
#if   defined(__GNUC__) && (defined(__linux__) || defined(_ARCH_PPC) || defined(__arm__))

   /* -- Support the GCC compiler on a PowerPC or Linux -- */
#  include "conversion/impl/Alias.h.xx-xxx-gcc"

#elif defined(__CYGWIN__) && defined(__GNUC__)

   /* -- Support the GCC compiler on Windows platform -- */
#  include "conversion/impl/Alias.h.cx-xxx-gcc"

#elif defined(__sun)

#  if   defined(__SUNPRO_CC)

      /* -- Support the SUNCC compiler on a SUN -- */
#     include "conversion/impl/Alias.h.xx-xxx-scc"

#  elif defined(__GNUC__)

      /* -- Support the GCC compiler on a SUN -- */
#     include "conversion/impl/Alias.h.sx-xxx-gcc"

#  elif

      /*  -- No can do, Unknown compiler on a SUN -- */
#     error __FILE__: Unknown compiler for SUN, no support


#  endif

#else

#  error __FILE__: Unknown target, no support

#endif
/* --------------------------------------------------------------------- */



/* --------------------------------------------------------------------- *//*!

  \def    ALIAS_BEG(_container)
  \brief  Introduces a group of aliases

  \param _container A completely arbitrary, but unique name

   The container name is completely arbitrary. On some compilers (scc on
   SOLARIS) aliases can only be defined within the context of a routine.
   The parameter \a _container is the name of the encapsulating routine.
   This is one of those interesting features that seemingly has no real
   reason, but...
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def   ALIAS_FNC(_return_type, _name, _alias)
  \brief Defines an alias for a function

  \param _return_type The return type of function call
  \param _name        The original name of the symbol
  \param _alias       The aliased name for the symbol

  \par
   This macro is self-explanatory. For those curious about the calling
   convention (i.e. why isn't it ALIAS_FNC(_alias, _name)), the thought
   was that if one introduced a macro that defined multiple aliases for
   a single symbol the natural extension would be
   ALIAS_FNC2(_name, alias1, alias2). Just want to make sure you know
   there is a method to my madness.

  \par
   The return type argument was added 09.27.05 for the new GCC (>3.4)
   compiler. This compiler now removes \e all references to the static
   functions that are not referenced. This means no symbol appears in
   for this function in the assembler. Because of this, the aliased
   symbol is set equal to a symbol that, as far as the assembler is
   concerned, no longer exists. The solution is to use the __attribute__
   specifier to declare the alias. This is certainly a better than trying
   to fake the compiler out with hidden assembler. However, the downside
   is that this syntax demands that the return type be specified.
   Unfortunately, this information is not available in the current
   macro, so a new argument must be added.
                                                                         */
/* --------------------------------------------------------------------- */




/* --------------------------------------------------------------------- *//*!

  \def    ALIAS_END(_container)
  \brief  Closes a group of aliases

  \param _container A completely arbitrary, but unique name.

   The container name is completely arbitrary. As explained in \e ALIAS_BEG,
   on some compilers (scc on SOLARIS)  aliases can only be defined within
   the context of a routine. The parameter \a _container is the name of
   the encapsulating routine. The name is not actually used by this macro
   for any real purpose, but it cleanly highlights the declaration of a
   block of aliases.
                                                                         */
/* --------------------------------------------------------------------- */


#endif

