// -*-Mode: C++;-*-
/**
@file
@brief Declare our RTEMS-API extension installer.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2014/02/16

@par Last commit:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(TOOL_DEBUG_EXTENSION_H)
#define      TOOL_DEBUG_EXTENSION_H
#include <rtems.h>


#if defined(__cplusplus)
extern "C" {
#endif

  /** @brief The installer for our extension to the classic RTEMS API. */
  void dbg_installExtension(void);

#if defined(__cplusplus)
} /* extern "C" */
#endif
#endif
