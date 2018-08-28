/*
** ++
**  Package: SAS
**	
**
**  Abstract: SAS interrupt controller routines
**
**  Implementation of SAS interrupts. See the corresponding header (".h") file for a
**  more complete description of the functions implemented below.      
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - June 26, 2014
**
**  Revision History:
**	None.
**
** --
*/

#include "map/Lookup.h"
#include "sas/Sas.h"

#include "Map.h"
#include "Interrupt.h"

/*
** ++
**
** GIC interrupt control table.
**
** This table contains values and register offsets to control
** functionality in the GIC (General Interrupt Controller).
** index 0: interrupt IRQ line number
** index 1: bitmask to enable and disable the interrupt
** index 2: GIC register offset to enable the interrupt
** index 3: GIC register offset to disable the interrupt
** index 4: GIC register offset to clear the interrupt 
**
** --
*/
   
static const unsigned GicCtl[IRQ_COUNT][5] = {
  {INTR_0,INTR_0_MASK,INTR_0_ENABLE,INTR_0_DISABLE,INTR_0_CLEAR},
  {INTR_1,INTR_1_MASK,INTR_1_ENABLE,INTR_1_DISABLE,INTR_1_CLEAR},
  {INTR_2,INTR_2_MASK,INTR_2_ENABLE,INTR_2_DISABLE,INTR_2_CLEAR},
  {INTR_3,INTR_3_MASK,INTR_3_ENABLE,INTR_3_DISABLE,INTR_3_CLEAR},
  {INTR_4,INTR_4_MASK,INTR_4_ENABLE,INTR_4_DISABLE,INTR_4_CLEAR},
  {INTR_5,INTR_5_MASK,INTR_5_ENABLE,INTR_5_DISABLE,INTR_5_CLEAR},
  {INTR_6,INTR_6_MASK,INTR_6_ENABLE,INTR_6_DISABLE,INTR_6_CLEAR},
  {INTR_7,INTR_7_MASK,INTR_7_ENABLE,INTR_7_DISABLE,INTR_7_CLEAR},
  {INTR_8,INTR_8_MASK,INTR_8_ENABLE,INTR_8_DISABLE,INTR_8_CLEAR},
  {INTR_9,INTR_9_MASK,INTR_9_ENABLE,INTR_9_DISABLE,INTR_9_CLEAR},
  {INTR_A,INTR_A_MASK,INTR_A_ENABLE,INTR_A_DISABLE,INTR_A_CLEAR},
  {INTR_B,INTR_B_MASK,INTR_B_ENABLE,INTR_B_DISABLE,INTR_B_CLEAR},
  {INTR_C,INTR_C_MASK,INTR_C_ENABLE,INTR_C_DISABLE,INTR_C_CLEAR},
  {INTR_D,INTR_D_MASK,INTR_D_ENABLE,INTR_D_DISABLE,INTR_D_CLEAR},
  {INTR_E,INTR_E_MASK,INTR_E_ENABLE,INTR_E_DISABLE,INTR_E_CLEAR},
  {INTR_F,INTR_F_MASK,INTR_F_ENABLE,INTR_F_DISABLE,INTR_F_CLEAR}  
};

/*
** ++
**
** This function performs the one time initialization of
** GIC interrupt lines.
**
** --
*/

SAS_Status IntrInit(void)
  {
  int i;  
  Gic gic = LookupGic();
  
  if(!gic) return SAS_NO_GIC;
  
  /* configure all GIC interrupt lines */
  /* SEMTODO: interrupts should be core specific */
  for(i=0; i<IRQ_COUNT; i++)
    {
    /* clear any pending GIC interrupts */
    uint32_t *cfg = (uint32_t *)(gic + GicCtl[i][INTR_PEND_IDX]);
    *cfg = GicCtl[i][INTR_MASK_IDX];

    /* enable the GIC interrupt line */
    cfg = (uint32_t *)(gic + GicCtl[i][INTR_ENABLE_IDX]);
    *cfg = GicCtl[i][INTR_MASK_IDX];
    }
  
  TeardownGic(gic);
  return SAS_SUCCESS;
  }
