/* --------------------------------------------------------------------- *//*!

   
  \file   Unions_big.h
  \brief  Big endian implementation of Union.h
  \author JJRussell - russell@slac.stanford.edu

\verbatim
  CVS $Id
\endverbatim

  Defines the implemetation of a set of useful unions between long long,
  ints, shorts and chars (bytes) which allow one to access subfields in
  a consistent fashion across big and little endian machines.
  This is the big endian implementaion.
                                                                         */
/* --------------------------------------------------------------------- */


/*
 | This file contains the big endian implementations of 3 unions. To get
 | the definition to be dropped in the parent include file, one must
 | define a symbol. This is because the definition of all but the first
 | is dependent on the previous. This allows the structures to be built
 | up in phases.
 |
 |      UNION NAME             SYMBOL
 |      U_short_as_bytes       UNIONS_SHORT_AS_BYTES
 |      U_int_as_shorts        UNIONS_INT_AS_SHORTS
 |      U_longlong_as_ints     UNIONS_LONGLONG_AS_INTS
 |
 | Note that there is not the traditional #ifndef at the top of this
 | file protecting against multiple includes. That is because it is
 | meant to be multiply included, first defining UNIONS_SHORT_AS_BYTES,
 | then UNIONS_INT_AS_SHORTS and finally UNIONS_LONGLONG_AS_INTS.
*/

#ifdef _UNIONS_SHORT_AS_BYTES

typedef struct _U_short_as_bytes
{
   union _U_byte  hi; /*!< Access the numerically least significant byte */
   union _U_byte  lo; /*!< Access the numerically most  significant byte */
}
U_short_as_bytes;

#endif


#ifdef _UNIONS_INT_AS_SHORTS

typedef struct _U_int_as_shorts
{
  union _U_short hi; /*!< Access the numerically least significant short */
  union _U_short lo; /*!< Access the numerically most  significant short */
}
U_int_as_shorts;

#endif


#ifdef _UNIONS_LONGLONG_AS_INTS

typedef struct _U_longlong_as_ints
{
      
 union _U_int    hi; /*!< Access the numerically least significant int   */
 union _U_int    lo; /*!< Access the numerically most  significant int   */
}
U_longlong_as_ints;

#endif
