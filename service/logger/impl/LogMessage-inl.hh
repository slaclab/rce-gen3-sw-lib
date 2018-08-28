// -*-Mode: C++;-*-
/**
@file LogMessage-inl.hh
@verbatim
                               Copyright 2010
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
Service

@par Abstract:
Implements the LogMessage inlines.

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2011/01/14

@par Last commit:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/

namespace service {

  namespace logger {

    inline size_t LogMessage::initialBufferSize() {return m_initialBufferSize;}

    inline void LogMessage::initialBufferSize(size_t s) {m_initialBufferSize = s;}

    inline LogMessage& LogMessage::uadd(const char *more) {m_buffer.add(more); return *this;}
  }; // logger
}; // service
