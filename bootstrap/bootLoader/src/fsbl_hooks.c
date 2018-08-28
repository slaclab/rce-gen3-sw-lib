/*!@file     fsbl_hooks.c
*
* @brief     First Stage BootLoader hooks
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)

* @date      April 30, 2013 -- Created
*
* $Revision: 2123 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

/*****************************************************************************
*
* @file fsbl_hooks.c
*
* This file provides functions that serve as user hooks.  The user can add the
* additional functionality required into these routines.  This would help retain
* the normal FSBL flow unchanged.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 3.00a np   08/03/12 Initial release
* </pre>
*
* @note
*
******************************************************************************/

#include <malloc.h>

#include "fsbl.h"
#include "sd.h"
#include "ff.h"
#include "xstatus.h"
#include "xil_cache.h"

#include "handoff.h"
#include "gpio.h"
#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, elf.h)
#include DAT_GENERATION(service, cmb, BSI_cfg.h)

/* Define to load an ELF file according to boot.prm and pass control to it */
#if 0
#  define HANDOFF
#endif

FRESULT readHex(FIL* fil, u32* buf);
u32     loadMAC(char*, u8*);
u32     loadPrm(char*, u32*, u32*);
u32     loadElf(u32*, u32);
/*u32     loadRAM(u32*, u32);*/

#define FALSE        0
#define TRUE         1
#define FPGA_VERSION 0x80000000       /* Location of firmware version number */
#define BSI_BRAM     0x84000000       /* Location of the BSI BRAM */
#define CL_NODE      0                /* Support only one node for now */

/*
 * Following constant is used to determine which Bank of the GPIO is
 * used for the BSI_READY pin.
 */
#define OUTPUT_BANK	XGPIOPS_BANK0	/* Bank to be used for BSI_READY */

#define BSI_READY	0xE	/* GPIO pin number of the BSI_READY signal */

/************************** Variable Definitions *****************************/


/************************** Function Prototypes ******************************/


/******************************************************************************
* This function is the hook which will be called  before the bitstream download.
* The user can add all the customized code required to be executed before the
* bitstream download to this routine.
*
* @param None
*
* @return
*		- XST_SUCCESS to indicate success
*		- XST_FAILURE.to indicate failure
*
****************************************************************************/
u32 FsblHookBeforeBitstreamDload(void)
{
	u32 Status;

	Status = XST_SUCCESS;

	/*
	 * User logic to be added here. Errors to be stored in the status variable
	 * and returned
	 */
	fsbl_printf(DEBUG_INFO, "In %s function\r\n", __func__);

	return (Status);
}

/******************************************************************************
* This function is the hook which will be called  after the bitstream download.
* The user can add all the customized code required to be executed after the
* bitstream download to this routine.
*
* @param None
*
* @return
*		- XST_SUCCESS to indicate success
*		- XST_FAILURE.to indicate failure
*
****************************************************************************/
u32 FsblHookAfterBitstreamDload(void)
{
	u32 Status;

	Status = XST_SUCCESS;

	/*
	 * User logic to be added here.
	 * Errors to be stored in the status variable and returned
	 */
	fsbl_printf(DEBUG_INFO, "In %s function\r\n", __func__);

	return (Status);
}

/******************************************************************************
* This function is the hook which will be called  before the FSBL does a handoff
* to the application. The user can add all the customized code required to be
* executed before the handoff to this routine.
*
* @param None
*
* @return
*		- XST_SUCCESS to indicate success
*		- XST_FAILURE.to indicate failure
*
****************************************************************************/
u32 FsblHookBeforeHandoff(void)
{
  u32  status     = XST_SUCCESS;
  u8   mac[6];
  union {
    UINT u32[2];
    char u8[8];
  }    macAdx;
  u32  phy;
  u32* bsiBram    = (u32*)BSI_BRAM;
#ifdef HANDOFF
  u32* handoffAdx = (u32*)HANDOFF_ADDR;
  u32  xfrAdx;
#endif
  u32  bsLoader;
  int  i;

  /*
   * User logic to be added here.
   * Errors to be stored in the status variable and returned
   */
  fsbl_printf(DEBUG_INFO,"In %s function\r\n", __func__);

  fsbl_printf(DEBUG_INFO, "%s: FPGA version = %08x\r\n", __func__,
              *(u32*)FPGA_VERSION);

  /* Read the MAC address for this board */
  status = loadMAC("BOOT.MAC", mac);
  if (status != XST_SUCCESS)  return status;

  /* Read the boot parameters */
  status = loadPrm("BOOT.PRM", &phy, &bsLoader);
  if (status != XST_SUCCESS)  return status;

  /* Copy our BSI parameters into the BSI BRAM for the IPMI Controller */
  macAdx.u8[7] = 0;
  macAdx.u8[6] = 0;
  for (i = 0; i < sizeof(mac); ++i)
    macAdx.u8[sizeof(mac) - 1 - i] = mac[i];

  bsiBram[BSI_CLUSTER_CFG_VERSION_OFFSET]               = 0xfe000009U;     // 0x00: version_size ( -2, 9 )
  bsiBram[BSI_NETWORK_PHY_TYPE_OFFSET]                  = phy;             // 0x01: Network PHY
  bsiBram[BSI_MAC_ADDR_OFFSET + 0]                      = macAdx.u32[0];   // 0x02: MAC address
  bsiBram[BSI_MAC_ADDR_OFFSET + 1]                      = macAdx.u32[1];
  bsiBram[BSI_INTERCONNECT_OFFSET]                      = 0xdeadbeefU;     // 0x04: Interconnect
  bsiBram[BSI_CMB_SERIAL_NUMBER_OFFSET + 0]             = 0xcafebabeU;     // 0x10: Serial number
  bsiBram[BSI_CMB_SERIAL_NUMBER_OFFSET + 1]             = 0x0000abadU;
  bsiBram[BSI_CLUSTER_ADDR_OFFSET]                      = 0x00000102;      // 0x12: Site, bay and element
  bsiBram[BSI_CLUSTER_GROUP_NAME_OFFSET + 0]            = 0x6c656853;      // 0x13: "Shelf"
  bsiBram[BSI_CLUSTER_GROUP_NAME_OFFSET + 1]            = 0x00000066;
  bsiBram[BSI_CLUSTER_GROUP_NAME_OFFSET + 2]            = 0;
  bsiBram[BSI_CLUSTER_GROUP_NAME_OFFSET + 3]            = 0;
  bsiBram[BSI_CLUSTER_GROUP_NAME_OFFSET + 4]            = 0;
  bsiBram[BSI_CLUSTER_GROUP_NAME_OFFSET + 5]            = 0;
  bsiBram[BSI_CLUSTER_GROUP_NAME_OFFSET + 6]            = 0;
  bsiBram[BSI_CLUSTER_GROUP_NAME_OFFSET + 7]            = 0;
  bsiBram[BSI_EXTERNAL_INTERCONNECT_OFFSET]             = 0x01020304;      // 0x1b: ext interconnect
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET +  0] = 0x05060708;      // 0x30: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET +  1] = 0x090a0b0c;      // 0x31: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET +  2] = 0x0d0e0f10;      // 0x32: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET +  3] = 0x11121314;      // 0x33: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET +  4] = 0x15161718;      // 0x34: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET +  5] = 0x191a1b1c;      // 0x35: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET +  6] = 0x1d1e1f20;      // 0x36: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET +  7] = 0x21222324;      // 0x37: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET +  8] = 0x25262728;      // 0x38: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET +  9] = 0x292a2b2c;      // 0x39: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 10] = 0x2d2e2f30;      // 0x3a: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 11] = 0x31323334;      // 0x3b: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 12] = 0x35363738;      // 0x3c: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 13] = 0x393a3b3c;      // 0x3d: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 14] = 0x3d3e3f40;      // 0x3e: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 15] = 0x41424344;      // 0x3f: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 16] = 0x45464748;      // 0x40: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 17] = 0x494a4b4c;      // 0x41: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 18] = 0x4d4e4f50;      // 0x42: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 19] = 0x51525354;      // 0x43: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 20] = 0x55565758;      // 0x44: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 21] = 0x595a5b5c;      // 0x45: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 22] = 0x5d5e5f60;      // 0x46: Switch configuration
  bsiBram[BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + 23] = 0x61626364;      // 0x47: Switch configuration
  bsiBram[BSI_CLUSTER_ELEMENT_INTERCONNECT_OFFSET]      = 0x65666768;      // 0x48: Cluster Element interconnect

  /* Make sure the data gets flushed from cache */
  Xil_DCacheFlushRange(BSI_BRAM, BSI_CFG_SIZE);

  /* Initialize the GPIO subsystem */
  GpioInit();

  /* Tell the IPMI Controller that our BSI parameters are ready for reading */
  GpioSet(OUTPUT_BANK, BSI_READY);

#ifdef HANDOFF
  /* Now load the bootstrap */
  status = loadElf(&xfrAdx, bsLoader);
  //status = loadRAM(&xfrAdx, bsLoader);

  /* Store the transfer address in the shared well-known location */
  *handoffAdx = xfrAdx;

  fsbl_printf(DEBUG_INFO, "%s: Control will be transferred to: %08x\r\n",
              __func__, *handoffAdx);
#endif

  return status;
}


/******************************************************************************
* This function is the hook which will be called in case FSBL fall back
*
* @param None
*
* @return None
*
****************************************************************************/
void FsblHookFallback(void)
{
	/*
	 * User logic to be added here.
	 * Errors to be stored in the status variable and returned
	 */
  fsbl_printf(DEBUG_INFO,"In %s function \r\n", __func__);
	while(1);
}


FRESULT readHex(FIL* fil, u32* buf)
{
  int   n = 0;
  TCHAR c;
  UINT  rc, br;
  u32   v = 0;

  do {					/* Read bytes until buffer gets filled */
    rc = f_read(fil, &c, 1, &br);
    if (br != 1)  return rc;	/* Return on EOF or error */
    v <<= 4;
    if      (c >= '0' && c <= '9')  v |= c - '0';
    else if (c >= 'A' && c <= 'F')  v |= c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')  v |= c - 'a' + 10;
    else {                  n = 0;  v  = 0; } /* No digit: reset */
  } while (++n < 2 * sizeof(*buf));        /* 2 characters per byte */
  *buf = v;
  return 0;
}


u32 loadMAC(char* macFile, u8* mac)
{
  FIL         fil;                /* File object */
  char        buffer[32];
  char*       the_file = buffer;
  FRESULT     rc;                 /* Result code */
  UINT        br;

  strcpy_rom(buffer, macFile);
  the_file = (char*)buffer;

  rc = f_open(&fil, the_file, FA_READ);
  if (rc) {
    fsbl_printf(DEBUG_GENERAL, "%s: Unable to open file %s: %d\r\n",
                __func__, the_file, rc);
    return XST_FAILURE;
  }

  rc = f_read(&fil, (void*)mac, 6 * sizeof(*mac), &br);
  if (rc) {
    fsbl_printf(DEBUG_GENERAL, "%s: *** ERROR: f_read returned %d\r\n",
                __func__, rc);
  }
  fsbl_printf(DEBUG_GENERAL, "%s: MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n", __func__,
              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  f_close(&fil);

  return XST_SUCCESS;
}


u32 loadPrm(char* prmFile, u32 *phy, u32 *bsLoader)
{
  FIL         fil;                /* File object */
  char        buffer[32];
  char*       the_file = buffer;
  FRESULT     rc;                 /* Result code */

  strcpy_rom(buffer, prmFile);
  the_file = (char*)buffer;

  rc = f_open(&fil, the_file, FA_READ);
  if (rc) {
    fsbl_printf(DEBUG_GENERAL, "%s: Unable to open file %s: %d\r\n",
                __func__, the_file, rc);
    return XST_FAILURE;
  }

  rc = readHex(&fil, bsLoader);
  if (rc) {
    fsbl_printf(DEBUG_GENERAL, "%s: Unable to read %s from %s: %d\r\n",
                __func__, "os", the_file, rc);
    return XST_FAILURE;
  }
  fsbl_printf(DEBUG_GENERAL, "%s: %s = %08x\r\n", __func__, "O/S", *bsLoader);

  rc = readHex(&fil, phy);
  if (rc) {
    fsbl_printf(DEBUG_GENERAL, "%s: Unable to read %s from %s: %d\r\n",
                __func__, "phy", the_file, rc);
      return XST_FAILURE;
  }
  fsbl_printf(DEBUG_GENERAL, "%s: %s = %08x\r\n", __func__, "PHY", *phy);

  f_close(&fil);

  return XST_SUCCESS;
}


#ifdef HANDOFF
u32 loadElf(u32* xfrAdx, u32 bsLoader)
{
  FIL         fil;                /* File object */
  char        the_file[32];
  char*       b;
  FRESULT     rc;                 /* Result code */
  UINT        br;
  u32         os  = (bsLoader & 0xfff00000) >> 20;
  u32         ver = (bsLoader & 0x000fffff) >>  0;
  const char* hex = "0123456789abcdef";
  int         i;
  Elf32_Ehdr  eh;
  Elf32_Phdr* ph;
  u32*        mag;

  /* Make sure xfrAdr is initialized to something useful */
  *xfrAdx = 0x00000000;

  /* Form the path to the file */
  b = the_file;
  *b++ = '/';
  for (i = 0; i < 8; ++i) { *b++ = hex[(os  >> 28) & 0xf]; os  <<= 4; }
  *b++ = '/';
  for (i = 0; i < 8; ++i) { *b++ = hex[(ver >> 28) & 0xf]; ver <<= 4; }
  *b++ = '\0';
  strcat(the_file, ".elf");

  /* Open it */
  rc = f_open(&fil, the_file, FA_READ);
  if (rc) {
    fsbl_printf(DEBUG_GENERAL, "%s: Unable to open file '%s': %d\r\n",
                __func__, the_file, rc);
    return XST_FAILURE;
  }

  /* Read the ELF header */
  rc = f_read(&fil, (void*)&eh, sizeof(eh), &br);
  if (rc) {
    fsbl_printf(DEBUG_GENERAL, "%s: Unable to read %s from %s: %d\r\n",
                __func__, "Ehdr", the_file, rc);
      return XST_FAILURE;
  }

  fsbl_printf(DEBUG_GENERAL, "e_machine   = %08x\r\n", eh.e_machine);
  fsbl_printf(DEBUG_GENERAL, "e_phoff     = %08x\r\n", eh.e_phoff);
  fsbl_printf(DEBUG_GENERAL, "e_phentsize = %08x\r\n", eh.e_phentsize);
  fsbl_printf(DEBUG_GENERAL, "e_phnum     = %08x\r\n", eh.e_phnum);
  fsbl_printf(DEBUG_GENERAL, "e_entry     = %08x\r\n\n", eh.e_entry);

  /* Magic number matches? */
  mag = (u32*)eh.e_ident;
  if (*mag != ELFMAG) {
    fsbl_printf(DEBUG_GENERAL, "%s: Bad ELF magic number for %s: %08x\r\n",
                __func__, the_file, *mag);
    return XST_FAILURE;
  }

  /* Machine type matches? */
  if (eh.e_machine != EM_ARM) {
    fsbl_printf(DEBUG_GENERAL, "%s: Wrong machine type for %s: %u\r\n",
                __func__, the_file, eh.e_machine);
    return XST_FAILURE;
  }

  /* Allocate space for the program headers */
  ph = (Elf32_Phdr*)malloc(eh.e_phnum * sizeof(*ph));
  if (ph == NULL) {
    fsbl_printf(DEBUG_GENERAL, "%s: Insufficient space to load %u Phdrs for %s\r\n",
                __func__, eh.e_phnum, the_file);
    return XST_FAILURE;
  }

  /* Seek to the beginning of the program headers */
  rc = f_lseek(&fil, eh.e_phoff);
  if (rc) {
    fsbl_printf(DEBUG_GENERAL, "%s: f_lseek failed for %s: %d\r\n",
                __func__, the_file, rc);
    return XST_FAILURE;
  }

  /* Read all the program headers in one fell swoop */
  rc = f_read(&fil, (void*)ph, eh.e_phnum * sizeof(*ph), &br);
  if (rc) {
    fsbl_printf(DEBUG_GENERAL, "%s: Unable to read %s from %s: %d\r\n",
                __func__, "Phdr", the_file, rc);
      return XST_FAILURE;
  }

  /* Loop over program headers, loading them as needed */
  for (i = 0; i < eh.e_phnum; ++ph, ++i) {
    fsbl_printf(DEBUG_GENERAL, "p_type      = %08x\r\n", ph->p_type);
    fsbl_printf(DEBUG_GENERAL, "p_offset    = %08x\r\n", ph->p_offset);
    fsbl_printf(DEBUG_GENERAL, "p_paddr     = %08x\r\n", ph->p_paddr);
    fsbl_printf(DEBUG_GENERAL, "p_vaddr     = %08x\r\n", ph->p_vaddr);
    fsbl_printf(DEBUG_GENERAL, "p_memsz     = %08x\r\n", ph->p_memsz);
    fsbl_printf(DEBUG_GENERAL, "p_filesz    = %08x\r\n\n", ph->p_filesz);

    /* Skip segment types that are not marked PT_LOAD */
    //if (ph->p_type == PT_LOAD) {
      /* Seek to the beginning of the program section */
      rc = f_lseek(&fil, ph->p_offset);
      if (rc) {
        fsbl_printf(DEBUG_GENERAL, "%s: f_lseek failed for %s: %d\r\n",
                    __func__, the_file, rc);
        return XST_FAILURE;
      }
      /* Load the program section directly into its final destination */
      rc = f_read(&fil, (void*)ph->p_paddr, ph->p_filesz, &br);
      if (rc) {
        fsbl_printf(DEBUG_GENERAL, "%s: Unable to read %s from %s: %d\r\n",
                    __func__, "segment", the_file, rc);
        return XST_FAILURE;
      } else {
        unsigned* p = (unsigned*)ph->p_paddr;
        fsbl_printf(DEBUG_GENERAL, "segment: %08x - %08x: %08x %08x %08x %08x\r\n",
                    ph->p_paddr, ph->p_paddr + ph->p_filesz,
                    p[0], p[1], p[2], p[3]);
      }
      //if (ph->p_filesz < ph->p_memsz) {
      //  memset((void*)(ph->p_paddr + ph->p_filesz), 0x0, ph->p_memsz - ph->p_filesz);
      //}
      /* Make sure the data gets flushed from cache */
      Xil_DCacheFlushRange(ph->p_paddr, ph->p_memsz);
      //}
  }

  free(ph);
  f_close(&fil);

  /* Return the transfer address */
  *xfrAdx = eh.e_entry;

  return XST_SUCCESS;
}
#endif


#if 0
u32 loadRAM(u32* xfrAdx, u32 bsLoader)
{
  u32         os  = (bsLoader & 0xfff00000) >> 20;
  u32         ver = (bsLoader & 0x000fffff) >>  0;
  int         i;
  Elf32_Ehdr* eh;
  Elf32_Phdr* ph;
  u32*        mag;
  u32         srcAdx;

  /* Make sure xfrAdr is initialized to something useful */
  *xfrAdx = 0x00000000;

  srcAdx = 0x08000000;

  /* Read the ELF header */
  eh = (Elf32_Ehdr*)srcAdx;

  fsbl_printf(DEBUG_GENERAL, "e_machine   = %08x\r\n", eh->e_machine);
  fsbl_printf(DEBUG_GENERAL, "e_phoff     = %08x\r\n", eh->e_phoff);
  fsbl_printf(DEBUG_GENERAL, "e_phentsize = %08x\r\n", eh->e_phentsize);
  fsbl_printf(DEBUG_GENERAL, "e_phnum     = %08x\r\n", eh->e_phnum);
  fsbl_printf(DEBUG_GENERAL, "e_entry     = %08x\r\n\n", eh->e_entry);

  /* Magic number matches? */
  mag = (u32*)eh->e_ident;
  if (*mag != ELFMAG) {
    fsbl_printf(DEBUG_GENERAL, "%s: Bad ELF magic number for %08x: %08x\r\n",
                __func__, srcAdx, *mag);
    return XST_FAILURE;
  }

  /* Machine type matches? */
  if (eh->e_machine != EM_ARM) {
    fsbl_printf(DEBUG_GENERAL, "%s: Wrong machine type for %08x: %u\r\n",
                __func__, srcAdx, eh->e_machine);
    return XST_FAILURE;
  }

  /* Read all the program headers in one fell swoop */
  ph = (Elf32_Phdr*)(srcAdx + eh->e_phoff);

  /* Loop over program headers, loading them as needed */
  for (i = 0; i < eh->e_phnum; ++ph, ++i) {
    fsbl_printf(DEBUG_GENERAL, "p_type      = %08x\r\n", ph->p_type);
    fsbl_printf(DEBUG_GENERAL, "p_offset    = %08x\r\n", ph->p_offset);
    fsbl_printf(DEBUG_GENERAL, "p_paddr     = %08x\r\n", ph->p_paddr);
    fsbl_printf(DEBUG_GENERAL, "p_vaddr     = %08x\r\n", ph->p_vaddr);
    fsbl_printf(DEBUG_GENERAL, "p_memsz     = %08x\r\n", ph->p_memsz);
    fsbl_printf(DEBUG_GENERAL, "p_filesz    = %08x\r\n\n", ph->p_filesz);

    /* Skip segment types that are not marked PT_LOAD */
    if (ph->p_type == PT_LOAD) {
      /* Seek to the beginning of the program section */
      memcpy((void*)ph->p_paddr, (void*)(srcAdx + ph->p_offset), ph->p_filesz);
#if 0
      if (ph->p_filesz < ph->p_memsz) {
        memset((void*)(ph->p_paddr + ph->p_filesz), 0x0, ph->p_memsz - ph->p_filesz);
      }
#endif
      {
        unsigned* p = (unsigned*)ph->p_paddr;
        fsbl_printf(DEBUG_GENERAL, "segment: %08x - %08x: %08x %08x %08x %08x\r\n",
                    ph->p_paddr, ph->p_paddr + ph->p_filesz,
                    p[0], p[1], p[2], p[3]);
      }
      /* Make sure the data gets flushed from cache */
      Xil_DCacheFlushRange(ph->p_paddr, ph->p_memsz);
    }
  }

  free(ph);

  /* Return the transfer address */
  *xfrAdx = eh->e_entry;

  return XST_SUCCESS;
}
#endif
