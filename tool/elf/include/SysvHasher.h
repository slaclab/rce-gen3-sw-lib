// -*-Mode: C;-*-
/**
@file
@brief Declare and implement the SysvHasher class.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#ifndef TOOL_ELF_SYSVHASHER_H
#define TOOL_ELF_SYSVHASHER_H

#include <stdio.h> /* For NULL. */


#include "elf/baseTypes.h"

#define swp(a) a

/** @class tool::elf::SysvHasher
    @brief Make use of the dynamic symbol table and its hash table (System V format, not GNU).

    The hash table is organized as a set of buckets each of which is a
    chain of symbol indexes. The first index for bucket i is
    bucket[i]. The rest of the chain starts at chain[bucket[i]]. The
    next chain entry after chain[i] is chain[chain[i]], unless
    chain[i] is the end-marker value ST_UNDEF.

    We scan the chosen bucket's chain until we find the index of a
    symbol which has the name we're looking for or until we encounter
    ST_UNDEF.

    The bucket chosen for a symbol is the symbol name hash modulo the
    number of buckets.
*/
typedef struct {
  unsigned           nbucket;  // Number of buckets.
  int                nchain;   // Number of chains.
  const unsigned*    bucket;   // Pointer to bucket[0].
  const unsigned*    chain;    // Pointer to chain[0].
  unsigned           nsym;     // Number of symbols (1 + max(hashtable symbol index))
  const char*        stringtabAddr;  // Pointer to the string table used by the symbols.
  const elf_Symbol*  symtabAddr;  // Pointer to symbol[0].
} elf_SysvHasher;

// Return the maximum symbol table index used in the hash table.
static inline unsigned elf_SysvHasher_maxSymIndex(const elf_SysvHasher* hsh) {
  const unsigned* indexes  = hsh->bucket;
  unsigned  nindexes       = hsh->nbucket + hsh->nchain;
  unsigned m = 0;
  int i;
  for (i = 0; i < nindexes; ++i) if (swp(indexes[i]) > m) m = swp(indexes[i]);
  return m;
}

/** @brief Initialize a SysvHasher. */
static inline void elf_SysvHasher_init(elf_SysvHasher*    hsh,
                                       const unsigned*    hashtabAddr,
                                       const elf_Symbol*  symtabAddr,
                                       const char*        stringtabAddr)
{
  hsh->nbucket = swp(hashtabAddr[0]);
  hsh->nchain  = swp(hashtabAddr[1]);
  hsh->bucket  = hashtabAddr + 2;
  hsh->chain   = hsh->bucket + hsh->nbucket;
  hsh->nsym    = 1 + elf_SysvHasher_maxSymIndex(hsh);
  hsh->stringtabAddr = stringtabAddr;
  hsh->symtabAddr = symtabAddr;
}

static inline unsigned elf_SysvHasher_hash(const char *symbol) {
  // From the ELF gABI 4.1.
  unsigned h = 0;
  unsigned g;
  while (*symbol) {
    h = (h << 4) + (unsigned char)(*symbol++);
    if ((g = h & 0xf0000000)) {
      h ^= g >> 24;
    }
    h &= ~g;
  }
  return h;
}

/** @brief Return a pointer to the ELF symbol that matches the given
    name (NULL if no match or a match to an undefined reference).

    @note We take the name's hash value as an argument so that we can
    search multiple symbol tables without rehashing each time.
*/
static inline const elf_Symbol* elf_SysvHasher_lookup(const elf_SysvHasher* hsh,
						      unsigned hashValue,
						      const char* name)
{
  unsigned i = hashValue % hsh->nbucket;
  int isym;
  for (isym = swp(hsh->bucket[i]); isym != STN_UNDEF; isym = swp(hsh->chain[isym])) {
    const elf_Symbol* const sym = hsh->symtabAddr + isym;
    if (elf_Symbol_nameMatches(sym, name, hsh->stringtabAddr) && elf_Symbol_isDefinition(sym)) return sym;
  }
  return NULL;
}
#undef swp
#endif
