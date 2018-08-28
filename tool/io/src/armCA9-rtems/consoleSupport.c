// -*-Mode: C;-*-
/**
@file
@brief Implement console support routines for RTEMS on Zynq.
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
\$Date: 2014-09-02 17:30:47 -0700 (Tue, 02 Sep 2014) $ by \$Author: tether $.

@par Credits:
SLAC
*/

#include <rtems.h>

#include <unistd.h>


/* The global arrays Console_Port_Data[] and Console_Port_Tbl[], both
   indexed by minor device number.
*/
#include <bsp/zynq-uart.h>

/* Zynq UART register definitions. */
#include <bsp/zynq-uart-regs.h>


#include "debug/os/syslog.h"
#include "io/platform/consoleSupport.h"


/* If this pointer is non-null use it to put a character into the syslog. */
static void (*syslogPutc)(char ch) = NULL;

/* printk() calls rtems_putc() calls *BSP_output_char().*/
extern BSP_output_char_function_type BSP_output_char;

/* getchark() calls *BSP_poll_char(). */
extern BSP_polling_getchar_function_type BSP_poll_char;


/* Base addresses for UART0 and UART1. getUartRegs() may be called
   before the Console_Port_Data[] is initialized so we use
   Console_Configuration_Port[] instead, it's statically initialized
   in console-config.c in the Zynq BSP.
*/

static inline volatile zynq_uart* getUartRegs(int minor) {
  return (volatile zynq_uart*)Console_Configuration_Ports[minor].ulCtrlPort1;
}



#define IRQ_NONE 0x0000U

#define IRQ_ALL  0xffffU


/* Call this when you know that the interrupt handler, with its RTEMS
   termios calls, is being used.*/
void io_setFdConsole(void (*sysPutc)(char)) {
  syslogPutc      = sysPutc;
  BSP_output_char = io_putcFd1;
  BSP_poll_char   = io_getcFd0;
}


/* Call this when you know that the interrupt handler, with its RTEMS
   termios calls, won't work.*/
void io_setPolledConsole(void (*sysPutc)(char)) {
  syslogPutc      = sysPutc;
  BSP_output_char = io_uartPutcPolled;
  BSP_poll_char   = io_uartGetcPolled;
  bsp_console_select();    // Make sure that Console_Port_Minor is set.
  volatile zynq_uart* const uart = getUartRegs(Console_Port_Minor);
  uart->irq_en   = IRQ_NONE; // Enable no interrupts.
  uart->irq_dis  = IRQ_ALL;  // Disable all interrupts.
  uart->irq_sts  = IRQ_ALL;  // Clear all pending interrupt requests.
}


/* For printk() output when /dev/console is open and usable, except possibly
   in interrupt service routines.
*/
void io_putcFd1(char ch) {
  if (rtems_interrupt_is_in_progress()) {
    // Time-stamping may not work in an ISR.
    void (*const sputc)(char) = syslogPutc;
    if (sputc) syslogPutc = dbg_syslogPutcNoTimestamp;
    io_uartPutcPolled(ch);
    syslogPutc = sputc;
  }
  else {
    if (syslogPutc) syslogPutc(ch);
    write(1, &ch, 1);
  }
}


/* For getchark() input when /dev/console is open and usable, except
   possibly in interrupt service routines. 
*/
int io_getcFd0(void) {
  if (rtems_interrupt_is_in_progress()) {
    return io_uartGetcPolled();
  }
  else {
    unsigned char ch;
    ssize_t const count = read(0, &ch, 1);
    return (count > 0) ? (int)ch : -1;
  }
}


/* For printk() output when /dev/console is closed or unusable. */
void io_uartPutcPolled(char ch) {
  volatile zynq_uart* const uart = getUartRegs(Console_Port_Minor);
  if (syslogPutc) syslogPutc(ch);
  if (ch == '\x0a'/*LF*/) {
    // Send a carriage return after each linefeed.
    while (uart->channel_sts & ZYNQ_UART_CHANNEL_STS_TFUL);
    uart->tx_rx_fifo = (uint32_t)'\x0d'/*CR*/;
  }
  while (uart->channel_sts & ZYNQ_UART_CHANNEL_STS_TFUL);
  uart->tx_rx_fifo = (uint32_t)ch;
}


/* For getchark() input when /dev/console is closed or unusable. */
int io_uartGetcPolled(void) {
  volatile zynq_uart* const uart = getUartRegs(Console_Port_Minor);
  return (uart->channel_sts & ZYNQ_UART_CHANNEL_STS_REMPTY) ? -1 : (uint32_t)uart->tx_rx_fifo;
}
