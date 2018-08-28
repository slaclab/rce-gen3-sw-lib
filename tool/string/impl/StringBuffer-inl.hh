// -*-Mode: C++;-*-
/**
@file
@brief Defines the StringBuffer inlines.

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
2011/01/14

@par Revision date:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/

#include <algorithm>
#include <cstring>

namespace tool {

  namespace string {

    inline StringBuffer::StringBuffer(size_t alloc):
      m_buffer(std::max<size_t>(alloc, MINIMUM_ALLOCATION)),
      m_strlen(0)
    {}

    inline StringBuffer::StringBuffer(const char *start):
      m_buffer(),
      m_strlen(std::strlen(start))
    {
      // Allow room for expansion.
      m_buffer.resize(std::max<size_t>(2U*strlen() + 1U, MINIMUM_ALLOCATION));
      std::strcpy(&m_buffer[0], start);
    }

    inline StringBuffer::~StringBuffer() {}

    inline const char* StringBuffer::str() const {return &m_buffer[0];}

    inline size_t StringBuffer::strlen() const {return m_strlen;}

    inline size_t StringBuffer::allocation() const {return m_buffer.size();}

    inline void StringBuffer::swap(StringBuffer& other) {
      std::swap(m_strlen, other.m_strlen);
      m_buffer.swap(other.m_buffer);
    }

    inline void StringBuffer::clear() {m_buffer[0] = '\0'; m_strlen = 0;}

  }; // string

}; // tool
