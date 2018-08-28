/**
*
* @brief   Load a firmware bitstream onto an FPGA
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    October 17, 2013 -- Created
*
* $Revision: 3125 $
*
* @verbatim:
*                               Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <stdio.h>
#include <malloc.h>

#include <rtems.h>
#include <rtems/status-checks.h>

#include "fpga/fpga.h"
#include "conversion/BSWP.h"
#include "time/cpu/time.h"

// pcap.h is in $(RELEASE_DIR)/xilinx/fsbl/src
// so need to add it to the rules.mk for this package
#include "pcap.h"
#include "xdevcfg.h"

typedef struct FPGA {
  XDcfg* xdCfg;
} FPGA_t;


/**
 * @brief Initialize the utility and its underlying layers
 *
 * @param devId The device ID to target
 * @return A pointer to the utility's control structure
 */
FPGA_t* TOOL_FPGA_initialize(uint16_t devId)
{
  int           rc;
  FPGA_t*       fpga;
  XDcfg_Config* cfg;

  fpga = malloc(sizeof(*fpga));
  if (!fpga) {
    RTEMS_SYSLOG("%s: No memory found for a control structure\n", __func__);
    return 0;
  }

  fpga->xdCfg = malloc(sizeof(*fpga->xdCfg));
  if (!fpga->xdCfg) {
    RTEMS_SYSLOG("%s: No memory found for the XDcfg control structure\n", __func__);
    return 0;
  }

  cfg = XDcfg_LookupConfig(devId);
  if (cfg == NULL) {
    RTEMS_SYSLOG("%s: XDcfg_LookupConfig for device ID %d failed\n",
                 __func__, devId);
    return 0;
  }

  rc = XDcfg_CfgInitialize(fpga->xdCfg, cfg, cfg->BaseAddr);
  if (rc != 0) {
    RTEMS_SYSLOG("%s: XDcfg_CfgInitialize for device ID %d failed; status = %d\n",
                 __func__, devId, rc);
    return 0;
  }

  // Enable and select the PCAP interface
  XDcfg_EnablePCAP(fpga->xdCfg);
  XDcfg_SetControlRegister(fpga->xdCfg, XDCFG_CTRL_PCAP_PR_MASK);

  return fpga;
}


/**
 * @brief Tear the utility down and release its resources
 *
 * @param fpga A pointer to the utility's control structure
 */
void TOOL_FPGA_teardown(FPGA_t* fpga)
{
  free(fpga->xdCfg);
  free(fpga);
}


/**
 * @brief Function for loading an FPGA with the contents of a .bin file
 *
 * @param fpga A handle as returned by @a TOOL_FPGA_init()
 * @param src  A buffer containing the source to load onto the fabric
 * @param size The size in bytes of the @a src buffer
 * @return A status value; 0 for success, some non-zero integer for failure
 */
unsigned TOOL_FPGA_loadBin(FPGA_t* fpga, uint8_t* src, size_t size)
{
  uint32_t* startAddr     = (uint32_t*)src;
  uint32_t* loadAddr      = (uint32_t*)XDCFG_DMA_INVALID_ADDRESS;
  uint32_t  imageLength   = size >> 2;  /* 32 bit words */
  uint32_t  dataLength    = 0;          /* 32 bit words */
  uint32_t  encryptedFlag = XDCFG_NON_SECURE_PCAP_WRITE;
  int             rc;

  // Buffer must be 64 byte aligned "to avoid crossing a 4K byte boundary"
  if ((uintptr_t)src & 0x3f) {
    RTEMS_SYSLOG("%s: Can't load a buffer (%p) that is not 64 byte aligned",
                 __func__, src);
    return -1;
  }

  // Make sure that the data is in memory for the DMA to operate correctly
  rtems_cache_flush_multiple_data_lines(src, size);

  // Clear DMA and PCAP Done interrupts
  XDcfg_IntrClear(fpga->xdCfg, XDCFG_IXR_D_P_DONE_MASK);

  uint64_t t0 = TOOL_TIME_lticks();

  // Transfer the bitstream
  rc = XDcfg_Transfer(fpga->xdCfg,
                      startAddr,
                      imageLength,
                      loadAddr,
                      dataLength,
                      encryptedFlag);
  if (rc != XST_SUCCESS) {
    RTEMS_SYSLOG("%s: XDcfg_Transfer failed: 0x%08x\n", __func__, rc);
    return rc;
  }

  // Poll PCAP Done interrupt
  volatile unsigned isr;
  do {
    isr = XDcfg_IntrGetStatus(fpga->xdCfg);
  }
  while ((isr & XDCFG_IXR_D_P_DONE_MASK) != XDCFG_IXR_D_P_DONE_MASK);

  uint64_t        dT = TOOL_TIME_lticks() - t0;
  struct timespec t  = TOOL_TIME_t2s(dT);
  RTEMS_SYSLOG("%s: 0x%08x = %u bytes loaded in %lu.%03lu seconds (%llu ticks)\n",
               __func__, size, size, t.tv_sec, t.tv_nsec / 1000000, dT);

  return rc;
}


/**
 * @brief Function for loading an FPGA with the contents of a .bit file
 *
 *        Note that this function can overwrite the @a src buffer in
 *        order to 64 byte align and/or byte swap the loadable portion.
 *
 * @param fpga A handle as returned by @a TOOL_FPGA_init()
 * @param src  A buffer containing the source to load onto the fabric
 * @param size The size in bytes of the @a src buffer
 * @return A status value; 0 for success, some non-zero integer for failure
 */
unsigned TOOL_FPGA_loadBit(FPGA_t* fpga, uint8_t* src, size_t size)
{

  // Find number of reserved bytes to skip
  uint8_t* p  = src;
  unsigned sz = (p[0] << 8) + p[1];

  p += sizeof(uint16_t) + sz;           // Skip reserved bytes
  p += sizeof(uint16_t);                // Skip unknown word

  if (*p++ != 'a') {
    RTEMS_SYSLOG("%s: Token '%c' not found\n", __func__, 'a');
    return -1;
  }

  // The design string
  sz = (p[0] << 8) + p[1];
  p += sizeof(uint16_t);

  if (strlen((char*)p) + 1 != sz) {     // +1 for trailing null
    RTEMS_SYSLOG("%s: Design size: %08x != %08x\n", __func__, strlen((char*)p) + 1, sz);
    return -1;
  }
  RTEMS_SYSLOG("%s: Design: %s\n", __func__, p);
  p += sz;

  if (*p++ != 'b') {
    RTEMS_SYSLOG("%s: Token '%c' not found\n", __func__, 'b');
    return -1;
  }

  // The part number
  sz = (p[0] << 8) + p[1];
  p += sizeof(uint16_t);

  if (strlen((char*)p) + 1 != sz) {     // +1 for trailing null
    RTEMS_SYSLOG("%s: Part size: %08x != %08x\n", __func__, strlen((char*)p) + 1, sz);
    return -1;
  }
  RTEMS_SYSLOG("%s: Part #: %s\n", __func__, p);
  p += sz;

  if (*p++ != 'c') {
    RTEMS_SYSLOG("%s: Token '%c' not found\n", __func__, 'c');
    return -1;
  }

  // The date stamp
  sz = (p[0] << 8) + p[1];
  p += sizeof(uint16_t);

  if (strlen((char*)p) + 1 != sz) {     // +1 for trailing null
    RTEMS_SYSLOG("%s: Date size: %08x != %08x\n", __func__, strlen((char*)p) + 1, sz);
    return -1;
  }
  RTEMS_SYSLOG("%s: Date:   %s\n", __func__, p);
  p += sz;

  if (*p++ != 'd') {
    RTEMS_SYSLOG("%s: Token '%c' not found\n", __func__, 'd');
    return -1;
  }

  // The time stamp
  sz = (p[0] << 8) + p[1];
  p += sizeof(uint16_t);

  if (strlen((char*)p) + 1 != sz) {     // +1 for trailing null
    RTEMS_SYSLOG("%s: Time size: %08x != %08x\n", __func__, strlen((char*)p) + 1, sz);
    return -1;
  }
  RTEMS_SYSLOG("%s: Time:   %s\n", __func__, p);
  p += sz;

  if (*p++ != 'e') {
    RTEMS_SYSLOG("%s: Token '%c' not found\n", __func__, 'e');
    return -1;
  }

  // Get the ayload size
  sz = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
  p += sizeof(uint32_t);
  RTEMS_SYSLOG("%s: Size:   0x%08x = %u bytes\n", __func__, sz, sz);

  unsigned byteSwap = 0;
  int      i;
  for (i = 0; i < 13; ++i) {
    static const uint32_t rsvd[] =
      { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
        0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
        0x000000bb, 0x11220044, 0xffffffff, 0xffffffff, 0xaa995566 };

    if (((uint32_t*)src)[i] != rsvd[i]) {
      if (((uint32_t*)src)[i] != __builtin_bswap32(rsvd[i])) {
        RTEMS_SYSLOG("%s: Reserved word 0x%08lx not found at offset %08x: 0x%08lx\n",
                     __func__, rsvd[i], &src[i << 2] - src, ((uint32_t*)src)[i]);
        return -1;
      } else {
        ++byteSwap;
      }
    }
  }

  // Payload must be 64 byte aligned "to avoid crossing a 4K byte boundary"
  uint8_t* buf64 = (uint8_t*)((uintptr_t)p & ~0x3f);
  if (buf64 < src) {
    RTEMS_SYSLOG("%s: Can't 64 byte align payload (%p) in buffer provided (%p)",
                 __func__, p, src);
    return -1;
  }

  // The fabric is big endian: byte swap if necessary
  if (byteSwap == 3) {
    // Align the bitstream to load by overwriting the aligned buffer
    BSWP_swap32bN((uint32_t*)buf64, (uint32_t*)p, sz >> 2);
  } else if (byteSwap == 0) {
    // Align the bitstream to load by overwriting the aligned buffer
    memcpy((uint32_t*)buf64, (uint32_t*)p, sz >> 2);
  } else {
    RTEMS_SYSLOG("%s: Faulty data: %u of 3 reserved words are byte swapped\n",
                 __func__, byteSwap);
    return -1;
  }

  //uint32_t* b = (uint32_t*)src;
  //int i;
  //for (i = 0; i < 16; ++i) {
  //  printk("%08x: %08x %08x %08x %08x\n", (unsigned)b, b[0], b[1], b[2], b[3]);
  //  b += 4;
  //}

  return TOOL_FPGA_loadBin(fpga, buf64, sz);
}


/**
 * @brief Enable the level shifters
 * @return A status value; 0 for success, some non-zero integer for failure
 */
unsigned TOOL_FPGA_enable(FPGA_t* fpga)
{
  unsigned rc;

  rc = PcapEnablePartition();
  if (rc == XST_FAILURE) {
    RTEMS_SYSLOG("%s: PcapEnablePartition failed: 0x%08x\n", __func__, rc);
  }

  return rc;
}
