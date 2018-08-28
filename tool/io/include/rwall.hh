// -*-Mode: C++;-*-
/**
@file
@brief Declares the functions readAll() and writeAll().
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
2010/04/01

@par Last commit:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(TOOL_IO_RWALL_HH)
#define      TOOL_IO_RWALL_HH

#include <cstddef>

namespace tool {

  namespace io {

    /// @brief Attempt to read all the bytes requested before returning.
    /// @param[in] fd The file descriptor from which to read.
    /// @param[in] buffer The buffer into which to read.
    /// @param[in] nbytes The number of bytes to read.
    /// @return The number of bytes transferred, 0 if EOF or -1 if there was an error.
    ///
    /// The POSIX read() function may not transfer all the bytes
    /// requested even if it encounters no read error or end-of-file
    /// condition. For some devices, e.g., pipes it may simply take
    /// what's available in a system buffer and return. This function
    /// will call read() repeatedly in an attempt to transfer all the
    /// bytes requested.
    std::ptrdiff_t readAll(int fd, void *buffer, std::size_t nbytes);

    /// @brief Attempt to write all the bytes requested before returning.
    /// @param[in] fd The file descriptor to which to write.
    /// @param[in] buffer The buffer from which to write.
    /// @param[in] nbytes The number of bytes to write.
    /// @return The number of bytes transferred or -1 if there was an error.
    ///
    /// The POSIX write() function may not transfer all the bytes
    /// requested even if encounters no write error.  For some
    /// devices, e.g., pipes it may simply fill the space remaining in
    /// a system buffer and return. This function will call write()
    /// repeatedly in an attempt to transfer all the bytes requested.
    std::ptrdiff_t writeAll(int fd, const char *buffer, std::size_t nbytes);
  }; // io

}; // tool

#endif // TOOL_IO_RWALL_HH
