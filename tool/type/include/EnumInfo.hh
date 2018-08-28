// -*-Mode: C++;-*-
/**
@file EnumInfo.hh
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Tool

@par Abstract:
Declare the EnumInfo and related templates.

@author
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2011/01/26

@par Last commit:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(TOOL_TYPE_ENUMINFO_HH)
#define      TOOL_TYPE_ENUMINFO_HH

#include <cstddef>

namespace tool {
  namespace type {

    /** @brief The default declaration of the EnumInfo template. Specialize it
        to provide information about your enumerations.
        @see emax()
        @see emin()
        @see enext()
        @see eprev()
        @see ecount()
        @see estr()
    */
    template<typename Enumeration> struct EnumInfo {
    };

    /** @brief Use EnumInfo<T>::max to return the maximum member of enumeration T. */
    template<typename Enumeration> Enumeration emax() {return EnumInfo<Enumeration>::max;}

    /** @brief Use EnumInfo<T>::min() to return the minimum member of enumeration T. */
    template<typename Enumeration> Enumeration emin() {return EnumInfo<Enumeration>::min;}

    /** @brief Test for a reserved enumerator that stands for an invalid value. */
    template<typename Enumeration> bool evalid(Enumeration c) {return EnumInfo<Enumeration>::valid(c);}

    /** @brief Does an integer represent a valid enumerator? Test BEFORE converting to the enum type.  */
    template<typename Enumeration> bool evalid(int i) {return EnumInfo<Enumeration>::valid(i);}

    /** @brief Does an unsigned represent a valid enumerator. Test BEFORE converting to the enum type. */
    template<typename Enumeration> bool evalid(unsigned u) {return EnumInfo<Enumeration>::valid(u);}

    /** @brief Use EnumInfo<T>::next(e) to return the member after e of enumeration T. */
    template<typename Enumeration> Enumeration enext(Enumeration e) {return EnumInfo<Enumeration>::next(e);}

    /** @brief Use EnumInfo<T>::prev(e) to return the member before e of enumeration T. */
    template<typename Enumeration> Enumeration eprev(Enumeration e) {return EnumInfo<Enumeration>::prev(e);}

    /** @brief Use EnumInfo<T>::count() to return the number of enumerators in enum T. */
    template<typename Enumeration> std::size_t ecount() {return EnumInfo<Enumeration>::count;}

    /** @brief Use EnumInfo<T>::str(e) to return the C-string representation of member e in enum T. */
    template<typename Enumeration> const char* estr(Enumeration e)  {return EnumInfo<Enumeration>::tostr(e);}

  }; // type
}; // tool

#endif // TOOL_TYPE_ENUMINFO_HH
