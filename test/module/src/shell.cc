// -*-Mode: C++;-*-
/*!
*
* @brief   A module to launch the shell
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    April 12, 2012 -- Created
*
* $Revision: 1929 $
*
* @verbatim
*                               Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include "datCode.hh"
#if !tgt_board_rce440
#include DAT_PUBLIC(service, shell, ShellCommon.hh)
#endif

// GDB stub is compiled with C bindings
extern "C" int rtems_gdb_start(int pri, char *ttyName);


extern "C" void rce_appmain(uintptr_t)
{
#if !tgt_board_rce440
   // Allow users to log in via telnet and execute interactive commands.
  service::shell::initialize();

  // Start a debugger daemon
  // First  arg = 0 --> Use socket IO over TCP.
  // Second arg = 0 --> Priority is chosen by the debugger daemon
  rtems_gdb_start(0, 0);
#endif
}
