// -*-Mode: C;-*-
/**
@file
@brief Declare console support routines for RTEMS on Zynq.
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
2014/07/28

@par Last commit:
\$Date: 2014-07-31 17:05:57 -0700 (Thu, 31 Jul 2014) $ by \$Author: tether $.

@par Credits:
SLAC
*/
#if !defined(TOOL_IO_CONSOLESUPPORT_H)
#define      TOOL_IO_CONSOLESUPPORT_H

#if defined(__cplusplus)
extern "C" {
#endif


/** @brief Make printk() use io_putcFd1() and getchark() use io_getcFd0().

    @param[in] syslogPutc If non-null, a pointer to a function that
    io_putcStdio() will use to place characters in the syslog. 
*/
void io_setFdConsole(void (*syslogPutc)(char));


/** @brief Make printk() use io_uartPutcPolled() and getchark() use io_uartGetcPolled().

    @param[in] syslogPutc If non-null, a pointer to a function that
    io_uartPutcPolled() will use to place characters in the syslog. 
*/
void io_setPolledConsole(void (*syslogPutc)(char));


/** @brief Send a character via write(1, ...). Put it in the syslog
    as well if a function for that was given to io_setStdioConsole().
*/
void io_putcFd1(char ch);


/** @brief Get a character via read(0, ...). */
int  io_getcFd0(void);


/** @brief Send a character directly to the UART. Put it in the syslog
    as well if a function for that was given to io_setPolledConsole().
*/
void io_uartPutcPolled(char ch);


/** @brief Get a character directly from the UART, returning -1 if
    none is available at the moment.
*/
int  io_uartGetcPolled(void);


#if defined(__cplusplus)
} // extern "C"
#endif

#endif
