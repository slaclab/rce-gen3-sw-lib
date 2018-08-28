// -*-Mode: C;-*-
/**
@file
@brief Implement RTEMS fatal error processing for ARM.
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
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2014/02/16

@par Last commit:
\$Date: 2014-10-08 14:57:34 -0700 (Wed, 08 Oct 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3804 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/src/armCA9-rtems/fatal_new.c $

@par Credits:
SLAC
*/
#include <stdarg.h>

#include <rtems.h>

#include "cpu/cpu.h"

#include "debug/registers.h"

#include "debug/platform/fatal.h"

#include "debug/os/syslog.h"

#include "io/platform/consoleSupport.h"



static bool tasksEnabledFlag = false;

static int nestingLevel = 0;



bool dbg_taskingIsEnabled() {return tasksEnabledFlag;}

void dbg_setTaskingIsEnabled(bool b) {tasksEnabledFlag = b;}

int  dbg_fatalNestingLevel() {return nestingLevel;}



static void muShell(void);

// This name is reserved by the RTEMS configuration as the fatal error handler.
void bsp_fatal_extension
(
 rtems_fatal_source source,
 bool is_internal,
 rtems_fatal_code code
)
{
  const int level = __sync_add_and_fetch(&nestingLevel, 1);

  // Prevent an endless fatal-error-within-handler loop in case
  // even printk() or the micro-shell causes trouble.
  if (level > 2) {
    while (true);
  }

  // Use polled console I/O, direct to the UART, when needed.
  if (source == RTEMS_FATAL_SOURCE_EXCEPTION ||
      source == RTEMS_FATAL_SOURCE_EXIT      ||
      rtems_interrupt_is_in_progress()       ||
      !dbg_taskingIsEnabled())
  {
    io_setPolledConsole(dbg_syslogPutcNoTimestamp);
  }

  if (level > 1) {
    printk("The above error occurred inside the fatal error handler.\n");
    muShell();
  }

  /* If the error is such that we can't return from this function but
     instead run the micro-shell then we have to set up the simplified
     console and syslog support since almost no OS support will be
     available.  Otherwise we can just leave the existing console and
     syslog support in place.
  */

  if (source == RTEMS_FATAL_SOURCE_EXCEPTION) {
    dbg_printFatal(source, (int)is_internal, code, printk);
    printk("The above error was due to a hardware exception.\n");
  }
  else if (source == RTEMS_FATAL_SOURCE_EXIT) {
    /* At this point the task's stack has been completely unwound, either through
       exiting the task's top-level function or by a C++ throw which was never
       caught. Either way it's too late to delete or suspend the task here;
       attempting to do either results in a blown-stack error.
    */
    dbg_printFatal(source, (int)is_internal, code, printk);
    printk("The offending task's stack has been unwound, can't delete or suspend it.\n");
    printk("The above error may have been caused by an uncaught C++ exception.\n");
  }
  else if (dbg_taskingIsEnabled()) {
    /* rtems_fatal() called while tasking is enabled. It still might
       have been called from an ISR.
    */
    dbg_printFatal(source, (int)is_internal, code, printk);
    if (!rtems_interrupt_is_in_progress()) {
      printk("Suspending the offending task.\n");
      __sync_sub_and_fetch(&nestingLevel, 1);
      rtems_task_suspend(rtems_task_self()); // Will not return, enters a new task.
    }
  }
  else {
    dbg_printFatal(source, (int)is_internal, code, printk);
    printk("The above error occurred before multi-tasking was enabled.\n");
  }

  muShell();
  __sync_sub_and_fetch(&nestingLevel, 1); // Should never get here.
}

void dbg_printFatal
(
rtems_fatal_source source,
int is_internal,
rtems_fatal_code code,
dbg_PrintFunc print
)
{
  unsigned taskId = rtems_task_self();
  const char* ptaskName = NULL;
  char taskName[16];
  dbg_Registers gregs;
  dbg_pushGeneralRegisters();
  dbg_popGeneralRegisters(&gregs);

  ptaskName = rtems_object_get_name(taskId, sizeof taskName, taskName);
  if (!ptaskName) ptaskName = "<no task name>";

  print("\n------------------------------------------------------------------------------\n");
  if (rtems_interrupt_is_in_progress()) {
    print("A fatal error has occurred in an interrupt service routine.\n");
    if (dbg_taskingIsEnabled()) {
      print("The last task to run was task ID 0x%08x, task name '%s'.\n", taskId, ptaskName);
    }
  }
  else if (dbg_taskingIsEnabled()) {
    print("A fatal error has occurred in task ID 0x%08x, task name '%s'.\n", taskId, ptaskName);
  }
  else {
    print("A fatal error has occurred.\n");
  }
  if (is_internal) print("The source is internal to RTEMS.\n");
  print("Error source: %s\n", rtems_fatal_source_text(source));

  if (source == RTEMS_FATAL_SOURCE_EXCEPTION) {
    // We're running in the context of an exception handler. We
    // want to print the information about the context that had
    // the exception. The low-level exception handlers have
    // saved a pointer to that information.
    dbg_printExceptionFrame((rtems_exception_frame*)code, print);
  }
  else {
    // Assume that we're running in the same context that had the
    // fatal error, i.e., somebody just called rtems_fatal(), etc.
    print("Fatal-error code: %d (0x%08x)\n", code, (unsigned)code);
    print("General registers (current):\n\n");
    dbg_printGeneralRegisters(&gregs, print);
  }
  // These registers may be of interest no matter the context.
  print("\nSpecial registers (current):\n\n");
  dbg_printSpecialRegisters(print);
  print("\n");
}

static inline unsigned umin(unsigned a, unsigned b) {return (a <= b) ? a : b;}

void dbg_printExceptionFrame(const rtems_exception_frame* ctx, dbg_PrintFunc print) {
  bool const wasThumb = ctx->register_cpsr && ARM_PSR_T; // Thumb state at exception time?
  unsigned const vector = umin((unsigned)ctx->vector, 1U+(unsigned)ARM_EXCEPTION_FIQ);
  static const char* const excNames[] = {
    "Reset", "Undefined instruction", "Software interrupt",
    "Prefetch abort", "Data abort", "Reserved", "IRQ", "FIQ", "*ILLEGAL VECTOR*"
  };
  // How far ahead of the offending instruction the PC is depends
  // on the instruction set and the nature of the exception.
  static const int armPcOffsets  [] = {0, 4, 4, 4, 8, 0, 0, 0, 0};
  static const int thumbPcOffsets[] = {0, 2, 2, 2, 8, 0, 0, 0, 0};
  const uintptr_t pc =
    (uintptr_t)ctx->register_pc
    - (wasThumb ? thumbPcOffsets[vector] : armPcOffsets[vector]);
  print("Exception vector: %u (%s)\n", vector, excNames[vector]);
  print("General registers (at time of exception):\n\n");
  print("    r0: 0x%08x      r1: 0x%08x      r2: 0x%08x      r3: 0x%08x\n",
        ctx->register_r0, ctx->register_r1,
        ctx->register_r2, ctx->register_r3);
  print("    r4: 0x%08x      r5: 0x%08x      r6: 0x%08x      r7: 0x%08x\n",
        ctx->register_r4, ctx->register_r5,
        ctx->register_r6, ctx->register_r7);
  print("    r8: 0x%08x      r9: 0x%08x     r10: 0x%08x     r11: 0x%08x\n",
        ctx->register_r8,  ctx->register_r9,
        ctx->register_r10, ctx->register_r11);
  print("   r12: 0x%08x      sp: 0x%08x      lr: 0x%08x      pc: 0x%08x\n",
        ctx->register_r12, ctx->register_sp,
        (uintptr_t)ctx->register_lr, pc);
  print("  cpsr: 0x%08x\n", ctx->register_cpsr);

#if 0  // @todo Figure out spsr
      if (vector == 4)
        do_data_abort(*(uint32_t*)pc, ctx->register_spsr, ctx);
      else
        _print_full_context(ctx->register_spsr);
#endif
}


// ---------- Micro-shell code ----------

// Named characters.
static int const cr  =  13;
static int const bs  =   8;
static int const del = 127;
static int const eot =   4;
static int const sp  =  ' ';
static int const tld =  '~';

static const char* syslogStart   = NULL; // Start of syslog buffer.
static const char* syslogNext    = NULL; // Insertion point.
static const char* syslogEnd     = NULL; // One past end of buffer.
static int         syslogWrapped = 0;    // Did the insertion point wrap?


// Command function forward declarations and command table.
static void reboot(char line[], int wnext);
static void help(char line[], int wnext);
static void syslog(char line[], int wnext);
static void ldump(char line[], int wnext);
static void mdump(char line[], int wnext);
static void medit(char line[], int wnext);
static void mprobe(char line[], int wnext);
static void wdump(char line[], int wnext);

typedef void (*Command)(char line[], int wnext);

static const struct {Command cmd; const char* word;} cmdtab[] = {
  {reboot, "reboot"},
  {help,   "help"},
  {syslog, "syslog"},
  {mdump,  "mdump"},
  {medit,  "medit"},
  {mprobe, "mprobe"},
  {ldump,  "ldump"},
  {wdump,  "wdump"},
  {NULL, NULL}
};


// External refs.
#include "debug/os/syslog.h"

#include "memory/mem.h"

#include "time/cpu/clockRate.h"

#include "time/platform/time.h"

void dbg_freezeSyslog(char const **start, char const **next, char const **end, int* wrapped);


// Forward decl. of local auxiliaries.
static void delayOneSecond (void);
static int  eq             (const char*, const char*);
static void getline        (char line[], int lineEnd);
static int  getUnsigned    (const char* start, unsigned *value);
static void mdumpImpl      (char line[], int wnext, int defaultSize, const char* cmd);
static void nextWord       (char line[], int* wstart, int* wnext);
static void printMemAscii  (const char* mem, unsigned bytes);
static void printMemHex    (const char* mem, unsigned bytes, unsigned size);
static int  readMemory     (char *outbuf, unsigned *bytesGotten, unsigned address, unsigned bytesRequested);

// The number of bytes to display per line in memory dumps.
#define LINE_BYTES 16


// Micro-shell main loop.
static void muShell(void) {
  /* Can't rely on RTEMS, the C library or other standard libraries so
     install the simple polled console support. Suppress further
     capture to the syslog by supplying a NULL syslogPutc function
     pointer. */
  io_setPolledConsole(NULL);

  /* get the current state of the syslog so that we may print it when requested. */
  dbg_getSyslogState(&syslogStart, &syslogNext, &syslogEnd, &syslogWrapped);

  printk("\nFatal-error handler micro-shell.\n");
  printk("End commands with carriage returns. Backspace and delete keys work.\n");
  printk("Use the 'help' command to see what's available.\n");
  char line[101];
  int const lineEnd = sizeof line - 1;

  while (1) {
    printk("\n> ");
    getline(line, lineEnd);
    int p1 = 0, p2 = 0;
    nextWord(line, &p1, &p2);
    if (line[p1]) { // Non-empty line.
      int i;
      for (i = 0; cmdtab[i].cmd; ++i) {
        if (eq(line + p1, cmdtab[i].word)) {
          cmdtab[i].cmd(line, p2);
          break;
        }
      }
      if (!cmdtab[i].cmd) printk("Try the 'help' command.\n");
    }
  }
}

// Command functions.

static void help(char line[], int wnext) {
  printk("\n");

  printk("reboot - Reboot the system.\n\n");

  printk("help   - Print this help text.\n\n");

  printk("syslog - Dump the syslog buffer.\n\n");

  printk("mdump  - Dump memory in hex and ASCII.\n");
  printk("         mdump <address> [<bytes> [size]]\n");
  printk("         <bytes> is the number of bytes to display.\n");
  printk("         <size> may be 1, 2 or 4.\n");
  printk("         Numbers may be: decimal, or hex with leading '0x'.\n");
  printk("         The size parameter affects only the display; memory\n");
  printk("         accesses are always byte-wide.\n\n");

  printk("ldump  - ldump <address> <bytes> is the same as mdump <address> <bytes> 4.\n\n");

  printk("wdump  - wdump <address> <bytes> is the same as mdump <address> <bytes> 2.\n\n");

  printk("medit  - Alter contents of memory.\n");
  printk("         medit <address> <byte1> <byte2> ... <byteN>\n");
  printk("         Numbers are treated in the same way as for mdump.\n\n");

  printk("mprobe - Print the type and access permissions for a memory address.\n");
  printk("         mprobe <address>\n\n");
}

static void reboot(char line[], int wnext) {
  printk("Rebooting (hard reset) ...\n");
  delayOneSecond(); // Let our final message get out.
  TOOL_CPU_reset(2); // 2 == Hard reset.
}

static void syslog(char line[], int wnext) {
  printk("\n");
  const char* ch;
  //Print oldest content first: from the insertion point to the end,
  // unless the insertion point has never wrapped. Then from the
  // beginning to the insertion point.
  if (syslogWrapped) {
    for (ch = syslogNext; ch < syslogEnd; ++ch) printk("%c", *ch);
  }
  for (ch = syslogStart; ch < syslogNext; ++ch) printk("%c", *ch);
  printk("---------- End of syslog dump ----------\n");
}


static void mdump(char line[], int wnext) {
  mdumpImpl(line, wnext, 1, "mdump");
}

static void ldump(char line[], int wnext) {
  mdumpImpl(line, wnext, 4, "ldump");
}

static void wdump(char line[], int wnext) {
  mdumpImpl(line, wnext, 2, "wdump");
}

static void mdumpImpl(char line[], int wnext, int defaultSize, const char* cmd) {
  int waddress = 0;
  int wbytes = 0;
  int wsize = 0;
  waddress = wnext;
  nextWord(line, &waddress, &wnext);
  wbytes = wnext;
  nextWord(line, &wbytes, &wnext);
  if (defaultSize == 1) {// mdump only.
    wsize = wnext;
    nextWord(line, &wsize, &wnext);
  }

  unsigned address = 0;
  unsigned bytes = 320;
  unsigned size  =   defaultSize;
  if (!line[waddress]) {
    printk("You have to provide an address after '%s '.\n", cmd);
    return;
  }
  int ok = 1;
  ok = getUnsigned(line + waddress, &address);
  if (!ok) return;
  if (line[wbytes]) ok = getUnsigned(line + wbytes, &bytes);
  if (!ok) return;
  if (defaultSize == 1) { // mdump only.
    if (line[wsize]) ok = getUnsigned(line + wsize, &size);
    if (!ok) return;
    if (size != 1 && size != 2 && size != 4) {
      printk("Size parameter %u is not 1, 2 or 4.\n", size);
      return;
    }
  }

  __attribute__((aligned(4))) char mem[LINE_BYTES];
  while (bytes > 0) {
    unsigned n;
    ok = readMemory(mem, &n, address, (bytes > LINE_BYTES) ? LINE_BYTES: bytes);
    if (!ok) break;
    printk("0x%08X ", address);
    printMemHex(mem, n, size);
    printMemAscii(mem, n);
    printk("\n");
    address += n;
    bytes -= n;
  }
  if (!ok) printk("Can't read from address 0x%08x.\n", address);
}

static void medit(char line[], int wnext) {
  int waddress = wnext;
  nextWord(line, &waddress, &wnext);
  if (!line[waddress]) {
    printk("You have to provide an address after 'medit '.\n");
    return;
  }
  unsigned address;
  if (!getUnsigned(line + waddress, &address)) {
    return;
  }
  int wbyte = wnext;
  for (nextWord(line, &wbyte, &wnext);
       line[wbyte];
       ++address, wbyte = wnext, nextWord(line, &wbyte, &wnext))
    {
      unsigned byte;
      if (getUnsigned(line + wbyte, &byte)) {
        if (!(mem_getFlagsWrite(address) & MEM_WRITE)) {
          printk("Can't write to address 0x%08x.\n", address);
          return;
        }
        *(char*)address = byte;
      }
      else {
        return;
      }
    }
}

static void mprobe(char line[], int wnext) {
  int waddress = wnext;
  nextWord(line, &waddress, &wnext);
  if (!line[waddress]) {
    printk("You have to provide an address after 'mprobe '.\n");
    return;
  }
  unsigned address;
  if (!getUnsigned(line + waddress, &address)) {
    return;
  }

  int flags = mem_getFlagsWrite(address);
  if (!(flags & MEM_WRITE)) flags = mem_getFlagsRead(address);
  printk("Address 0x%08x is ", address);
  if (!(flags & MEM_READ)) {
    printk("inaccessible.\n");
    return;
  }
  if (flags & MEM_CACHED) printk("cached ");     else printk("uncached ");
  if (flags & MEM_WRITE)  printk("read-write "); else printk("read-only ");
  if (flags & MEM_SHARED) printk("shared ");     else printk("unshared ");
  if (flags & MEM_HANDSHAKE) {
    printk("strongly-ordered ");
  }
  else if (flags & MEM_DEVICE) {
    printk("device ");
  }
  else {
    printk("normal ");
  }
  printk("memory.\n");
}


// Auxiliaries.

static void delayOneSecond(void) {
  unsigned long long const rate = time_getZynqClockRate(ZYNQ_CLOCK_CPU_3X2X);
  unsigned long long const now  = TOOL_TIME_lticks();
  unsigned long long const deadline = now + rate;
  while(TOOL_TIME_lticks() < deadline);
}

int eq(const char* s1, const char* s2) {
  char ch;
  while ( (ch = *s1++) ) {
    if (ch != *s2++) return 0;
  }
  return !*s2;
}

static void getline(char line[], int lineEnd) {
  int ch;
  int next = 0;
  while ((ch = getchark()) != cr) {
    if (  ((ch == bs) || (ch == del)) && (next > 0))   {
      // Backspace one char if the line is nonempty.
      --next;
      printk("%c", bs);
      printk("%c", sp);
      printk("%c", bs);
    }
    else if ( (ch < sp) || (ch > tld)) {
      // Ignore other non-printing characters.
    }
    else if (next >= lineEnd) {
      putk("\nLine too long.\n");
      next = 0;
      break;
    }
    else {
      // Echo and store a printable character.
      printk("%c", ch);
      line[next++] = (char)ch;
    }
  }
  line[next] = '\0';
  printk("\n");
}

static int getUnsigned(const char* word, unsigned *value) {
  *value = 0;
  const char* s = word;
  if (!s) return 0;
  if (!*s) return 0;
  // Check for 0x or 0X.
  if (*s++ == '0' && (*s == 'x' || *s == 'X')) {
    // Accumulate hex.
    while (*++s) {
      if      (*s >= '0' && *s <= '9') {
        *value = (*value << 4) + (*s - '0');
      }
      else if (*s >= 'a' && *s <= 'f') {
        *value = (*value << 4) + (*s - 'a' + 10);
      }
      else if (*s >= 'A' && *s <= 'F') {
        *value = (*value << 4) + (*s - 'A' + 10);
      }
      else {
        printk("Not a number: %s\n", word);
        return 0;
      }
    }
    return 1;
  }
  else {
    // Accumulate decimal.
    s = word - 1;
    while (*++s) {
      if (*s >= '0' && *s <= '9') {
        *value = (*value * 10) + (*s - '0');
      }
      else {
        printk("Not a number: %s\n", word);
        return 0;
      }
    }
    return 1;
  }
}

/* Skip spaces starting at *wstart, incrementing *wstart. If there is
   no next word then line[*wstart] and line [*wnext] will be null upon
   return. If a word is found then *wstart will be the index of its
   first character and *wnext will be 1 + the index of the character
   that ended the word. That word-ending character will be replaced by
   a null.
*/
void nextWord(char line[], int *wstart, int *wnext) {
  while (line[*wstart] == ' ') ++*wstart;
  *wnext = *wstart;
  while ((line[*wnext] != ' ') && (line[*wnext] != '\0')) ++*wnext;
  if (line[*wnext]) {
    // There is more input after the word we just found.
    line[*wnext] = '\0';
    ++*wnext;
  }
}

static void printMemAscii(const char* mem, unsigned bytes) {
  int i;
  for (i = 0; i < bytes; ++i) {
    if (mem[i] >= ' ' && mem[i] <= '~') printk("%c", mem[i]);
    else printk(".");
  }
}

static void printMemHex(const char* mem, unsigned bytes, unsigned size) {
  unsigned n = bytes / size;
  int i;
  if (size == 1) {
    for (i = 0; i < n; ++i) printk("%02X ", mem[i]);
    for (; i < LINE_BYTES; ++i) printk("   ");
  }
  else if (size == 2) {
    const uint16_t* p = (const uint16_t*)mem;
    for (i = 0; i < n; ++i) printk("%04X ", p[i]);
    for (; i < (LINE_BYTES/2); ++i) printk("     ");
  }
  else {
    const uint32_t* p = (const uint32_t*)mem;
    for (i = 0; i < n; ++i) printk("%08X ", p[i]);
    for (; i < (LINE_BYTES/4); ++i) printk("         ");
  }
}

static int readMemory(char *outbuf, unsigned *bytesGotten, unsigned address, unsigned bytesRequested) {
  unsigned i;
  for (i = 0; i < bytesRequested; ++i) {
    int flags = mem_getFlagsRead(address);
    if (MEM_READ & flags) outbuf[i] = *(char*)(address + i);
    else break;
  }
  *bytesGotten = i;
  return *bytesGotten == bytesRequested;
}
