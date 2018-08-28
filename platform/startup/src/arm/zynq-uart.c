// -*-Mode: C;-*-
/**
@file
@brief Interrupt-driven termios I/O and bare-metal polled I/O for the Zynq UART.
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
2014/07/21

@par Last commit:
\$Date: 2015-07-14 10:55:53 -0700 (Tue, 14 Jul 2015) $ by \$Author: smaldona $.

@par Credits:
SLAC
*/
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

/* The global arrays Console_Port_Data[] and Console_Port_Tbl[], both
   indexed by minor device number.
*/
#include <bsp/zynq-uart.h>

/* Zynq UART register definitions. */
#include <bsp/zynq-uart-regs.h>

/* Interrupt vector numbers and handler management. */
#include <bsp/irq.h>

/* For libchip_serial_default_probe(), which always returns true. */
#include <libchip/sersupp.h>

/* For time_getZynqClockRate().*/
#include "time/cpu/clockRate.h"

#include "io/platform/consoleSupport.h"
#include "debug/os/syslog.h"

/* ========== Parameters, forward declarations  ========== */

/* This number of chars in the Rx FIFO that will trigger an interrupt. */
static unsigned const rxTriggerLevel = 30;

/* The upper eight bits of a 10-bit timeout counter which when active
   counts down by one each time the Rx line is idle for the time it
   takes to signal one bit. The timeout is used to cause an interrupt
   when the Rx data line has been idle for a while as insurance
   for the case when the Rx FIFO is taking a long time to fill
   up to the trigger level.

   After you write a one to the RSTO bit in the control register the
   UART loads the counter from the timeout-value register but doesn't
   start the counter. The end of first character received starts the
   countdown. Each character received thereafter reloads and restarts
   the counter. If the count ever reaches zero then the UART disables
   the counter and generates an interrupt. */
static unsigned const rxTimeout = (300 >> 2);


/* Get a pointer to the console_data struct for the device. This
   structure contains a pointer to termios' internal structure
   describing the devices's console properties as well as a pointer to
   the driver's provate data for the device. There is also a bActive
   flag which is set true rhwn the Tx FIFO is nonempty. */
static inline console_data* getConsoleData(int minor);


/* Get the number of characters put in the Tx FIFO by the last call to
   writeUart(). Driver-private data. */
static inline int getNumCharsSent(const console_data*);


/* Set the sent-chars count for the device. */
static inline void setNumCharsSent(console_data*, int nsent);


/* Get a pointer to termios' console-properties struct for the
   device. */
static inline struct rtems_termios_tty* getTtyDescription(const console_data*);


/* Get a pointer to the UART's register set. */
static inline volatile zynq_uart* getUartRegs(int minor);


/* Calculate a baud rate from the contents of the two clock-divisor
   registers in the UART. uartClock is the ref clock rate, divided by
   8 if the CLKS bit is set in the mode register.*/
static inline unsigned calculateBaudRate
(uint32_t uartClock,
 uint32_t sampleRateDivisor,
 uint32_t bitRateDivisor);


/* The inverse of calculateBaudRate. Returns 0 if the rate is
   supported, else -1. */
static inline int calculateBaudDivisors
(uint32_t  bitRate,
 uint32_t* sampleRateDivisor,
 uint32_t* bitRateDivisor,
 bool      divUartClkBy8);




#define UART_FIFO_SIZE 64

#define NO_OF_UARTS 2

#define BIT_RATE_MAX_PERCENT_ERROR 3

#define IRQ_NONE 0x0000U

#define IRQ_ALL  0xffffU


/* ========== UART functions for interrupt-driven I/O ========== */

static void handleUartInterrupt(void*);


/* Driver private data*/
static unsigned numCharsSent[NO_OF_UARTS]; 




/* Perform interrupt-related UART initialization without interfering
   with the serial port settings installed by the boot loader and
   without actually enabling UART interrupts. Called by RTEMS'
   console_initialize() function. */
static void initializeUart(int minor) {
  volatile zynq_uart* const uart        = getUartRegs(minor);
  console_data*       const consoleData = getConsoleData(minor);
  /* Set up some working storage private to the device. */
  consoleData->pDeviceContext = (void*)&numCharsSent[minor];
  setNumCharsSent(consoleData, 0);
  // Disable all interrupts at the UART, clear all pending interrupt conditions.
  uart->irq_en   = IRQ_NONE;
  uart->irq_dis  = IRQ_ALL;
  uart->irq_sts  = IRQ_ALL;
  // Install the interrupt handler only for the console UART.
  if (minor == Console_Port_Minor) {
    rtems_interrupt_handler_install
      (Console_Port_Tbl[minor]->ulIntVector, // IRQ "vector" (GIC ID).
       "ZynqUart",
       RTEMS_INTERRUPT_UNIQUE, // Unshared GIC interrupt.
       handleUartInterrupt,
       (void*)minor);          // Argument to handler.
    // Set the Rx trigger level.
    uart->rx_fifo_trg_lvl = rxTriggerLevel;
    // Set the Rx timeout.
    uart->rx_timeout = rxTimeout;
  }
}




/* Enable UART interrupts. Called by rtems_termio_open() when no other
   open for the device is extant. arg points to an
   rtems_libio_open_close_args_t which we don't need.

   It's possible that there are characters in the Tx FIFO but if so
   they were not drawn from the device's raw output queue, so a
   subsequent TEMPTY interrupt with numCharsSent = 0 will do no harm.

   I assume that if the Rx FIFO contains more than the Rx trigger
   level then the Rx trigger interrupt will go off as soon as it's
   enabled.*/
static int firstUartOpen(int major, int minor, void* ptr) {
  volatile zynq_uart*            const uart        = getUartRegs(minor);
  console_data*                  const consoleData = getConsoleData(minor);
  rtems_libio_open_close_args_t* const args        = (rtems_libio_open_close_args_t *)ptr;
  struct rtems_termios_tty*      const tty         = (struct rtems_termios_tty *)args->iop->data1;

  // Save the pointer to the termios data for this device.
  consoleData->termios_data = tty;

  // Haven't taken any chars from the device's raw output buffer yet.
  setNumCharsSent(consoleData, 0);

  /* Give RTEMS the current baud rate setting. RTEMS requires that the
     rate we give it be an exact match for one of the official termios
     baud rates so that it can calculate the corresponding mask for
     the c_cflag field in the termios struct. The rate actually set in
     the UART may be a few percent off from one of the official values
     so we find the closest match in RTEMS' baud rate table. */
  uint32_t const uartClk =
    time_getZynqClockRate(ZYNQ_CLOCK_UART_REF) >> ((uart->mode & ZYNQ_UART_MODE_CLKS) ? 3 : 0);
  unsigned const bitRate = calculateBaudRate(uartClk, uart->baud_rate_gen, uart->baud_rate_div);
  int i;
  uint32_t errmin = 0xffffffffU;
  rtems_termios_baud_t bestMatch = bitRate;
  for (i = 0; rtems_termios_baud_table[i].name; ++i) { // Sentinel entry has null name.
    uint32_t const tabRate = rtems_termios_baud_table[i].local_value;
    uint32_t const e       = (bitRate > tabRate) ? bitRate - tabRate : tabRate - bitRate;
    if (e < errmin) {
      errmin = e;
      bestMatch = tabRate;
    }
  }
  rtems_termios_set_initial_baud(tty, bestMatch);

  // From now on I/O for this device will be interrupt-driven if it's the console.
  if (minor == Console_Port_Minor) {
    io_setFdConsole(dbg_syslogPutcWithTimestamp);
    uart->irq_sts  = IRQ_ALL;
    uart->irq_en   = ZYNQ_UART_TIMEOUT | ZYNQ_UART_RTRIG | ZYNQ_UART_TEMPTY;
    uart->irq_dis  = IRQ_NONE;
  }

  return 0; // Value not used by termios.
}




/* Disable UART interrupts. Called by rtems_termio_close() when no
   other open for the device is extant.

   Characters in the Tx FIFO will drain away by themselves, but we
   have to leave those in the Rx FIFO alone so that polled-mode
   routines can read them. */
static int lastUartClose(int major, int minor, void* arg) {
  if (minor == Console_Port_Minor) {
    io_setPolledConsole(dbg_syslogPutcWithTimestamp);
  }
  else {
    volatile zynq_uart* const uart = getUartRegs(minor);
    uart->irq_en   = IRQ_NONE; // Enable no interrupts.
    uart->irq_dis  = IRQ_ALL;  // Disable all interrupts.
    uart->irq_sts  = IRQ_ALL;  // Clear all pending interrupt requests.
  }
  return 0; // Value not used by termios.
}




static void writeBufSyslog(const char *buf, size_t len) {
  int i;
  for (i = 0; i < len; ++i) {
    dbg_syslogPutcNoTimestamp(buf[i]);
  }
}

static void writeStrSyslog(const char *str) {
  int i;
  for (i = 0; str[i]; ++i) {
    dbg_syslogPutcNoTimestamp(str[i]);
  }
}

/* Transfer as many characters as possible from the given buffer to
   the Tx FIFO.  Record the number of characters transferred for later
   use by the interrupt handler when the TEMPTY interrupt goes off.

   This function runs in the context of the interrupt handler which
   calls it indirectly via rtems_termios_dequeue_characters(). */
static ssize_t writeUart(int minor, const char* buf, size_t len) {
  volatile zynq_uart* const uart        = getUartRegs(minor);
  console_data*       const consoleData = getConsoleData(minor);
  int n = 0;
  while (n < len && !(uart->channel_sts & ZYNQ_UART_CHANNEL_STS_TNFUL)) {
    uart->tx_rx_fifo = (uint32_t)buf[n++];
  }
  setNumCharsSent(consoleData, n);
  consoleData->bActive = true;
  return 0; // Value not used by termios.
}




/* Set serial port attributes and termios flags. Setting the port
   attributes is a big deal and requires a soft-reset of the Tx and Rx
   sides of the UART.  It shouldn't be done unless you know that the
   line is idle.

   The termios struct is the public part of the device's
   rtems_termios_tty struct, the part you give to the functions
   implementing the termios API, e.g., tcsetattr(). */
static int setUartAttributes(int minor, const struct termios *term) {
  volatile zynq_uart* const uart        = getUartRegs(minor);

  /* Set the requested baud rate if it's one we support. I've had
     trouble getting other rates to work.*/
  unsigned const rateFlag = term->c_cflag & CBAUD;
  unsigned rate;
  switch (rateFlag) {
  case B115200: rate = 115200; break;
  case B57600:  rate =  57600; break;
  case B38400:  rate =  38400; break;
  case B19200:  rate =  19200; break;
  default: return -1;
  }
  uint32_t const clksel = uart->mode & ZYNQ_UART_MODE_CLKS;
  uint32_t sampleRateDivisor;  // Divides UART sel_clk to get signal-sampling rate.
  uint32_t bitRateDivisor;     // Divides signal-sampling rate to get signal bits per sec.
  if ( 0 != calculateBaudDivisors(rate, &sampleRateDivisor, &bitRateDivisor, !!clksel)) {
    return -1;
  }

  /* Character size. */
  unsigned const sizeFlag = term->c_cflag & CSIZE;
  uint32_t size;
  switch (sizeFlag) {
  case CS7: size = ZYNQ_UART_MODE_CHRL_7; break;
  case CS8: size = ZYNQ_UART_MODE_CHRL_8; break;
  default: return -1;
  }
  size = ZYNQ_UART_MODE_CHRL(size); // Shift into position.

  /* Parity. */
  unsigned const parityFlag = term->c_cflag & (PARENB | PARODD);
  uint32_t parity;
  if (parityFlag & PARENB) {
    parity = (parityFlag & PARODD) ? ZYNQ_UART_MODE_PAR_ODD : ZYNQ_UART_MODE_PAR_EVEN;
  }
  else {
    parity = ZYNQ_UART_MODE_PAR_NONE;
  }
  parity = ZYNQ_UART_MODE_PAR(parity);

  /* Number of stop bits. */
  uint32_t stops = (term->c_cflag & CSTOPB) ? ZYNQ_UART_MODE_NBSTOP_STOP_2 : ZYNQ_UART_MODE_NBSTOP_STOP_1;
  stops = ZYNQ_UART_MODE_NBSTOP(stops);

  uint32_t const operatingMode = ZYNQ_UART_MODE_CHMODE(ZYNQ_UART_MODE_CHMODE_NORMAL);

  /* If the console is in use we need to take some care in order to
     prevent garbled output at least (I don't know what we can do to
     prevent garbled input). First we disable interrupts from the
     UART. We assume that the Rx trigger level we set leaves an
     adequate margin for the short time we'll need. We let the Tx FIFO
     drain, add a couple of NULs we don't mind losing then wait for
     the Tx FIFO to drain again.
  */
  uart->irq_en  = IRQ_NONE;
  uart->irq_dis = IRQ_ALL;
  while (!(uart->channel_sts & ZYNQ_UART_CHANNEL_STS_TEMPTY));
  uart->tx_rx_fifo = 0;
  uart->tx_rx_fifo = 0;
  while (!(uart->channel_sts & ZYNQ_UART_CHANNEL_STS_TEMPTY));
  /* Disable and perform a soft reset of reception and transmission.
     The disabling suppresses UART interrupts. The reset clears the
     rx_timeout register but we have to do it in order to change the
     baud rate. */
  uart->control = ZYNQ_UART_CONTROL_RXDIS
    | ZYNQ_UART_CONTROL_TXDIS
    | ZYNQ_UART_CONTROL_RXRES
    | ZYNQ_UART_CONTROL_TXRES;
  
  /* Now install the operating parameters determined above and restore
     the Rx timeout value. Re-enable interrupts. */
  uart->mode            = operatingMode | parity | size | stops | clksel;
  uart->baud_rate_gen   = sampleRateDivisor;
  uart->baud_rate_div   = bitRateDivisor;
  uart->rx_timeout      = rxTimeout;
  uart->irq_en          = IRQ_ALL;
  uart->irq_dis         = IRQ_NONE;
  /* Restart the UART and the Rx timeout counter. */
  uart->control = ZYNQ_UART_CONTROL_RXEN
    | ZYNQ_UART_CONTROL_TXEN
    | ZYNQ_UART_CONTROL_RSTTO;

  return 0;
}




static void handleUartInterrupt(void* arg) {
  int                 const minor       = (int)arg;
  volatile zynq_uart* const uart        = getUartRegs(minor);
  console_data*       const consoleData = getConsoleData(minor);
 
  // Get the interrupt status bits.
  uint32_t interruptStatus = uart->irq_sts;
  // Deal with the cause of the interrupt.
  if (interruptStatus & (ZYNQ_UART_TIMEOUT | ZYNQ_UART_RTRIG)) {
    // Rx FIFO trigger or Rx timeout.
    char buf[UART_FIFO_SIZE];
    int n = 0;
    while(!(uart->channel_sts & ZYNQ_UART_CHANNEL_STS_REMPTY)) {
      buf[n++] = (char)uart->tx_rx_fifo;
    }
    rtems_termios_enqueue_raw_characters(getTtyDescription(consoleData), buf, n);
  }
  if (interruptStatus & ZYNQ_UART_TEMPTY) {
    // Tx FIFO empty.
    int const nsent = getNumCharsSent(consoleData);
    setNumCharsSent(consoleData, 0); // The Tx FIFO is empty.
    consoleData->bActive = false;
    // The following will eventually invoke writeUart() in order to re-fill the Tx FIFO.
    rtems_termios_dequeue_characters(getTtyDescription(consoleData), nsent);
  }
  // Reset all pending interrupt conditions.
  uart->irq_sts = IRQ_ALL;
}




/* Define the collection of driver function pointers referred to by
   name by Console_Configuration_Ports[]. */
const console_fns zynq_uart_fns = {
  .deviceProbe                = libchip_serial_default_probe, // Always returns true.
  .deviceFirstOpen            = firstUartOpen,
  .deviceLastClose            = lastUartClose,
  .deviceRead                 = NULL,                         // POLLED read, not implemented.
  .deviceWrite                = writeUart,
  .deviceInitialize           = initializeUart,
  .deviceWritePolled          = NULL,
  .deviceSetAttributes        = setUartAttributes,
  .deviceOutputUsesInterrupts = true                          // This applies to both input and output.
};




/* ========== Driver helper functions ========== */

static inline console_data* getConsoleData(int minor) {
  return &Console_Port_Data[minor];
}



static inline int getNumCharsSent(const console_data* consoleData) {
  return *(int*)consoleData->pDeviceContext;
}



static inline void setNumCharsSent(console_data* consoleData, int nsent) {
  *(int*)consoleData->pDeviceContext = nsent;
}



static inline struct rtems_termios_tty* getTtyDescription(const console_data* consoleData) {
  return (struct rtems_termios_tty*)consoleData->termios_data;
}



static inline volatile zynq_uart* getUartRegs(int minor) {
  return (volatile zynq_uart*)Console_Port_Tbl[minor]->ulCtrlPort1;
}




static inline unsigned calculateBaudRate
(uint32_t uartClock,
 uint32_t sampleRateDivisor,
 uint32_t bitRateDivisor)
{
  return uartClock / (sampleRateDivisor * (bitRateDivisor + 1U));
}




static inline int calculateBaudDivisors
(uint32_t  bitRate,
 uint32_t* sampleRateDivisor,
 uint32_t* bitRateDivisor,
 bool      divUartClkBy8)
{
  uint32_t const uartClk = time_getZynqClockRate(ZYNQ_CLOCK_UART_REF) >> (divUartClkBy8 ? 3 : 0);
  // The max baud rate is half the input clock rate.
  if ((bitRate >> 1) > uartClk) return -1;

  // Try all possible bit-rate divisors to find the closest match to the desired bit rate.
  uint32_t minError = 0xFFFFFFFFU;
  uint32_t brdiv;
  for (brdiv = 4; brdiv < 255; brdiv++) {
    // Knowing the bit-rate divisor, the desired bit rate and the UART clock rate fixes the sample rate.
    uint32_t const srdiv       = uartClk / (bitRate * (brdiv + 1U));
    uint32_t const estBitRate  = calculateBaudRate(uartClk, srdiv, brdiv);
    uint32_t const rateError   = (bitRate > estBitRate) ? bitRate - estBitRate : estBitRate - bitRate;
    if (minError > rateError) {
      // Closest match so far.
      *sampleRateDivisor = srdiv;
      *bitRateDivisor    = brdiv;
      minError           = rateError;
    }
  }
  // Is the closest match close enough?
  return ( ((minError * 100) / bitRate) > BIT_RATE_MAX_PERCENT_ERROR ) ? -1 : 0;
}
