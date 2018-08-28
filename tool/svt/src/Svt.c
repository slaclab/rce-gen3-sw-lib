// -*-Mode: C;-*-
/**
@cond development
@file Svt.c
@brief Implementation and documentation of the SVT facility.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include <stdlib.h> 
#include <string.h>

#include "elf/linker.h"
#include "svt/Svt.h"
#include "ldr/Ldr.h"

/*
**
** Although the loader is targeted to the ARM 32-bit architecture, in fact ELF supports
** either 32-bit or 64-bit architectures. These #defines serve to serve as at least a 
** start of porting these functions should there be a move to a 64-bit architecture.  
**
*/

typedef Elf32_Sym   Symbol;
typedef Elf32_Word  Word;
typedef Elf32_Phdr  Segment;
typedef Elf32_Word  SegmentTag;  
typedef Elf32_Dyn   Element; 
typedef Elf32_Sword DynamicTag;

#define BITSET(field, word) (word |=  (1 << field))
#define BITCLR(field, word) (word &= ~(1 << field))

/*
** ++
**
** Symbol Table Block (STB)
**
** --
*/

typedef struct {
  Word        nbuckets;
  Word*       buckets;
  Symbol*     symbols;        //  
  Word        symndx;
  Word*       hashvals;
  const char* strings;        // 
  uint8_t*    base;           // 
  Word        pad; 
} STB;

/*
**
** Forward declarations for (facility private) local functions...
**
*/

static uint_fast32_t _hash(Svt_symbol);
static Segment*      _segment(SegmentTag, Ldr_elf*);
static Element*      _element(DynamicTag, Element* vector);
static STB*          _convert(void* stb, Ldr_elf*);
static void*         _translate(Svt_symbol, STB*);

/*
** ++
**
**
** --
*/
 
static uint32_t _resident = 0; 
static STB*     _stbs[SVT_MAX_TABLES];
           
/*
** ++
**
**
** --
*/

void* Svt_Translate(Svt_symbol symbol, uint32_t search_list)
 {

 uint32_t remaining = _resident & search_list;
 STB**    stbs      = _stbs;

 while(remaining)
  {

  uint32_t stb = __builtin_ffsl(remaining) - 1;
  
  BITCLR(stb, remaining);
 
  void* value = _translate(symbol, stbs[stb]);
 
  if(value) return value;  
  }
 
 return (void*)0;
 }

/*
** ++
** 
**
** --
*/

uint32_t Svt_Install(uint32_t table, const char* image)
 {
 
 uint32_t status;
 
 if(table > (SVT_MAX_TABLES - 1)) return SVT_INVALID_TABLE;
 if((1 << table) & _resident)     return SVT_DUPLICATE; 
 
 lnk_load(image, (void *)table, &status, NULL);
 
 return status;
 }

/*
** ++
**
**
** --
*/

uint32_t Svt_Setup(uint32_t table, Ldr_elf* elf, void* stb)
 {
 
 STB* stbc = _convert(stb, elf);

 if(!stbc) return SVT_IMAGE_NOT_TABLE;

 _stbs[table] = stbc;
 _resident   |= 1 << table;

 return SVT_SUCCESS;  
 }

/*
** ++
**
**
** --
*/

#define ERROR (STB*)0 

STB* _convert(void* stb, Ldr_elf* elf)
 {

 uint8_t* base = Ldr_Offset(elf);  

 Segment* segment = _segment(PT_DYNAMIC, elf);
 
 if(!segment) return ERROR;
 
 Element* vector = (Element*)(base + segment->p_vaddr);

 Element* element = _element(DT_STRTAB, vector);
 
 if(!element) return ERROR;

 const char* strings = (const char*)(base + element->d_un.d_ptr);
     
 element = _element(DT_SYMTAB, vector);
 
 if(!element) return ERROR;
 
 Symbol* symbols = (Symbol*)(base + element->d_un.d_ptr); 

 element = _element(DT_GNU_HASH, vector);

 if(!element) return ERROR;

 STB* this = (STB*)stb;
 
 this->strings = strings;

 this->symbols = symbols; 

 unsigned* hash = (unsigned*)(base + element->d_un.d_ptr);
 
 unsigned nbuckets = *hash++;
 
 this->nbuckets = nbuckets;
 
 this->symndx = *hash++;
 
 uint32_t bloomsize = *hash++; // as we don't need bloom filter, don't save bloom size

 hash++;                       // diito for bloom shift count...
  
 hash += bloomsize;
 
 this->buckets = hash;
 
 hash += nbuckets;
 
 this->hashvals = hash;
 
 this->base = base;
 
 return this;
 }

/*
** ++
**
**
** --
*/


#define FOREVER 1

void* _translate(Svt_symbol this, STB* stb)
 { 
 
 Word hash  = _hash(this);
 Word index = stb->buckets[hash % stb->nbuckets];
        
 if(!index) return (void*)0;
     
 const Symbol* symbol  = &stb->symbols[index];  
 Word*         hashval = &stb->hashvals[index - stb->symndx];
 
 hash &= ~1;
     
 while(FOREVER) 
  {
                                
  Word expected = *hashval++;

  const char* name = stb->strings + symbol->st_name;
  
  if((hash == (expected & ~1)) && !strcmp(this, name)) return (void*)(stb->base + symbol->st_value);

  if(expected & 1) break; // end of chain...
  
  symbol++;
  }

return (void*)0;
}

/*
** ++
**
**
** --
*/

static uint_fast32_t _hash(Ldr_name symbol)
 {
 
 uint_fast32_t  hash      = 5381;
 const uint8_t* next      = (const uint8_t*)symbol;
 unsigned       next_char = *next++;
 
 while(next_char)
  {
  hash = hash * 33 + next_char;
 
  next_char = *next++;
  }

return hash & 0xFFFFFFFF;
}

/*
** ++
**
**
** --
*/

static Segment* _segment(SegmentTag tag, Ldr_elf* elf)
 {
 
 Segment* segment   = (Segment*)((uint8_t*)elf + (uint32_t)elf->e_phoff); 
 uint32_t remaining = (uint32_t)elf->e_phnum;

 while(remaining--) {if(segment->p_type == tag) return segment; segment++;}  

 return (Segment*)0;
 }
 
/*
** ++
**
**
** --
*/

static Element* _element(DynamicTag tag, Element* vector)
 {
 
 Element* entry = vector;

 while(entry->d_tag != DT_NULL) {if(entry->d_tag == tag) return entry; entry++;}  

 return (Element*)0;  
 }


/** @endcond */

// Documentation

// #defines 
/**
   @def SVT
   @brief Facility number.  See configuration/system

   @def SVT_MAX_TABLES
   @brief Maximum number of SVTs.

   @def SVT_SYS_TABLE_NUM
   @brief The System SVT table index used by ::Svt_Install.

   @def SVT_APP_TABLE_NUM
   @brief The Application SVT table index used by ::Svt_Install.

   @def SVT_SYS_TABLE
   @brief The System SVT table bitmask for use with ::Svt_Translate.

   @def SVT_APP_TABLE
   @brief The Application SVT table bitmask for use with ::Svt_Translate.

   @def SVT_ANY_TABLE
   @brief The "any" table wildcard for use with ::Svt_Translate.

 */

// typedefs
/**
   @typedef Svt_symbol
   @brief C string to use for symbol lookups.
 */


// Enums
/**
   @enum Svt_status
   @brief The possible status codes returned by ::Svt_Install.

   @var SVT_SUCCESS         
   @brief Zero is always success, non-zero is an error.
   
   @var SVT_INVALID_TABLE   
   @brief The table index supplied to ::Svt_Install was greater than
   (::SVT_MAX_TABLES - 1)

   @var SVT_DUPLICATE       
   @brief The table index supplied to ::Svt_Install has already been allocated.

   @var SVT_IMAGE_NOT_TABLE 
   @brief The image supplied to ::Svt_Install was not an SVT.
*/


// functions
/**

   @fn void* Svt_Translate(Svt_symbol symbol, uint32_t tables)
   @brief Look up (translate) the specified symbol located in a
          set of Symbol/Value Tables.
   @param[in] symbol   Pointer to a C string specifying the symbol.
   @param[in] tables   Bitlist of the tables to be searched.
   @return A pointer to the object (value) stored in the table.  If
           the symbol is not found in any of the SVTs specified in the
           bitlist, NIL (0) is returned.

   Each offset of the bitlist corresponds to a table. If the (one-bit)
   field corresponding to an offset is SET the table is to be
   searched. If the field is clear the table is ignored. Tables are
   numbered from from zero (0) to thirty-one (31). Tables are searched
   in monotonically increasing order. That is, the first table
   searched is table 0 and the last is table 31. The first table which
   contains the specified symbol terminates the translation. Tables
   ::SVT_SYS_TABLE_NUM (the "system" table) and 
   ::SVT_APP_TABLE_NUM (the initial "application" table) are
   pre-installed and always present. The potential other thirty tables
   (0-29) are determined and installed by the user through calls to
   ::Svt_Install.

   Independent of the how many tables end up installed the caller may
   always search through ALL tables using the constant
   ::SVT_ANY_TABLE.  Note that the passed bitlist is always masked off
   (qualified) by the set of tables which have been \b actually
   installed.

   @note It is the \b caller's responsibility to cast the returned object
         to the correct type.



   @fn uint32_t Svt_Install(uint32_t table, const char* image)
   @brief Load and install a specific Symbol/Value table.
   @param[in] table  The table \b index to be installed. [0..31]
   @param[in] image  A pointer to the string naming the table (namespace:image).
   @return The status of the install operation (::Svt_status).

   @note Tables ::SVT_SYS_TABLE_NUM (the "system table) and table
         ::SVT_APP_TABLE_NUM (the initial "application" table) are are
         reserved to the system and have been previously installed by
         the time the user calls this function.

   @note For the format of \c namespace:image, see the 
         <a href="../../ldr/html/index.html">Ldr Facility</a>.
  

*/
