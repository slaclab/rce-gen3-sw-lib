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
#include <fcntl.h>
#include <sys/mman.h>

#include "memory/mem.h"

#include "map/Lookup.h"
#include "map/MapAxi.h"
#include "map/MapBsi.h"
#include "map/MapOcm.h"
#include "map/MapGic.h"

#define REGION_MAP_SIZE  4096

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
  int fd;
  void *mapped_base;    
  uint32_t start,size;

  fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1)
    {
    return 0;
    }

  start = (uint32_t)mem_Region_startAddress(MEM_REGION_SOCKET);
  size = mem_Region_size(MEM_REGION_SOCKET);
  
  mapped_base = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start);
  if (mapped_base == (void *) -1)
    return 0;    
  else
    return (Axi)mapped_base;
  } 

/*
** ++
**
**
** --
*/

Axi LookupAxi1(void)
  {
  int fd;
  void *mapped_base;    
  uint32_t start,size;

  fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1)
    {
    return 0;
    }

  start = (uint32_t)mem_Region_startAddress(MEM_REGION_FW_VER_CTL);
  size = mem_Region_size(MEM_REGION_FW_VER_CTL);

  mapped_base = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start);
  if (mapped_base == (void *) -1)
    return 0;    
  else
    return (Axi)mapped_base;
  } 

/*
** ++
**
**
** --
*/

Axi LookupAxiSocket(void)
  {
  int fd;
  void *mapped_base;    
  uint32_t start,size;

  fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1)
    {
    return 0;
    }

  start = (uint32_t)mem_Region_startAddress(MEM_REGION_SOCKET);
  size = mem_Region_size(MEM_REGION_SOCKET);
  
  mapped_base = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start);
  if (mapped_base == (void *) -1)
    return 0;    
  else
    return (Axi)mapped_base;
  } 

/*
** ++
**
**
** --
*/

Axi LookupAxiFw(void)
  {
  int fd;
  void *mapped_base;    
  uint32_t start,size;

  fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1)
    {
    return 0;
    }

  start = (uint32_t)mem_Region_startAddress(MEM_REGION_FW_VER_CTL);
  size = mem_Region_size(MEM_REGION_FW_VER_CTL);

  mapped_base = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start);
  if (mapped_base == (void *) -1)
    return 0;    
  else
    return (Axi)mapped_base;
  } 

/*
** ++
**
**
** --
*/

uint32_t LookupRegion(uint32_t region, uint32_t offset)
  {
  int fd;
  void *mapped_base;    
  uint32_t start,size;

  fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1)
    {
    return 0;
    }

  start = (uint32_t)mem_Region_startAddress(region);
  if(!start) return 0;
  size = mem_Region_size(region);
  if((offset+REGION_MAP_SIZE) > size) return 0;
  start += offset;
  mapped_base = mmap(0, REGION_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start);
  if (mapped_base == (void *) -1)
    return 0;    
  else
    return (uint32_t)mapped_base;
  } 

/*
** ++
**
**
** --
*/

Bsi LookupBsi(void)
  {
  int fd;
  void *mapped_base;    
  uint32_t start,size;

  fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1)
    {
    return 0;
    }

  start = (uint32_t)mem_Region_startAddress(MEM_REGION_BSI);
  size = mem_Region_size(MEM_REGION_BSI);

  mapped_base = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start);
  if (mapped_base == (void *) -1)
    return 0;
  else
    return (Bsi)mapped_base;
  } 

/*
** ++
**
**
** --
*/

Ocm LookupOcm(void)
  {
  int fd;
  void *mapped_base;    
  uint32_t start,size;

  fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1)
    {
    return 0;
    }

  start = (uint32_t)mem_Region_startAddress(MEM_REGION_OCM);
  size = mem_Region_size(MEM_REGION_OCM);

  mapped_base = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start);
  if (mapped_base == (void *) -1)
    return 0;
  else
    return (Ocm)mapped_base;
  } 

/*
** ++
**
**
** --
*/

Gic LookupGic(void)
  {
  int fd;
  void *mapped_base;    
  uint32_t start,size;

  fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1)
    {
    return 0;
    }

  start = (uint32_t)mem_Region_startAddress(MEM_REGION_HIGHREG);
  size = GIC_MAP_SIZE;
  
  mapped_base = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, start+GIC_CPU_MAP_OFFSET);
  if (mapped_base == (void *) -1)
    return 0;
  else
    return (Gic)mapped_base;
  } 

/*
** ++
**
**
** --
*/

int TeardownAxi0(Axi axi)
  {
  return munmap((void *)axi, mem_Region_size(MEM_REGION_AXI0_TEST));
  } 

/*
** ++
**
**
** --
*/

int TeardownAxi1(Axi axi)
  {
  return munmap((void *)axi, mem_Region_size(MEM_REGION_AXI1_TEST));
  } 

/*
** ++
**
**
** --
*/

int TeardownAxiSocket(Axi axi)
  {
  return munmap((void *)axi, mem_Region_size(MEM_REGION_SOCKET));
  } 

/*
** ++
**
**
** --
*/

int TeardownAxiFw(Axi axi)
  {
  return munmap((void *)axi, mem_Region_size(MEM_REGION_FW_VER_CTL));
  } 

/*
** ++
**
**
** --
*/

int TeardownAxiRegion(uint32_t addr, uint32_t region)
  {
  return munmap((void *)addr, REGION_MAP_SIZE);
  } 

/*
** ++
**
**
** --
*/

int TeardownBsi(Bsi bsi)
  {
  return munmap((void *)bsi, mem_Region_size(MEM_REGION_BSI));
  } 


/*
** ++
**
**
** --
*/

int TeardownOcm(Ocm ocm)
  {
  return munmap((void *)ocm, mem_Region_size(MEM_REGION_OCM));
  } 

 /*
** ++
**
**
** --
*/

int TeardownGic(Gic gic)
  {
  return munmap((void *)gic, GIC_MAP_SIZE);
  } 

#ifdef __cplusplus
}
#endif /* __cplusplus */
