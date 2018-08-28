/*!
* @file      fpgaLoad.cc
*
* @brief     Module for loading a bitstream file onto an FPGA.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      October 21, 2013 -- Created
*
* $Revision: 3125 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/statvfs.h>
#include <rtems/libio.h>
#include <errno.h>

#include <rtems.h>

#include "fpga/fpga.h"
#include "debug/print.hh"
#include "time/Time.hh"
#include "xilinxcfg/xparameters.h"


#define DCFG_DEVICE_ID  XPAR_XDCFG_0_DEVICE_ID


namespace tool {

  namespace fpga {

    namespace test {

void loadBit(const char* file_name)
{
  int          rv;
  int          fildes;
  struct stat  stat_buff;
  ssize_t      num_bytes;
  uint8_t*     buffer;

  fildes = open( file_name, O_RDONLY );
  if (fildes == -1) {
    printk("%s: Unable to open filename '%s'\n", __func__, file_name);
    return;
  }

  rv = fstat( fildes, &stat_buff );
  if (rv != 0) {
    printk("%s: fstat failed: 0x%08x\n", __func__, rv);
    return;
  }

  buffer = new uint8_t[stat_buff.st_size];
  if (buffer == 0) {
    printk("%s: Insufficient memory available\n", __func__);
    return;
  }

  num_bytes = read(fildes, buffer, stat_buff.st_size);
  if (num_bytes != stat_buff.st_size) {
    printk("%s: read failed: 0x%08x vs 0x%08llx\n", __func__, num_bytes, stat_buff.st_size);
    delete [] buffer;
    return;
  }

  rv = close(fildes);
  if (rv != 0) {
    printk("%s: close failed: 0x%08x\n", __func__, rv);
    delete [] buffer;
    return;
  }

  printk("%s: 6, size = %u\n", __func__, stat_buff.st_size);

  FPGA_t* fpga = TOOL_FPGA_initialize(DCFG_DEVICE_ID);
  if (fpga == 0) {
    printk("%s: TOOL_FPGA_init failed\n", __func__);
    delete [] buffer;
    return;
  }

  rv = TOOL_FPGA_loadBit(fpga, buffer, stat_buff.st_size);
  if (rv != 0) {
    printk("%s: TOOL_FPGA_load failed: 0x%08x\n", __func__, rv);
    delete [] buffer;
    return;
  }

  rv = TOOL_FPGA_enable(fpga);
  if (rv != 0) {
    printk("%s: TOOL_FPGA_enable failed: 0x%08x\n", __func__, rv);
    delete [] buffer;
    return;
  }

  delete [] buffer;
  TOOL_FPGA_teardown(fpga);
}


extern "C" void rce_appmain()
{
  uint64_t t0 = tool::time::lticks();

  loadBit("/mnt/user/fpga.bit");

  timespec t = TOOL_TIME_t2s(tool::time::lticks() - t0);
  tool::debug::printv("fpgaLoad: Exiting; Run time: %u.%03u seconds\n",
                      t.tv_sec, t.tv_nsec / 1000000);
}

    } // test

  } // fpga

} // tool
