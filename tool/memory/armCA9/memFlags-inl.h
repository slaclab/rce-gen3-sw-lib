// -*-Mode: C;-*-
/**
@file
@brief Implement MMU-related operations for Cortex-A9 (ARMv7-A).
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


/** All code here is PIC so that it may be used in shared objects. */

static inline uint32_t  mem_getPhysRead(uint32_t virt) {
  register uint32_t parValue; /* Value read from the PAR. */
  register uint32_t physAddr; /* Physical address result. */
  /* The ATS1CPR operation causes an update of the Physical Address
     Register in CP15 as a side effect, a so-called "indirect
     write". According to the ARMv7-A Architecture manual section
     B3.15.5 such indirect writes require a context sync before the
     result is guaranteed to be visible.
  */
  asm volatile
    (
     "mcr   p15,0,%[virt],c7,c8,0  \n\t"   /* ATS1CPR */
     "mov %[phys],%[virt],lsl#20   \n\t"   /* Save low 12 bits. */
     "isb                          \n\t"   /* Wait for result */
     "mrc   p15,0,%[par],c7,c4,0   \n\t"   /* Read PAR */
     "tst   %[par],#1              \n\t"   /* Check failure bit. */
     "orreq %[phys],%[par],lsr#12  \n\t"   /* Get physical address bits.*/
     "moveq %[phys],%[phys],ror#20 \n\t"   /* Put bits in the right places.*/
     "movne %[phys],#0             \n\t"   /* Return 0 on failure. */
     : [phys] "=r"(physAddr),
       [par]  "=&r"(parValue)
     : [virt] "r"  (virt)
     : "cc"
     );
  return physAddr;
}


static inline uint32_t  mem_getPhysWrite(uint32_t virt) {
  register uint32_t parValue; /* Value read from the PAR. */
  register uint32_t physAddr; /* Physical address result. */
  asm volatile
    (
     "mcr   p15,0,%[virt],c7,c8,1  \n\t"   /* ATS1CPW */
     "mov %[phys],%[virt],lsl#20   \n\t"   /* Save low 12 bits. */
     "isb                          \n\t"   /* Wait for result */
     "mrc   p15,0,%[par],c7,c4,0   \n\t"   /* Read PAR */
     "tst   %[par],#1              \n\t"   /* Check failure bit. */
     "orreq %[phys],%[par],lsr#12  \n\t"   /* Get physical address bits.*/
     "moveq %[phys],%[phys],ror#20 \n\t"   /* Put bits in the right places.*/
     "movne %[phys],#0             \n\t"   /* Return 0 on failure. */
     : [phys] "=r" (physAddr),
       [par]  "=&r"(parValue)
     : [virt] "r"  (virt)
     : "cc"
     );
  return physAddr;
}


extern int mem_flagsLookupRead[];

static inline int  mem_getFlagsRead(uint32_t virt) {
  register uint32_t parValue;  /* Value read from the PAR. */
  register int      flagSet;
  /* We use only the low byte of the PAR. We assume that on a
     successful translation the lowest two bits are zero; we don't use
     supersections and the translation didn't fail.  In that case we
     can use the byte as the offset into a table of four-byte flag
     sets.
  */
  asm volatile
    (
     "mcr   p15,0,%[virt],c7,c8,0  \n\t"   /* ATS1CPR */
     "isb                          \n\t"   /* Wait for result */
     "mrc   p15,0,%[par],c7,c4,0   \n\t"   /* Read PAR */
     "tst   %[par],#1              \n\t"   /* Check failure bit. */
     "andeq %[flags],%[par],#0xfc  \n\t"   /* Get offset into table. */
     "ldreq %[par],1f              \n\t"   /* Get address of table. */
     "ldreq %[flags],[%[flags],%[par]]\n\t"/* Translate PAR bits to flags.*/
     "b     2f                     \n\t"
     "1:                           \n\t"
     ".word mem_flagsLookupRead    \n\t"
     "2:                           \n\t"
     "movne %[flags],#0            \n\t"  /* Return flags=0 on failure. */
     : [par]  "=&r"(parValue),
       [flags]"=r" (flagSet)
     : [virt] "r"  (virt)
     : "cc"
     );
  return flagSet;
}


extern int mem_flagsLookupWrite[];

static inline int  mem_getFlagsWrite(uint32_t virt) {
  register uint32_t parValue;  /* Value read from the PAR. */
  register int      flagSet;
  /* See mem_getFlagsRead() re the use of the lowest eight PAR bits. */
  asm volatile
    (
     "mcr   p15,0,%[virt],c7,c8,1  \n\t"   /* ATS1CPW */
     "isb                          \n\t"   /* Wait for result */
     "mrc   p15,0,%[par],c7,c4,0   \n\t"   /* Read PAR */
     "tst   %[par],#1              \n\t"   /* Check failure bit. */
     "andeq %[flags],%[par],#0xfc  \n\t"   /* Get offset into table. */
     "ldreq %[par],1f              \n\t"   /* Get address of table. */
     "ldreq %[flags],[%[flags],%[par]]\n\t"/* Translate PAR bits to flags.*/
     "b     2f                     \n\t"
     "1:                           \n\t"
     ".word mem_flagsLookupWrite   \n\t"
     "2:                           \n\t"
     "movne %[flags],#0            \n\t"  /* Return flags=0 on failure. */
     : [par]  "=&r"(parValue),
       [flags]"=r" (flagSet)
     : [virt] "r"  (virt)
     : "cc"
     );
  return flagSet;
}


static inline uint32_t mem_getPhysFlagsRead(uint32_t virt, int* flags) {
  register uint32_t parValue;  /* Value read from the PAR. */
  register uint32_t physAddr;  /* Physical address result. */
  register int      flagSet;
  /* See mem_getFlagsRead() re the use of the lowest eight PAR bits. */
  /* Note that [virt] and [phys] are allowed to be the same register. */
  asm volatile
    (
     "mcr   p15,0,%[virt],c7,c8,0  \n\t"   /* ATS1CPR */
     "mov   %[phys],%[virt],lsl#20 \n\t"   /* Save low 12 bits. */
     "isb                          \n\t"   /* Wait for result */
     "mrc   p15,0,%[par],c7,c4,0   \n\t"   /* Read PAR */
     "tst   %[par],#1              \n\t"   /* Check failure bit. */
     "orreq %[phys],%[par],lsr#12  \n\t"   /* Get physical address bits.*/
     "andeq %[flags],%[par],#0xfc  \n\t"   /* Get offset into table. */
     "moveq %[phys],%[phys],ror#20 \n\t"   /* Put address bits in the right places.*/
     "ldreq %[par],1f              \n\t"   /* Get address of table. */
     "ldreq %[flags],[%[flags],%[par]]\n\t"/* Translate PAR bits to flags.*/
     "b     2f                     \n\t"
     "1:                           \n\t"
     ".word mem_flagsLookupRead    \n\t"
     "2:                           \n\t"
     "movne %[phys],#0             \n\t"   /* Return phys=0 on failure. */
     : [phys] "=r" (physAddr),
       [par]  "=&r"(parValue),
       [flags]"=&r"(flagSet)
     : [virt] "r"  (virt)
     );
  *flags = flagSet;
  return physAddr;
}


static inline uint32_t mem_getPhysFlagsWrite(uint32_t virt, int* flags) {
  register uint32_t parValue;  /* Value read from the PAR. */
  register uint32_t physAddr;  /* Physical address result. */
  register int      flagSet;
  /* See mem_getFlagsRead() re the use of the lowest eight PAR bits. */
  /* Note that [virt] and [phys] are allowed to be the same register. */
  asm volatile
    (
     "mcr   p15,0,%[virt],c7,c8,1  \n\t"   /* ATS1CPW */
     "mov   %[phys],%[virt],lsl#20 \n\t"   /* Save low 12 bits. */
     "isb                          \n\t"   /* Wait for result */
     "mrc   p15,0,%[par],c7,c4,0   \n\t"   /* Read PAR */
     "tst   %[par],#1              \n\t"   /* Check failure bit. */
     "orreq %[phys],%[par],lsr#12  \n\t"   /* Get physical address bits.*/
     "andeq %[flags],%[par],#0xfc  \n\t"   /* Get offset into table. */
     "moveq %[phys],%[phys],ror#20 \n\t"   /* Put address bits in the right places.*/
     "ldreq %[par],1f              \n\t"   /* Get address of table. */
     "ldreq %[flags],[%[flags],%[par]]\n\t"/* Translate PAR bits to flags.*/
     "b     2f                     \n\t"
     "1:                           \n\t"
     ".word mem_flagsLookupWrite   \n\t"
     "2:                           \n\t"
     "movne %[phys],#0             \n\t"   /* Return phys=0 on failure. */
     : [phys] "=r" (physAddr),
       [par]  "=&r"(parValue),
       [flags]"=&r"(flagSet)
     : [virt] "r"  (virt)
     );
  *flags = flagSet;
  return physAddr;
}
