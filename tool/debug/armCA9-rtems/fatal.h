// -*-Mode: C;-*-
/**
@file
@brief Declare RTEMS fatal error processing routines for ARM.
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
\$Date: 2014-05-15 13:33:40 -0700 (Thu, 15 May 2014) $ by \$Author: panetta $.

@par Credits:
SLAC
*/
#if !defined(TOOL_DEBUG_FATAL_H)
#define      TOOL_DEBUG_FATAL_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <rtems.h>

#include "debug/registers.h"

  /** @brief Is multi-tasking enabled? */
  bool dbg_taskingIsEnabled(void);
  void dbg_setTaskingIsEnabled(bool);

  /** @brief 0 = not in a fatal error handler, 1 = one level deep, etc. */
  int dbg_fatalNestingLevel(void);

  /** @brief Print useful information about a fatal error. */
  void dbg_printFatal(rtems_fatal_source source,
                      int is_internal,
                      rtems_fatal_code code,
                      dbg_PrintFunc print);

  /** @brief Print information stored by the default exception handlers. */
  void dbg_printExceptionFrame(const CPU_Exception_frame*, dbg_PrintFunc print);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif
