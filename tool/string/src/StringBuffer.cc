// -*-Mode: C++;-*-
/**
@file
@brief Implements the StringBuffer class (non-inline part).
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Tool

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2010/03/25

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/string/src/StringBuffer.cc $

@par Credits:
SLAC
*/

#include <cstdarg>
using std::va_list;

#include <stdio.h>

#include <vector>
using std::vector;



#include "string/StringBuffer.hh"

namespace tool {

  namespace string {

    StringBuffer& StringBuffer::vadd(const char *format, std::va_list ap) {

      // Copy the argument pointer in case we have to call
      // vsnprintf() a second time.
      va_list apLocal;
      va_copy(apLocal, ap);

      // With the first attempt we exhaust the arguments available
      // through ap. Limit counts the trailing '\0', n doesn't.
      int limit = allocation() - strlen();
      int n = vsnprintf(&m_buffer[strlen()], limit, format, ap);

      // If we have to make a second attempt we can't use va_start()
      // to re-initialize ap; va_start() is only usable when the
      // stack frame of the original variadic function is on top.
      if (n >= limit) {
	m_buffer.resize(strlen() + 2 * n); // Try to leave room for further adds.
	limit = allocation() - strlen();
	n = vsnprintf(&m_buffer[strlen()], limit, format, apLocal);
      }
      m_strlen += n;

      // Each va_copy() must be matched up with a va_end().
      va_end(apLocal);

      return *this;
    }

    StringBuffer &StringBuffer::add(const char *format, ...) {
      // Make the argument list pointer point to the first optional
      // argument.
      va_list ap;
      va_start(ap, format);

      // I don't know if it's required but I think it best to use
      // va_end() in the same stack frame as the corresponding
      // va_start(); I'm told that some implementations of va_start()
      // make changes to the stack frame that must be undone by
      // va_end() before the function can return.
      vadd(format, ap);
      va_end(ap);
      return *this;
    }

    StringBuffer& StringBuffer::uadd(const char *more) {
      const size_t newlen(strlen() + std::strlen(more));
      if (allocation() <= newlen) m_buffer.resize(2*newlen+1);
      std::strcpy(&m_buffer[strlen()], more);
      m_strlen = newlen;
      return *this;
    }

  }; // string

}; // tool
