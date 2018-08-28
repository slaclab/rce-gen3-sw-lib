/*
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	    000 - July 19, 2013
**
**  Revision History:
**	    None.
**
** --
*/



#include <stdio.h>
#include <string.h>

#include "map/Lookup.h"
#include "map/MapOcm.h"
#include "map/MapAxi.h"

#include "conversion/impl/BSWP.ih"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"

/*
** ++
**
**
** --
*/
const char* AxiReadBldInfo(Axi axi, char value[AXI_FWBUILD_MEM_SIZE])
  {
  unsigned* ap = (unsigned *)axi+AXI_FWBUILD_OFFSET;
  unsigned* val = (unsigned*)value;
  unsigned  idx = 0;
  do {
    *val++ = ap[idx];
  }
  while (++idx < (AXI_FWBUILD_MEM_SIZE/4) - 1);
  return value;
  }

/*
** ++
**
**
** --
*/
const char* BsiReadUboot(Bsi bsi, char value[BSI_UBOOT_VERSION_SIZE * sizeof(unsigned)])
  {
  unsigned* val = (unsigned*)value;
  unsigned  idx = 0;
  do {
    *val++ = BsiRead32(bsi,BSI_UBOOT_VERSION_OFFSET+idx);
  }
  while (++idx < BSI_UBOOT_VERSION_SIZE - 1);
  return value;
  }

/*
** ++
**
**
** --
*/
const char* BsiReadDat(Bsi bsi, char value[BSI_DAT_VERSION_SIZE * sizeof(unsigned)])
  {
  unsigned* val = (unsigned*)value;
  unsigned  idx = 0;
  do {
    *val++ = BsiRead32(bsi,BSI_DAT_VERSION_OFFSET+idx);
  }
  while (++idx < BSI_DAT_VERSION_SIZE - 1);
  return value;
  }

/*
** ++
**
**
** --
*/

int dumpBootVersion(void)
  {
  char hostname[1024];
  const char *group;
  uint32_t addr;
  uint32_t cluster;
  uint32_t bay;
  uint32_t element;
  uint64_t data;

  char bldinfo[AXI_FWBUILD_MEM_SIZE  * sizeof(unsigned)];
  char uboot[BSI_UBOOT_VERSION_SIZE  * sizeof(unsigned)];
  char dat[BSI_DAT_VERSION_SIZE  * sizeof(unsigned)];
  
  Axi axi;
  Bsi bsi;
  
  axi = LookupAxiFw();
  if(!axi)
    {
    printf("Failure mapping AXI\n");
    return -1;
    }
  else
    {
    AxiReadBldInfo(axi,bldinfo);
    printf("Firmware info:\n");
    printf("  bitfile version: 0x%x\n",*(uint32_t*)(axi+AXI_FWBITVER_OFFSET));
    printf("  ARM module version: 0x%x\n",*(uint32_t*)(axi+AXI_FWMODVER_OFFSET));
    printf("  %s\n",bldinfo);
    printf("  refclk0 freq sel: 0x%x\n",*(uint32_t*)(axi+AXI_CLKSEL0_OFFSET));
    printf("  refclk1 freq sel: 0x%x\n",*(uint32_t*)(axi+AXI_CLKSEL1_OFFSET));

    uint64_t dna = ((uint64_t)(*(uint32_t*)(axi+AXI_DNA0_OFFSET))<<32);
    dna |= (uint64_t)(*(uint32_t*)(axi+AXI_DNA1_OFFSET));
    printf("Zynq info:\n");    
    printf("  Device DNA: 0x%llx\n",dna);
    printf("  eFUSE: 0x%x\n",*(uint32_t*)(axi+AXI_EFUSE_OFFSET));

    }
  
  bsi = LookupBsi();  
  if(!bsi)
    {
    printf("Failure mapping BSI\n");
    return -1;
    }
  else
    {
    printf("BSI info:\n");
    data = BsiRead64(bsi,BSI_CMB_SERIAL_NUMBER_OFFSET);
    printf("  HW ID: 0x%llx\n",data);
    BsiReadUboot(bsi,uboot);
    BsiReadDat(bsi,dat);
    hostname[1023] = '\0';
    group = BsiReadGroup(bsi, hostname);
    addr = BsiRead32(bsi,BSI_CLUSTER_ADDR_OFFSET);
    cluster = BSI_CLUSTER_FROM_CLUSTER_ADDR(addr);
    bay = BSI_BAY_FROM_CLUSTER_ADDR(addr);
    element = BSI_ELEMENT_FROM_CLUSTER_ADDR(addr);
    if(!(*hostname == '\0'))
      {
      sprintf((char *)hostname,"%s/%d/%d/%d",group,cluster,bay,element);
      printf("  RCE ID: %s\n",hostname);
      }
    printf("  U-Boot version: %s\n",uboot);
    printf("  RPT sw tag: %s\n",dat);
    uint64_t mac = BsiRead64(bsi,BSI_MAC_ADDR_OFFSET);
    printf("  MAC address: 0x%llx\n",BSWP__swap64(mac)>>16);
    }
  
  TeardownBsi(bsi);
  TeardownAxiFw(axi);
  
  return 0;
  }

/*
** ++
**
**
** --
*/

void cleanup(void)
  {
  }
