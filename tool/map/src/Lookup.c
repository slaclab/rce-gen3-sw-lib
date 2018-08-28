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

#include "memory/mem.h"

#include "map/Lookup.h"
#include "map/MapAxi.h"
#include "map/MapBsi.h"
#include "map/MapOcm.h"
#include "map/MapGic.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
** ++
**
**
** --
*/

Axi LookupAxi0(void)
  {
  /* 1 to 1 mapping */
  return (Axi)mem_Region_startAddress(MEM_REGION_AXI0_TEST);
  } 

/*
** ++
**
**
** --
*/

Axi LookupAxi1(void)
  {
  /* 1 to 1 mapping */
  return (Axi)mem_Region_startAddress(MEM_REGION_AXI1_TEST);
  } 

/*
** ++
**
**
** --
*/

Axi LookupAxiSocket(void)
  {
  /* 1 to 1 mapping */
  return (Axi)mem_Region_startAddress(MEM_REGION_SOCKET);
  } 

/*
** ++
**
**
** --
*/

Axi LookupAxiFw(void)
  {
  /* 1 to 1 mapping */
  return (Axi)mem_Region_startAddress(MEM_REGION_FW_VER_CTL);
  } 

/*
** ++
**
**
** --
*/

Axi LookupRegion(uint32_t region, uint32_t offset)
  {
  /* 1 to 1 mapping */
  return (Axi)((uint32_t)mem_Region_startAddress(region)+offset);
  } 

/*
** ++
**
**
** --
*/

Bsi LookupBsi(void)
  {
  /* 1 to 1 mapping */
  return (Bsi)mem_Region_startAddress(MEM_REGION_BSI);
  } 

/*
** ++
**
**
** --
*/

Ocm LookupOcm(void)
  {
  /* 1 to 1 mapping */
  return (Ocm)mem_Region_startAddress(MEM_REGION_OCM);
  } 

/*
** ++
**
**
** --
*/

Gic LookupGic(void)
  {
  /* 1 to 1 mapping */
  return (Gic)(mem_Region_startAddress(MEM_REGION_HIGHREG)+GIC_CPU_MAP_OFFSET);
  } 

 /*
** ++
**
**
** --
*/

int TeardownAxi0(Axi axi)
  {
  /* Nothing to unmap, no-op */
  return 0;
  } 

 /*
** ++
**
**
** --
*/

int TeardownAxi1(Axi axi)
  {
  /* Nothing to unmap, no-op */
  return 0;
  } 

 /*
** ++
**
**
** --
*/

int TeardownAxiSocket(Axi axi)
  {
  /* Nothing to unmap, no-op */
  return 0;
  } 

 /*
** ++
**
**
** --
*/

int TeardownAxiFw(Axi axi)
  {
  /* Nothing to unmap, no-op */
  return 0;
  } 

 /*
** ++
**
**
** --
*/

int TeardownRegion(uint32_t addr, uint32_t region)
  {
  /* Nothing to unmap, no-op */
  return 0;
  } 

 /*
** ++
**
**
** --
*/

int TeardownBsi(Bsi bsi)
  {
  /* Nothing to unmap, no-op */
  return 0;
  } 

 /*
** ++
**
**
** --
*/

int TeardownOcm(Ocm ocm)
  {
  /* Nothing to unmap, no-op */
  return 0;
  } 

 /*
** ++
**
**
** --
*/

int TeardownGic(Gic gic)
  {
  /* Nothing to unmap, no-op */
  return 0;
  } 

#ifdef __cplusplus
}
#endif /* __cplusplus */
