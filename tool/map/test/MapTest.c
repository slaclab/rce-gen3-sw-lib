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

#include "memory/mem.h"

#include "map/Lookup.h"
#include "map/MapAxi.h"
#include "map/MapBsi.h"
#include "map/MapOcm.h"
#include "map/MapGic.h"

/*
** ++
**
**
** --
*/

int main(int argc, char **argv)
  {  
  Axi socket;
  Axi fw;
  Axi axi0;
  Axi axi1;
  Bsi bsi;
  Ocm ocm;
  Gic gic;
  
  socket = LookupAxiSocket();  
  if(!socket)
    {
    printf("Failure mapping AXI socket)\n");
    return -1;
    }
  else
    printf("Mapped AXI socket to 0x%x %d bytes\n",(int)socket,mem_Region_size(MEM_REGION_SOCKET));

  fw = LookupAxiFw();  
  if(!fw)
    {
    printf("Failure mapping AXI fw)\n");
    return -1;
    }
  else
    printf("Mapped AXI fw to 0x%x %d bytes\n",(int)fw,mem_Region_size(MEM_REGION_FW_VER_CTL));

  axi0 = LookupAxi0();  
  if(!axi0)
    {
    printf("Failure mapping AXI0)\n");
    return -1;
    }
  else
    printf("Mapped AXI0 to 0x%x %d bytes\n",(int)axi0,mem_Region_size(MEM_REGION_AXI0_TEST));

  axi1 = LookupAxi1();  
  if(!axi1)
    {
    printf("Failure mapping AXI1)\n");
    return -1;
    }
  else
    printf("Mapped AXI1 to 0x%x %d bytes\n",(int)axi1,mem_Region_size(MEM_REGION_AXI1_TEST));

  bsi = LookupBsi();  
  if(!bsi)
    {
    printf("Failure mapping BSI\n");
    return -1;
    }
  else
    printf("Mapped BSI to 0x%x %d bytes\n",(int)bsi,mem_Region_size(MEM_REGION_BSI));
      
  ocm = LookupOcm();
  if(!ocm)
    {
    printf("Failure mapping OCM\n");
    return -1;
    }
  else
    printf("Mapped OCM to 0x%x %d bytes\n",(int)ocm,mem_Region_size(MEM_REGION_OCM));

  gic = LookupGic();
  if(!gic)
    {
    printf("Failure mapping GIC\n");
    return -1;
    }
  else
    printf("Mapped GIC to 0x%x %d bytes\n",(int)gic,GIC_MAP_SIZE);
  
  TeardownAxiSocket(socket);
  TeardownAxiFw(fw);
  TeardownAxi0(axi0);
  TeardownAxi1(axi1);
  TeardownBsi(bsi);
  TeardownOcm(ocm);
  TeardownGic(gic);
  
  return 0;
  }
