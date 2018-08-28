// -*-Mode: C;-*-
/**
@file
@brief Declare the syslog C API for RTEMS.
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
\$Date: 2014-07-31 11:09:56 -0700 (Thu, 31 Jul 2014) $ by \$Author: tether $.

@par Credits:
SLAC
*/
#if !defined(TOOL_DEBUG_SYSLOG_H)
#define      TOOL_DEBUG_SYSLOG_H

#if defined(__cplusplus)
extern "C" {
#endif

/** @brief Put a character into the syslog, adding a timestamp at the beginning of each line. */
void dbg_syslogPutcWithTimestamp(char);

/** @brief Put a character into the syslog but don't even try to add timestamps to lines. */
void dbg_syslogPutcNoTimestamp(char);

/** @brief An iterator over the contents of the syslog buffer that
    finds the content as arbitrarily-sized chunks of
    storage. Assumes that the syslog buffer is not changed while
    iteration is in progress.
*/
typedef struct dbg_SyslogChunkerStruct dbg_SyslogChunker;

/** @brief Set up the syslog buffer, if any.

In some environments we keep an in-memory log of console
output for debugging.
*/
void dbg_initSyslog(void);

/** @brief Make the syslog empty. */
void dbg_clearSyslog(void);

/** @brief Allocate and initialize an object used to retrieve syslog content. */
dbg_SyslogChunker* dbg_SyslogChunker_construct(void);

/** @brief Finalize and deallocate a dbg_SyslogChunker. */
void dbg_SyslogChunker_destroy(dbg_SyslogChunker*);

/** @brief Find the next chunk: update chunk address and size. */
void dbg_SyslogChunker_advance(dbg_SyslogChunker*);

/** @brief Get the address of the current chunk. */
const char* dbg_SyslogChunker_chunkAddress(const dbg_SyslogChunker*);

/** @brief Get the size of the current chunk. */
int dbg_SyslogChunker_chunkSize(const dbg_SyslogChunker*);

#if defined(__cplusplus)
} /* extern "C" */
#endif
#endif
