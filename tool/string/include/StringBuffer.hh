// -*-Mode: C++;-*-
/**
@file
@brief Declares the StringBuffer class.
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
2011/01/12

@par Revision date:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(TOOL_STRING_STRINGBUFFER_HH)
#define      TOOL_STRING_STRINGBUFFER_HH

#include <cstdarg>
#include <vector>



namespace tool {

  namespace string {

    /** @brief Holds a dynamically allocated %string which can be extended.

        The %string will always have a terminating NUL. A StringBuffer
        is copyable and assignable.
    */
    class StringBuffer {
    public:

      enum {
        /** @brief Each buffer starts with at least this many bytes. */
        MINIMUM_ALLOCATION = 100
      };

      /** @brief Create zero-length %string with an initial allocation. */
      explicit StringBuffer(size_t alloc=MINIMUM_ALLOCATION);

      /** @brief Copy an existing %string. */
      explicit StringBuffer(const char *);

      /** @brief Free the %string buffer. */
      ~StringBuffer();

      /** @brief Add a %string directly, unformatted.

          The given %string is just appended to the existing one in the buffer.
      */
      StringBuffer& uadd(const char *string);

      /// @brief Format and append new content to the existing message %string.
      /// @return A reference to this object so that adds may be chained.
      StringBuffer& add(const char *format, ...);

      /// @brief Add with formatting using an already initialized va_list.
      /// @return A reference to this object so that adds may be chained.
      StringBuffer& vadd(const char *format, std::va_list);

      /** @brief The stored %string. */
      const char* str() const;

      /** @brief Equivalent to std::strlen(this->str()). */
      size_t strlen() const;

      /** @brief The number of bytes allocated for the %string buffer. */
      size_t allocation() const;

      /** @brief Exchange the internal data of two StringBuffers. */
      void swap(StringBuffer&);

      /** @brief Make the contents a null %string (but don't free any storage). */
      void clear();

    private:

      // The vector is NUL-padded so that m_buffer.size() ==
      // m_buffer.capacity(); This is so that we can add content just
      // be overwriting without needing to use push_back(). The price
      // we pay is having to track the actual %string size ourselves.
      std::vector<char> m_buffer;

      size_t m_strlen;
    };

  } // string

} // tool

#include "string/impl/StringBuffer-inl.hh"

#endif // TOOL_STRING_STRINGBUFFER_HH
