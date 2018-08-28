// -*-Mode: C++;-*-
/**
@file rwall.cc
@brief Implements the functions readAll() and writeAll().
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
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/io/src/rwall.cc $

@par Credits:
SLAC
*/

#include <cstddef>
using std::ptrdiff_t;
using std::size_t;

#include <unistd.h>



#include "io/rwall.hh"

namespace tool {

  namespace io {

    ptrdiff_t readAll(int fd, void *buffer, size_t nbytes) {
      char *cbuff       = reinterpret_cast<char*>(buffer);
      char *const cend  = cbuff + nbytes;
      ptrdiff_t retval  = nbytes;
      while (cbuff != cend) {
	// cbuff: Next address to read to
	ptrdiff_t k = read(fd, cbuff, cend - cbuff);
	if (k == -1) {
	  retval = -1;
	  cbuff = cend;
	}
	else if (k == 0) {
	  retval = cend - cbuff;
	  cbuff = cend;
	}
	else {
	  cbuff += k;
	}
      }
      return retval;
    }

    ptrdiff_t writeAll(int fd, const char *buffer, size_t nbytes) {
      const char *cbuff       = reinterpret_cast<const char*>(buffer);
      const char *const cend  = cbuff + nbytes;
      ptrdiff_t retval  = nbytes;
      while (cbuff != cend) {
	// cbuff: Next address to write from.
	ptrdiff_t k = write(fd, cbuff, cend - cbuff);
	if (k == -1) {
	  retval = -1;
	  cbuff = cend;
	}
	else {
	  cbuff += k;
	}
      }
      return retval;
    }
  }; // io

}; // tool
