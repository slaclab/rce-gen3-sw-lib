// -*-Mode: C;-*-
/**
@file
@brief Encoding and decoding of status codes.
@ingroup configurationsystem

Status codes returned from core system facilities contain two fields:
a facility number and an error code.

Status codes from RTEMS directives are integers small enough to
fit within the error code field of the status codes described in this
file. Thus they will automatically appear to have facility number
zero provided that the error code field occupies the lowest-order
part of the status code. For this reason facility number zero
mustn't be used for any other purpose and the error code field
must not be moved.

The status value of zero is special in that it's the value
used to signal the successful completion of an operation, no matter
what facility the operation is executed by.

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
2014/02/20

@par Last commit:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/
#if !defined(CONFIGURATION_SYSTEM_H)
#define      CONFIGURATION_SYSTEM_H

#if defined(__cplusplus)
extern "C" {
#endif

/** @brief Status code field definitions. */
typedef enum {
  STS_M_FACILITY = 0x00ff0000,  /**< @brief In-place mask for the facility number.*/
  STS_V_FACILITY = 16,          /**< @brief Shift needed to right-justify the facility number. */

  STS_M_ERRCODE  = 0x0000ffff,  /**< @brief In-place mask for the error code.*/
  STS_V_ERRCODE  = 0            /**< @brief Shift needed to right-justify the error code. */
} sts_Fields;

/** @brief Create a status code from a facility number and an error code. */
#define STS_ENCODE(facility, errcode) \
((facility << STS_V_FACILITY) | (errcode << STS_V_ERRCODE))

/** @brief Extract the facility number from a status code built by STS_ENCODE(). */
static inline unsigned sts_facility(unsigned status) {return (status & STS_M_FACILITY) >> STS_V_FACILITY;}

/** @brief Extract the error code from a status code built by STS_ENCODE(). */
static inline unsigned sts_errcode(unsigned status) {return (status & STS_M_ERRCODE) >> STS_V_ERRCODE;}

/** @brief Pre-defined status codes. */
typedef enum {
  STS_K_SUCCESS = 0    /**< @brief Always means success no matter the facility. */
} sts_Predefined;

#if defined(__cplusplus)
} // extern "C"
#endif
#endif
