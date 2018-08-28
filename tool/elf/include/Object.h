// -*-Mode: C;-*-
/**
@file
@brief Declare and implement class Object.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#ifndef TOOL_ELF_OBJECT_H
#define TOOL_ELF_OBJECT_H

#include <stdio.h> /* For NULL.*/
#include <string.h>



#include "elf/baseTypes.h"
#include "elf/GnuHasher.h"
#include "elf/SysvHasher.h"

#define swp(a) a

/** @brief A descriptor for an ELF dynamic shared object.

    Given the memory address M of an ELF object:
    - The ELF header starts at M, which is page-aligned.
    - The program header table starts at M plus the value of e_phoff.
    - M is the load address of one of the loadable segments S, which contains
      at least the ELF header and hence must have a file offset
      of zero and the lowest file vaddr of all the loadable segments.
      in the object.

    Given these assumptions one can convert these virtual addresses to
    memory addresses by adding (M - vaddr(S)):
    - Segment base vaddrs.
    - The value of e_entry in the ELF header.
    - The pointers to hash tables, string tables, symbol tables,
      relocation tables, etc. in the dynamic segment.

   In the special case of a DSO vaddr(S) is zero.
 */
typedef struct {
  const elf_Header*   header;
  const elf_Pheader*  phdrtab;
  int                 numPhdr;
  const elf_Dynamic*  dyntab;
  const elf_Symbol*   dynSymboltab;
  const char*         dynStringtab;
  const unsigned*     sysvHashtab;
  const unsigned*     gnuHashtab;
  char*               baseAddress;
  void (**inittab)(void);
  int                 numInits;
  const elf_Rel*      reltab;
  int                 numRels;
  const elf_Rel*      pltReltab;
  int                 numPltRels;
  const char*         exidx;
  unsigned            exidxSize;

  elf_SysvHasher      sysvHasher;
  elf_GnuHasher       gnuHasher;
} elf_Object;

/** @brief Find the parts of the objecy relevant to dynamic linking.
    @returns An elf_Status code (or the logical OR of several).
*/
static inline int elf_Object_init(elf_Object* obj, const char* headerAddr) {
  int status;
  const elf_Pheader* dynamic  = NULL; /* Addr. of Pheader for dynamic section. */
  const elf_Pheader* exidx    = NULL;
  obj->header       = (const elf_Header*)headerAddr;
  obj->phdrtab      = NULL;
  obj->numPhdr      = 0;
  obj->dyntab       = NULL;
  obj->dynSymboltab = NULL;
  obj->dynStringtab = NULL;
  obj->sysvHashtab  = NULL;
  obj->gnuHashtab   = NULL;
  obj->baseAddress  = (char*)0;
  obj->inittab      = NULL;
  obj->numInits     = 0;
  obj->reltab       = NULL;
  obj->numRels      = 0;
  obj->pltReltab    = NULL;
  obj->numPltRels   = 0;
  obj->exidx        = 0;
  obj->exidxSize    = 0;
  memset(&obj->sysvHasher, 0, sizeof obj->sysvHasher);
  memset(&obj->gnuHasher,  0, sizeof obj->gnuHasher);

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
      else if (elf_Pheader_isExidx(phdr)) {
        exidx = phdr;
      }
    }
    if (!loadable) status |= ELF_NO_LOADABLE_SEGMENTS;
    if (!dynamic)  status |= ELF_NO_DYNAMIC_SECTION;
    obj->baseAddress = (char*)obj->header - lowVaddr;
    if (exidx) {
      obj->exidx     = elf_Pheader_segAddr(exidx, obj->baseAddress);
      obj->exidxSize = elf_Pheader_memSize(exidx);
    }
  }

  /* Scan the dynamic section to find hash tables, symbol table,
     string table, init array and relocation tables.
  */
  if (status == ELF_OK) {
    const elf_Dynamic* dyn;
    const elf_Symbol*  symtab;
    const unsigned*    hashtab;
    const char*        strtab;
    const elf_Rel*     reltab;
    elf_InitFunc**     inittab;
    unsigned           n;
    obj->dyntab      = (const elf_Dynamic*)(obj->baseAddress + elf_Pheader_virtAddr(dynamic));
    for (dyn = obj->dyntab; !elf_Dynamic_isSentinel(dyn); ++dyn) {
      if ((symtab  = elf_Dynamic_symtabAddr      (dyn, obj->baseAddress))) obj->dynSymboltab = symtab;
      if ((strtab  = elf_Dynamic_stringtabAddr   (dyn, obj->baseAddress))) obj->dynStringtab = strtab;
      if ((hashtab = elf_Dynamic_sysvHashtabAddr (dyn, obj->baseAddress))) obj->sysvHashtab  = hashtab;
      if ((hashtab = elf_Dynamic_gnuHashtabAddr  (dyn, obj->baseAddress))) obj->gnuHashtab   = hashtab;
      if ((reltab  = elf_Dynamic_reltabAddr      (dyn, obj->baseAddress))) obj->reltab       = reltab;
      if ((reltab  = elf_Dynamic_pltReltabAddr   (dyn, obj->baseAddress))) obj->pltReltab    = reltab;
      if ((inittab = elf_Dynamic_inittabAddr     (dyn, obj->baseAddress))) obj->inittab      = inittab;

      if ((n = elf_Dynamic_numRels    (dyn))) obj->numRels    = n;
      if ((n = elf_Dynamic_numPltRels (dyn))) obj->numPltRels = n;
      if ((n = elf_Dynamic_numInits   (dyn))) obj->numInits   = n;
    }
    if (!obj->dynSymboltab)                    status |= ELF_NO_DYNAMIC_SYMBOLS;
    if (!obj->dynStringtab)                    status |= ELF_NO_DYNAMIC_STRINGS;
    if (!obj->sysvHashtab && !obj->gnuHashtab) status |= ELF_NO_HASH_TABLE;
  }

  if (status == ELF_OK) {
    if (obj->sysvHashtab)
      elf_SysvHasher_init (&obj->sysvHasher, obj->sysvHashtab, obj->dynSymboltab, obj->dynStringtab);
    if (obj->gnuHashtab)
      elf_GnuHasher_init  (&obj->gnuHasher,  obj->gnuHashtab,  obj->dynSymboltab, obj->dynStringtab);
  }
  return status;
}

/* @brief Make a 32-bit hash from a string. The GNU hash function is
   used if a GNU hash table is present, otherwise the System V hash
   function is used.
*/
static inline unsigned elf_Object_hash(const elf_Object* obj, const char* name) {
  return obj->gnuHashtab ? elf_GnuHasher_hash(name) : elf_SysvHasher_hash(name);
}

/** @brief Find the elf_Symbol that matches the given name and hash
   value (NULL if no match or a match to an undefined ref).
*/
static inline const elf_Symbol* elf_Object_lookup(const elf_Object* obj,
                                                  unsigned hashValue,
                                                  const char* name) {
  return obj->gnuHashtab
    ? elf_GnuHasher_lookup  (&obj->gnuHasher,  hashValue, name)
    : elf_SysvHasher_lookup (&obj->sysvHasher, hashValue, name);
}

/** @brief Return the raw value for an absolute symbol, or a true memory address
    for non-absolute symbols.
*/
static inline uintptr_t elf_Object_getSymbolValue(const elf_Object* obj, const elf_Symbol* sym) {
  const uintptr_t raw = elf_Symbol_value(sym);
  return elf_Symbol_isAbsolute(sym) ? raw : raw + (uintptr_t)obj->baseAddress;
}

#undef swp
#endif
