// -*-Mode: C;-*-
/**
@file
@brief Declare and implement the GnuHasher class.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#ifndef TOOL_ELF_GNUHASHER_H
#define TOOL_ELF_GNUHASHER_H

#include <stdio.h> /* For NULL. */


#include "elf/baseTypes.h"

#define swp(a) a

/** @class tool::elf::GnuHasher

    @brief Make use of the dynamic symbol table and its hash table
    (GNU format, not System V).

    A Bloom filter with two hash functions is used to check whether a
    symbol is in the hash table. The second hash value is gotten by
    shifting right a full 32-bit unsigned hash value by a number of
    bits determined by the static linker (shift2).

    The hash table proper is organized as a set of buckets each of
    which comprises two arrays SA and HA. SA contains elf_Symbol
    values for the bucket; the static linker has sorted the
    ELF symbol table from symndx upwards so that each bucket's
    symbols are consecutive in memory. The upper 31 bits of each
    entry HA[i] are from the hash value for symbol SA[i].
    The low-order bit is zero except for the last
    hash value in the bucket. The HA arrays are stored in
    the hash table.

    The bucket chosen for a symbol is the symbol name hash modulo the
    number of buckets.

    The lookup procedure in essence:
    - Check the Bloom filter to determine if the symbol is
      absent from the hash table. If it is then return NULL.
    - Find the bucket arrays SA and HA.
    - Attempt to find an index imatch such that
      the upper 31 bits of the hash value and HA[imatch]
      are equal. If none then return NULL.
    - Perform a string comparision of SA[imatch]
      and the symbol we're looking for. If they
      compare as equal then return &SA[imatch]
      else return NULL.

   For full details see https://blogs.oracle.com/ali/entry/gnu_hash_elf_sections.

*/
typedef struct {
  unsigned           nbuckets;  // Number of buckets.
  unsigned           symndx;    // Index of first symbol in hash table.
  unsigned           maskwords; // Number of 32-bit words in Bloom filter.
  unsigned           shift2;    // A Bloom filter parameter.
  const unsigned*    bloom;     // Pointer to filter word array. 
  const unsigned*    buckets;   // Pointer to array of bucket start-indexes.
  const unsigned*    hashes;    // Pointer to first hash-value array.
  const char*        stringtabAddr;  // Pointer to the string table used by the symbols.
  const elf_Symbol*  symtabAddr;   // Pointer to dynamic symbol table.
} elf_GnuHasher;

/** @brief Initialize a GnuHasher. */
static inline void elf_GnuHasher_init(elf_GnuHasher* hsh,
				      const unsigned*    hashtabAddr,
				      const elf_Symbol*  symtabAddr,
				      const char*        stringtabAddr)
{
  hsh->nbuckets  = swp(hashtabAddr[0]);
  hsh->symndx    = swp(hashtabAddr[1]);
  hsh->maskwords = swp(hashtabAddr[2]);
  hsh->shift2    = swp(hashtabAddr[3]);
  
  hsh->bloom     = hashtabAddr  + 4;
  hsh->buckets   = hsh->bloom   + hsh->maskwords;
  hsh->hashes    = hsh->buckets + hsh->nbuckets;

  hsh->stringtabAddr = stringtabAddr;
  hsh->symtabAddr    = symtabAddr;
}

static inline unsigned elf_GnuHasher_hash(const char *name) {
  uint_fast32_t h = 5381;
  unsigned char c;
  for (c = *name; c != '\0'; c = *++name)
    h = h * 33 + c;
  return h & 0xffffffff;
}

/** @brief Return a pointer to the ELF symbol that matches the given
    name (NULL if no match or a match to an undefined reference).

    @note We take the name's hash value as an argument so that we can
    search multiple symbol tables without rehashing each time.
*/
static inline const elf_Symbol* elf_GnuHasher_lookup(const elf_GnuHasher* hsh,
						     const unsigned       hashValue,
						     const char*          name)
{
  // Apply the Bloom filter.
  {
    unsigned const h1      = hashValue;
    unsigned const h2      = h1 >> hsh->shift2;
    unsigned const n       = (h1 >> 5) & (hsh->maskwords - 1U);
    unsigned const bitmask = (1U << (h1 & 0x1f)) | (1U << (h2 & 0x1f));
    if ((hsh->bloom[n] & bitmask) != bitmask) return NULL;
  }

  // Find the bucket and check its hash values.
  {
    unsigned const n = hashValue % hsh->nbuckets;
    // Empty bucket?
    if (0 == hsh->buckets[n]) return NULL;
    { // No.
      unsigned          const h       = hashValue & (~1U);
      const elf_Symbol*       symbol  = hsh->symtabAddr + hsh->buckets[n];
      const unsigned*         hash    = hsh->hashes     + hsh->buckets[n] - hsh->symndx;
      for (;;) {
	if ( (h == (*hash & (~1U)))
	     && elf_Symbol_nameMatches(symbol, name, hsh->stringtabAddr)
	     && elf_Symbol_isDefinition(symbol)
	     )
	  return symbol;
	if (*hash++ & 1U) break;
	++symbol;
      }
    }
  }
  return NULL;
}
#undef swp
#endif
