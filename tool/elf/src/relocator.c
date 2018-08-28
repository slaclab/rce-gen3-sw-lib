// -*-Mode: C;-*-
/**
@file
@brief Implement a relocator for RTEMS startup code, shared-object RTEMS.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2013/12/14

@par Last commit:
\$Date: 2015-01-12 10:57:01 -0800 (Mon, 12 Jan 2015) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3929 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/src/relocator.c $

@par Credits:
SLAC

*/
#include <stdarg.h>
#include <stdio.h> /* For NULL. */


/* Here we have stripped-down ELF object scanning and relocation
   processing. See Object.h and linker.c for the full code with
   comments. This file must be self-contained since it can't
   call external non-inline functions; they're not accessible until
   the relocation is done. Local functions can be found
   using offsets which require no relocation.
*/
#include "elf/baseTypes.h"
#include "elf/lnkStatus.h"
#include "memory/mem.h"
#include "memory/cpu/memRegionConstants.h"

extern void _start(void);

typedef struct {
  const elf_Header*   header;
  const elf_Pheader*  phdrtab;
  int                 numPhdr;
  const elf_Dynamic*  dyntab;
  const elf_Symbol*   dynSymboltab;
  char*               baseAddress;
  const elf_Rel*      reltab;
  int                 numRels;
  const elf_Rel*      pltReltab;
  int                 numPltRels;
} elf_Object;

static inline int elf_Object_init(elf_Object* obj, const char* headerAddr) {
  int status;
  const elf_Pheader* dynamic  = NULL; /* Addr. of Pheader for dynamic section. */
  obj->header       = (const elf_Header*)headerAddr;
  obj->phdrtab      = NULL;
  obj->numPhdr      = 0;
  obj->dyntab       = NULL;
  obj->dynSymboltab = NULL;
  obj->baseAddress  = (char*)0;
  obj->reltab       = NULL;
  obj->numRels      = 0;
  obj->pltReltab    = NULL;
  obj->numPltRels   = 0;

  /* Header OK? */
  status = elf_Header_check(obj->header);

  /* Scan the program header table for loadable segments and the dynamic section. */ 
  if (status == ELF_OK) {
    int                loadable = 0;             /* Any segments loadable? */
    unsigned           lowVaddr = 0xffffffff;    /* Lowest vaddr of a loadable seg. */
    int i;
    obj->numPhdr = elf_Header_numPhdr(obj->header);
    obj->phdrtab = elf_Header_addrPhdr(obj->header);
    for (i = 0; i < obj->numPhdr; ++i) {
      const elf_Pheader* phdr  = obj->phdrtab + i;
      if (elf_Pheader_isLoadable(phdr)) {
        const unsigned vaddr = elf_Pheader_virtAddr(phdr);
        loadable = 1;
        lowVaddr = vaddr < lowVaddr ? vaddr : lowVaddr;
      }
      else if (elf_Pheader_isDynamic(phdr)) {
        dynamic = phdr;
      }
    }
    if (!loadable) status |= ELF_NO_LOADABLE_SEGMENTS;
    if (!dynamic)  status |= ELF_NO_DYNAMIC_SECTION;
    obj->baseAddress = (char*)obj->header - lowVaddr;
  }

  /* Scan the dynamic section. */
  if (status == ELF_OK) {
    const elf_Dynamic* dyn;
    const elf_Symbol*  symtab;
    const elf_Rel*     reltab;
    unsigned           n;
    obj->dyntab      = (const elf_Dynamic*)(obj->baseAddress + elf_Pheader_virtAddr(dynamic));
    for (dyn = obj->dyntab; !elf_Dynamic_isSentinel(dyn); ++dyn) {
      if ((symtab  = elf_Dynamic_symtabAddr      (dyn, obj->baseAddress))) obj->dynSymboltab = symtab;
      if ((reltab  = elf_Dynamic_reltabAddr      (dyn, obj->baseAddress))) obj->reltab       = reltab;
      if ((reltab  = elf_Dynamic_pltReltabAddr   (dyn, obj->baseAddress))) obj->pltReltab    = reltab;

      if ((n = elf_Dynamic_numRels    (dyn))) obj->numRels    = n;
      if ((n = elf_Dynamic_numPltRels (dyn))) obj->numPltRels = n;
    }
    if (!obj->dynSymboltab)                    status |= ELF_NO_DYNAMIC_SYMBOLS;
  }
  return status;
}

static inline uintptr_t elf_Object_getSymbolValue(const elf_Object* obj, const elf_Symbol* sym) {
  const uintptr_t raw = elf_Symbol_value(sym);
  return elf_Symbol_isAbsolute(sym) ? raw : raw + (uintptr_t)obj->baseAddress;
}

static int relocate(elf_Object* elf, const elf_Rel* reltab, unsigned numRels);

static int modifyObject
(int       const rtype,
 unsigned* const where,
 unsigned        symbolValue,
 unsigned        thumb,
 unsigned        baseaddr);

int lnk_relocate() {
  /* Perform relocations on the RTEMS shared object loaded at the start of the RTS Region. */
  const char* const headerAddress = (const char *)RTS_START;
  int status;
  elf_Object elf;

  status = elf_Object_init(&elf, headerAddress);
  if (status) return status;

  status = relocate(&elf, elf.reltab, elf.numRels);
  if (status) return status;
  
  status = relocate(&elf, elf.pltReltab, elf.numPltRels);
  if (status) return status;
  
  _start();
  return status;
}

static inline void splitInterwork(unsigned origValue, unsigned symType, unsigned* symValue, unsigned* thumb) {
  if (symType == STT_FUNC) {
    *symValue = origValue & ~1U;
    *thumb    = origValue &  1U;
  }
  else {
    *symValue = origValue;
    *thumb    = 0;
  }
}


int relocate(elf_Object* obj, const elf_Rel* relArray, unsigned numRels)
{
  unsigned      i;
  int status = LNK_OK;

  for (i = 0; i < numRels && !status; ++i) {
    const elf_Rel*    const rel      = relArray + i;
    unsigned          const type     = elf_Rel_type(rel);
    unsigned          const symIndex = elf_Rel_symbolIndex(rel);
    unsigned*         const where    = (unsigned*)elf_Rel_target(rel, obj->baseAddress);
    const elf_Symbol* const symbol    = obj->dynSymboltab + symIndex;
    unsigned                symval;
    unsigned                thumb;

    if (symIndex == 0) {
      /* This relocation doesn't refer to any kind of symbol. */
      status = modifyObject(type, where, 0, 0, (unsigned)obj->baseAddress);
    }
    else if (elf_Symbol_isDefinition(symbol)) {
      /* This relocation refers to a locally defined symbol. */
      splitInterwork
        (elf_Object_getSymbolValue(obj, symbol), elf_Symbol_type(symbol), &symval, &thumb);
      status = modifyObject(type, where, symval, thumb, (unsigned)obj->baseAddress);
    }
    else if (elf_Symbol_isWeak(symbol)) {
      /* This relocation is for an undefined weak reference, treat as zero. */
      status = modifyObject(type, where, 0, 0, (unsigned)obj->baseAddress);
    }
    else {
      /* This relocation is for an undefined strong reference. */
      status = LNK_UNDEFINED_SYMBOLS;
    }
  } /* Loop over relocations. */
  return status;
}

int modifyObject
(int       const rtype,
 unsigned* const where,
 unsigned        symbolValue,
 unsigned        thumb,
 unsigned        baseaddr)
{
  int status = LNK_OK;
  switch(rtype) {
  case R_ARM_JUMP_SLOT:
    *where = symbolValue | thumb;
    break;
  case R_ARM_GLOB_DAT:
  case R_ARM_ABS32:
    *where = (*where + symbolValue) | thumb;
    break;
  case R_ARM_RELATIVE:
    *where = *where + baseaddr;
    break;
  case R_ARM_REL32:
    *where =  (*where + symbolValue - (unsigned)where) | thumb;
    break;
  default:
    status = LNK_BAD_RELOCATION_TYPE;
  }
 return status;
}
