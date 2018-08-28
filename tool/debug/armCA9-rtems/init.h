// -*-Mode: C;-*-
/**
@file
@brief Declare the debug-init API for ARM RTEMS.
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
2014/02/09

@par Last commit:
\$Date: 2014-07-31 17:05:57 -0700 (Thu, 31 Jul 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3597 $

@par Credits:
SLAC
*/
#if !defined(TOOL_DEBUG_INIT_H)
#define      TOOL_DEBUG_INIT_H

#if defined(__cplusplus)
extern "C" {
#endif

  /** @brief Set up the syslog and simple console support early in system initialization. */
  void dbg_earlyInit(void);

  /** @brief Do the remaining init. of the debug package when multi-threading is available. */
  void dbg_lateInit(void);

#if defined(__cplusplus)
} /* extern "C" */
#endif
#endif
