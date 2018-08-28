        .FILE   "memTest.s"

                /* Global Constant Definitions */

#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)
#include DAT_PUBLIC(bootstrap, common, params.h)
#include DAT_PUBLIC(bootstrap, common, dcr.h)

                /* Global Entry Point Definitions */

        .SECTION        ".text","ax"

        .Global Boot0
        .GLOBAL INBGIN

                /* External Routine Definitions */

        .EJECT

                /* INBGIN - Task initialization point */

Boot0:
INBGIN: LI      R30,0X1         /* Initialize step count */
        MTDCR   LEDREG,R30      /* Leds <--- 1 */
        ADDI    R30,R30,1       /* Increment step count */
        LI      R31,0           /* Initialize zero register */
/*      LIS     R0,(1<<26|1<<27|1<<28|1<<29|1<<30|1<<31)@H */
                                        /* Set mask high */
/*      LIS     R0,0 */
/*      ORI     R0,R0,1<<10|1<<11 */
                                        /* Add mask low */
/*      MTCCR0  R0 */           /* Initialize core configuration register */
        ICCCI   R0,R0           /* Invalidate instruction cache */
        LIS     R3,EXTADDR@H    /* Set RLDRAM address */
        LI      R4,0            /* Clear memory offset */
        LI      R0,0X100        /* Set data cache congruence class count */
        MTCTR   R0              /* Transfer data cache congruence class count to CTR */
INDCAC: DCCCI   R3,R4           /* Invalidate a data cache congruence class */
        ADDI    R4,R4,0X20      /* Update memory offset */
        BDNZ    INDCAC          /* Loop over data cache congruence class */
/*      MTSGR   R31 */          /* Initialize storage guarded register */
        LIS     R0,(1<<31)@H    /* Set mask */
        MTICCR  R0              /* Initialize instruction cache cacheability register */
        MTDCCR  R31             /* Disable data cache cacheability */
        MTDCR   LEDREG,R30      /* Leds <--- 2 */
        ADDI    R30,R30,1       /* Increment step count */

        .EJECT

                /* Memory test parameters */

        LIS     R7,EXTEND@H     /* Top of memory */
        LI      R4,EXTSIZE      /* Test block length */
        SRWI    R4,R4,2         /* Convert length to longword count */
        SLWI    R6,R4,2         /* Block increment, in bytes */
        MTDCR   MEMERST,R31     /* Clear the memory status register */
        LI      R28,0           /* Keep a count of how many times we loop */

        LI      R26,0           /* Set up a mask to look at the double bit errors */
        LIS     R26,0x00FF@H    /* 0x00ff0000 */

                /* Test RLDRAM with zeroes */

INALL:  LI      R27,1           /* To see where we are */
        LIS     R3,EXTADDR@H    /* Set RLDRAM base address */
INBLK0: LA      R5,-4(R3)       /* Offset starting address */
        MTCTR   R4              /* Transfer word count to CTR */
INLD0:  STWU    R31,4(R5)       /* Clear next word */
        BDNZ    INLD0           /* Loop over words */
        LA      R5,-4(R3)       /* Offset starting address */
        MTCTR   R4              /* Transfer word count to CTR */
        MTDCR   MEMERST,R31     /* Clear the memory status register */
INCK0:  LWZU    R0,4(R5)        /* Get next word */
        MR.     R0,R0           /* Check next word */
        BNE     INERR           /* Branch if word non-zero */
        MFDCR   R8,MEMERST      /* Get RLDRAM read error status */
        AND.    R8,R8,R26       /* Set up condition codes */
        BNE     INERR           /* Branch if result is non-zero */
        BDNZ    INCK0           /* Loop over words */
        ADD     R3,R3,R6        /* Increment for the next block */
        CMPW    CR0,R3,R7       /* Are we at the end? */
        BLT+    INBLK0          /* Branch to next block */
        MTDCR   LEDREG,R30      /* Leds <--- 6 */
        ADDI    R30,R30,1       /* Increment step count */

        .EJECT

                /* Test RLDRAM with ones */

        LI      R27,2           /* To see where we are */
        LIS     R3,EXTADDR@H    /* Set RLDRAM base address */
INBLK1: LA      R5,-4(R3)       /* Offset starting address */
        MTCTR   R4              /* Transfer word count to CTR */
        LI      R0,0XFFFFFFFF   /* Set ones */
INLD1:  STWU    R0,4(R5)        /* Load next word */
        BDNZ    INLD1           /* Loop over words */
        LA      R5,-4(R3)       /* Offset starting address */
        MTCTR   R4              /* Transfer word count to CTR */
        MTDCR   MEMERST,R31     /* Clear the memory status register */
INCK1:  LWZU    R0,4(R5)        /* Get next word */
        NOT.    R0,R0           /* Check next word */
        BNE     INERR           /* Branch if word not all ones */
        BDNZ    INCK1           /* Loop over words */
        MFDCR   R8,MEMERST      /* Get RLDRAM read error status */
        AND.    R8,R8,R26       /* Set up condition codes */
        BNE     INERR           /* Branch if result is non-zero */
        ADD     R3,R3,R6        /* Increment for the next block */
        CMPW    CR0,R3,R7       /* Are we at the end? */
        BLT+    INBLK1          /* Branch to next block */
        MTDCR   LEDREG,R30      /* Leds <--- 7 */
        ADDI    R30,R30,1       /* Increment step count */

        .EJECT

                /* Test RLDRAM with addresses */

        LI      R27,3           /* To see where we are */
        LIS     R3,EXTADDR@H    /* Set RLDRAM base address */
INBLKA: LA      R5,-4(R3)       /* Offset starting address */
        MTCTR   R4              /* Transfer word count to CTR */
INLDA:  ADDI    R5,R5,4         /* Prepare address to store */
        STW     R5,0(R5)        /* Store address at address */
        BDNZ    INLDA           /* Loop over words */
        LA      R5,-4(R3)       /* Offset starting address */
        MTCTR   R4              /* Transfer word count to CTR */
        MTDCR   MEMERST,R31     /* Clear the memory status register */
INCKA:  LWZU    R0,4(R5)        /* Get next word */
        CMPW    CR0,R0,R5       /* Check next word */
        BNE     INERR           /* Branch if no match */
        BDNZ    INCKA           /* Loop over words */
        MFDCR   R8,MEMERST      /* Get RLDRAM read error status */
        AND.    R8,R8,R26       /* Set up condition codes */
        BNE     INERR           /* Branch if result is non-zero */
        ADD     R3,R3,R6        /* Increment for the next block */
        CMPW    CR0,R3,R7       /* Are we at the end? */
        BLT+    INBLKA          /* Branch to next block */

                /* Loop to repeat tests */

        ADDI    R28,R28,1       /* Count loops */
        B       INALL           /* Go again */

        .SECTION        ".text","ax"

        .EJECT

                /* Error processing */

INERR:  LIS     R29,0XDEAD      /* Set error flag */
        OR      R30,R30,R29     /* Update status with error flag */
        MTDCR   LEDREG,R30      /* Update LED */
        LIS     R29,WE@H        /* Set wait state mask */
INLOOP: MTMSR   R29             /* Disable processing */
        B       INLOOP          /* Loop forever */
