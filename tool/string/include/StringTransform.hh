// -*-Mode: C++;-*-
/**
@file
@par Abstract:
@brief Transform various primitive types to std::%string.

Code ported and extended from service/dynalink/src/Misc.hh

@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@author
Jim Panetta <panetta@slac.stanford.edu>

@par Date created:
2010/10/22

@par Last commit:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/


#ifndef TOOL_STRING_STRINGTRANSFORM_HH
#define TOOL_STRING_STRINGTRANSFORM_HH

#include <string>
#include <stdio.h>

namespace tool {
namespace string{

  using std::string;

  /** @internal @brief This is specialized to provide various format codes.*/
  template<typename T, bool hex=false>
  struct X2a_format {};

  /** @internal @brief Format a char using %%d.*/
  template<>
  struct X2a_format<char> {
    static const char *value() { return "%d"; }
  };
  
  /** @internal @brief Format an unsigned char using %%u.*/
  template<>
  struct X2a_format<unsigned char> {
    static const char *value() { return "%u"; }
  };

  /** @internal @brief Format a short using %%d.*/  
  template<>
  struct X2a_format<short> {
    static const char *value() { return "%d"; }
  };
  
  /** @internal @brief Format an unsigned short using %%u.*/
  template<>
  struct X2a_format<unsigned short> {
    static const char *value() { return "%u"; }
  };

  /** @internal @brief Format an int using %%d.*/
  template<>
  struct X2a_format<int> {
    static const char *value() {return "%d";}
  };

  /** @internal @brief Format an unsigned using %%u.*/
  template<>
  struct X2a_format<unsigned> {
    static const char *value() {return "%u";}
  };

  /** @internal @brief Format a long using %%ld.*/
  template<>
  struct X2a_format<long> {
    static const char *value() {return "%ld";}
  };

  /** @internal @brief Format a long long using %%lld.*/
  template<>
  struct X2a_format<long long> {
    static const char *value() {return "%lld";}
  };

  /** @internal @brief Format an unsigned long using %%lu.*/
  template<>
  struct X2a_format<unsigned long> {
    static const char *value() {return "%lu";}
  };

  /** @internal @brief Format an unsigned long long using %%llu.*/
  template<>
  struct X2a_format<unsigned long long> {
    static const char *value() {return "%llu";}
  };

  /** @internal @brief Format a char with 0x%%02x. */
  template<>
  struct X2a_format<char, true> {
    static const char *value() { return "0x%02x"; }
  };
  
  /** @internal @brief Format an unsigned char with 0x%%02x. */
  template<>
  struct X2a_format<unsigned char, true> {
    static const char *value() { return "0x%02x"; }
  };
  
  /** @internal @brief Format a short with 0x%%04x. */
  template<>
  struct X2a_format<short, true> {
    static const char *value() { return "0x%04x"; }
  };
  
  /** @internal @brief Format an unsigned short with 0x%%04x. */
  template<>
  struct X2a_format<unsigned short, true> {
    static const char *value() { return "0x%04x"; }
  };
  
  /** @internal @brief Format a int with 0x%%08x. */
  template<>
  struct X2a_format<int, true> {
    static const char *value() {return "0x%08x";}
  };

  /** @internal @brief Format an unsigned with 0x%%08x. */
  template<>
  struct X2a_format<unsigned, true> {
    static const char *value() {return "0x%08x";}
  };

  /** @internal @brief Format a long with 0x%%08x. */
  template<>
  struct X2a_format<long, true> {
    static const char *value() {return "0x%08x";}
  };

  /** @internal @brief Format an unsigned long with 0x%%08x. */
  template<>
  struct X2a_format<unsigned long, true> {
    static const char *value() {return "0x%08x";}
  };

  /** @internal @brief Format a long long with 0x%%016llx. */
  template<>
  struct X2a_format<long long, true> {
    static const char *value() {return "0x%016llx";}
  };

  /** @internal @brief Format an unsigned long long with 0x%%016llx. */
  template<>
  struct X2a_format<unsigned long long, true> {
    static const char *value() {return "0x%016llx";}
  };

  /** @internal @brief Format a pointer with %%p. */
  template<typename T>
  struct X2a_format<T*> {
    static const char *value() {return "%p";}
  };

  /** @brief Format a value of integral type, producing a decimal %string. */
  template<typename T>
  std::string x2a(const T& x) {
    char buf[51];
    snprintf(buf, sizeof buf, X2a_format<T>::value(), x);
    return string(buf);
  }

  /** @brief Format a value of integral type, producing a hex %string with leading 0x.*/
  template<typename T>
  std::string x2h(const T& x) {
    char buf[51];
    snprintf(buf, sizeof buf, X2a_format<T, true>::value(), x);
    return string(buf);
  }

}
}

#endif
