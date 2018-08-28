// -*-Mode: C;-*-
/**
@cond development
@file Hash.c
@brief Implement the HASH algorithms
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include "hash/Hash.h"

/*
** ++
** 
** lookup3.c, by Bob Jenkins, May 2006, Public Domain.
**
** --
*/

#define hashsize(n) ((uint32_t)1<<(n))
#define hashmask(n) (hashsize(n)-1)
#define rot(x,k)    (((x)<<(k)) | ((x)>>(32-(k))))

/*
** ++
**
** mix -- mix 3 32-bit values reversibly.
**
** This is reversible, so any information in (a,b,c) before mix() is still in (a,b,c) 
** after mix().
**
** If four pairs of (a,b,c) inputs are run through mix(),or through  mix() in reverse,
** there are at least 32 bits of the output that are sometimes the same for one pair 
** and different for another pair. This was tested for:
** - pairs that differed by one bit, by two bits, in any combination of top bits of
**   (a,b,c), or in any combination of bottom bits of (a,b,c).
** - "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed the output 
**   delta to a Gray code (a^(a>>1)) so a string of 1's (as is commonly produced by 
**   subtraction) look like a single 1-bit difference.
** - the base values were pseudorandom, all zero but one bit set,or all zero plus a 
**   counter that starts at zero.
**
** Some k values for my "a-=c; a^=rot(c,k); c+=b;" arrangement that satisfy this are
**    4  6  8 16 19  4
**    9 15  3 18 27 15
**   14  9  3  7 17  3
** Well, "9 15 3 18 27 15" didn't quite get 32 bits diffing for "differ" defined as
** - with a one-bit base and a two-bit delta.I used
**   http://burtleburtle.net/bob/hash/avalanche.html to choose the operations, 
**   constants, and arrangements of the variables.
**
** This does not achieve avalanche.  There are input bits of (a,b,c) that fail to 
** affect some output bits of (a,b,c), especially of a.The most thoroughly mixed 
** value is c, but it doesn't really even achieve avalanche in c.
**
** This allows some parallelism.  Read-after-writes are good at doubling the number of
** bits affected, so the goal of mixing pulls in the opposite direction as the goal of
** parallelism.  I did what I could.  Rotates seem to cost as much as shifts on every 
** machine I could lay my hands on, and rotates are much kinder to the top and bottom
** bits, so I used rotates.
**
** --
*/

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

/*
** ++
**
** final -- final mixing of 3 32-bit values (a,b,c) into c
**
** Pairs of (a,b,c) values differing in only a few bits will usually produce values of
** c that look totally different.  This was tested for:
** - pairs that differed by one bit, by two bits, in any combination of top bits of 
**   (a,b,c), or in any combination of bottom bits of (a,b,c).
** - "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed the output 
**   delta to a Gray code (a^(a>>1)) so a string of 1's (as is commonly produced by 
**   subtraction) look like a single 1-bit difference.
** - the base values were pseudorandom, all zero but one bit set,or all zero plus a 
**   counter that starts at zero.
**
** These constants passed:
**  14 11 25 16 4 14 24
**  12 14 25 16 4 14 24
** and these came close:
**  4  8 15 26 3 22 24
**  10  8 15 26 3 22 24
** 11  8 15 26 3 22 24
**
** --
*/

#define cleanup(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}




#define MAGIC_NUMBER ((uint32_t)0xDEADBEEF)
#define REGISTERS_USED 3
 
uint64_t HashWord(const uint32_t* vector, uint32_t length, uint32_t seed)   
{

int32_t   remaining = length;
uint32_t  register2 = seed + MAGIC_NUMBER + (remaining << 2);
uint32_t  register1 = register2;
uint32_t  register0 = register1;     
                         
const uint32_t* next =  vector;

while(remaining > REGISTERS_USED)
  {
  register0 += next[0]; 
  register1 += next[1]; 
  register2 += next[2];
  mix(register0, register1, register2);
  next      += REGISTERS_USED;
  remaining -= REGISTERS_USED;
  } 

switch(remaining)
  {
  case 3 : register2 += next[2];
  case 2 : register1 += next[1];
  case 1 : register0 += next[0];
  }

cleanup(register0, register1, register2);

return (uint64_t)register2 | ((uint64_t)register1 << 32);
}


#define FOREVER -1

#define CONTINUE  ((uint32_t)0xFFFFFFFF)
 
static inline uint32_t _parse(uint32_t word);
 
uint64_t HashString(const char* vector, uint32_t seed)   
{

uint32_t register0 = seed + MAGIC_NUMBER;
uint32_t register1 = register0;
uint32_t register2 = register1;     
                        
const uint32_t* next = (uint32_t*)vector;

while(FOREVER)
  {  
  
  uint32_t next_word = *next++; uint32_t next_mask = _parse(next_word);
  
  register0 += next_mask & next_word; if(next_mask != CONTINUE) break;

  next_word = *next++; next_mask = _parse(next_word);
  
  register1 += next_mask & next_word; if(next_mask != CONTINUE) break;

  next_word = *next++; next_mask = _parse(next_word);
  
  register2 += next_mask & next_word; if(next_mask != CONTINUE) break;
  
  mix(register0, register1, register2);
  } 

cleanup(register0, register1, register2);

return (uint64_t)register2 | ((uint64_t)register1 << 32);
}


 
#define NONE  ((uint32_t)0x00000000)
#define ONE   ((uint32_t)0x000000FF)
#define TWO   ((uint32_t)0x0000FFFF)
#define THREE ((uint32_t)0x00FFFFFF)

static inline uint32_t _parse(uint32_t word)
 {
 if(!(word & 0x000000FF)) return NONE;
 if(!(word & 0x0000FF00)) return ONE;
 if(!(word & 0x00FF0000)) return TWO;
 if(!(word & 0xFF000000)) return THREE;

 return CONTINUE;
 }

/*
** ++
**
**
** --
*/
 
uint64_t Hash64_32(uint64_t parent, uint32_t child)   
{

uint32_t  register2    = MAGIC_NUMBER + (3 << 2);
uint32_t  register1    = register2;
uint32_t  register0    = register1;     

register2 += (uint32_t)parent;
register1 += (uint32_t)(parent >> 32);
register0 += child;

cleanup(register0, register1, register2);

return (uint64_t)register2 | ((uint64_t)register1 << 32);
}

/** @endcond */

/* Documentation */

/** @def HASH_SEED
    @brief Useful seed value for the hash functions in this package.
 */

/** @fn uint64_t HashWord(const uint32_t* vector, uint32_t length, uint32_t seed)
    @brief Hash the contents of a 32-bit WORD vector to a single 64-bit value.
    @param vector A pointer to the vector of 32-bit words to be hashed.
    @param length The length of the vector in units of words.
    @param seed The seed value for the hash.  A good value is HASH_SEED.
    @return The 64-bit hash value.

    @note While any value for seed will most likely suffice, a unique value
    with a well randomized set of bits is best.  HASH_SEED is a @#define
    provided as a good candidate.
*/

/** @fn uint64_t HashString(const char* vector, uint32_t seed)
    @brief Hash the contents of a string to a single, unique, 64-bit value.
    @param vector A pointer to a NULL terminated const char* string.
    @param seed The seed value for the hash.  A good value is HASH_SEED.
    @return The 64-bit hash value.

    @note While any value for seed will most likely suffice, a unique value
    with a well randomized set of bits is best.  HASH_SEED is a @#define
    provided as a good candidate.
*/


/** @fn uint64_t Hash64_32(uint64_t parent, uint32_t child);  
    @brief Hash the contents of a 64 and 32-bit value to form a single,
    unique, 64-bit value.
    @param parent The 64-bit value to be hashed.
    @param child The 32 bit value to be hashed.
    @return The 64-bit hash value.
 */
