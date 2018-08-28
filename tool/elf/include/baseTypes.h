// -*-Mode: C;-*-
/**
@file
@brief Define aliases and API for 32-bit little-endian ELF classes.

@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#ifndef TOOL_ELF_BASETYPES_H
#define TOOL_ELF_BASETYPES_H

#define ELF (9 << 16) /* Facility code for error messages. */

#include <stdint.h>
#include <stdio.h> /* For NULL.*/


#include "elf/elf32.h"

#define swp(a) a

typedef Elf32_Ehdr elf_Header;
typedef Elf32_Phdr elf_Pheader;
typedef Elf32_Sym  elf_Symbol;
typedef Elf32_Dyn  elf_Dynamic;
typedef Elf32_Rel  elf_Rel;
typedef void elf_InitFunc(void);

typedef enum {
  ELF_OK                   =    0,
  ELF_BAD_MAGIC            =    0x0001 | ELF,
  ELF_NOT_32BIT            =    0x0002 | ELF,
  ELF_BAD_VERSION          =    0x0004 | ELF,
  ELF_BAD_OBJECT_TYPE      =    0x0008 | ELF,
  ELF_BAD_HEADER_SIZE      =    0x0010 | ELF,
  ELF_NO_PROGRAM_HEADERS   =    0x0020 | ELF,
  ELF_NO_DYNAMIC_SECTION   =    0x0040 | ELF,
  ELF_NO_DYNAMIC_SYMBOLS   =    0x0080 | ELF,
  ELF_NO_DYNAMIC_STRINGS   =    0x0100 | ELF,
  ELF_NO_HASH_TABLE        =    0x0200 | ELF,
  ELF_NO_LOADABLE_SEGMENTS =    0x0400 | ELF
} elf_StatusCode;


static inline unsigned elf_Header_type    (const elf_Header* hdr)   {return swp(hdr->e_type);}
static inline unsigned elf_Header_machine (const elf_Header* hdr)   {return swp(hdr->e_machine);}
static inline unsigned elf_Header_version (const elf_Header* hdr)   {return swp(hdr->e_version);}
static inline unsigned elf_Header_entry   (const elf_Header* hdr)   {return swp(hdr->e_entry);}
static inline unsigned elf_Header_size    (const elf_Header* hdr)   {return swp(hdr->e_ehsize);}
static inline unsigned elf_Header_numPhdr (const elf_Header* hdr)   {return swp(hdr->e_phnum);}
static inline unsigned elf_Header_offPhdr (const elf_Header* hdr)   {return swp(hdr->e_phoff);}
static inline unsigned elf_Header_numShdr (const elf_Header* hdr)   {return swp(hdr->e_shnum);}
static inline unsigned elf_Header_offShdr (const elf_Header* hdr)   {return swp(hdr->e_shoff);}
static inline unsigned elf_Header_sectNameSectIndex(const elf_Header* hdr) {return swp(hdr->e_shstrndx);}

/** @brief Assuming that the program header table is in memory, calculate the address
    of its first entry.
*/
static inline const elf_Pheader* elf_Header_addrPhdr(const elf_Header* hdr) {
  return (const elf_Pheader*)((const char*)hdr + elf_Header_offPhdr(hdr));
}

/** @brief Make some simple checks on the ELF header.
    @return ELF status value (or a logical OR of several).

    Checks performed:
    - Correctness of the magic number.
    - 32-bit ELF.
    - ELF version.
    - Header size.
    - The object has program headers.
*/
static inline unsigned elf_Header_check(const elf_Header* hdr) {
  int  status = ELF_OK;
  if (     hdr->e_ident[EI_MAG0] != ELFMAG0
        || hdr->e_ident[EI_MAG1] != ELFMAG1
        || hdr->e_ident[EI_MAG2] != ELFMAG2
        || hdr->e_ident[EI_MAG3] != ELFMAG3
      )                                               status |= ELF_BAD_MAGIC;
  
  if (hdr->e_ident[EI_CLASS]       != ELFCLASS32)     status |= ELF_NOT_32BIT;
  if (hdr->e_ident[EI_VERSION]     != EV_CURRENT)     status |= ELF_BAD_VERSION;
  if (elf_Header_type(hdr)         != ET_EXEC
      && elf_Header_type(hdr)      != ET_DYN)         status |= ELF_BAD_OBJECT_TYPE;
  if (elf_Header_size(hdr)    != sizeof(*hdr))        status |= ELF_BAD_HEADER_SIZE;
  if (elf_Header_numPhdr(hdr) <= 0)                   status |= ELF_NO_PROGRAM_HEADERS;
  if (elf_Header_offPhdr(hdr) < elf_Header_size(hdr)) status |= ELF_NO_PROGRAM_HEADERS;
  return status;
}


static inline unsigned elf_Pheader_type       (const elf_Pheader* phdr) {return swp(phdr->p_type);}
static inline unsigned elf_Pheader_fileSize   (const elf_Pheader* phdr) {return swp(phdr->p_filesz);}
static inline unsigned elf_Pheader_memSize    (const elf_Pheader* phdr) {return swp(phdr->p_memsz);}
static inline unsigned elf_Pheader_fileOffset (const elf_Pheader* phdr) {return swp(phdr->p_offset);}
static inline unsigned elf_Pheader_virtAddr   (const elf_Pheader* phdr) {return swp(phdr->p_vaddr);}
static inline unsigned elf_Pheader_physAddr   (const elf_Pheader* phdr) {return swp(phdr->p_paddr);}
static inline unsigned elf_Pheader_flags      (const elf_Pheader* phdr) {return swp(phdr->p_flags);}
static inline unsigned elf_Pheader_align      (const elf_Pheader* phdr) {return swp(phdr->p_align);}

static inline unsigned elf_Pheader_isDummy      (const elf_Pheader* phdr)
  {return elf_Pheader_type(phdr) == PT_NULL;}

static inline unsigned elf_Pheader_isLoadable   (const elf_Pheader* phdr)
  {return elf_Pheader_type(phdr) == PT_LOAD;}

static inline unsigned elf_Pheader_isDynamic    (const elf_Pheader* phdr)
  {return elf_Pheader_type(phdr) == PT_DYNAMIC;}

static inline unsigned elf_Pheader_isExidx      (const elf_Pheader* phdr)
  {return elf_Pheader_type(phdr) == PT_ARM_EXIDX;}

static inline unsigned elf_Pheader_isExecutable (const elf_Pheader* phdr)
  {return 0 != (PF_X & elf_Pheader_flags(phdr));}

/** @brief Assuming that the segment described by a program header is in memory,
    calculate the address of the segment.
*/
static inline const char* elf_Pheader_segAddr (const elf_Pheader* phdr, const char* baseAddr) {
  return baseAddr + elf_Pheader_virtAddr(phdr);
}

static inline unsigned elf_Dynamic_tag           (const elf_Dynamic* dyn)
{return swp(dyn->d_tag);}

static inline unsigned elf_Dynamic_isSentinel    (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_NULL;}

static inline unsigned elf_Dynamic_isSymtab      (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_SYMTAB;}

static inline unsigned elf_Dynamic_isStringtab   (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_STRTAB;}

static inline unsigned elf_Dynamic_isSysvHashtab (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_HASH;}

static inline unsigned elf_Dynamic_isGnuHashtab  (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_GNU_HASH;}

static inline unsigned elf_Dynamic_isNeeded      (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_NEEDED;}

static inline unsigned elf_Dynamic_isInittab     (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_INIT_ARRAY;}

static inline unsigned elf_Dynamic_isInittabSize (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_INIT_ARRAYSZ;}

static inline unsigned elf_Dynamic_isReltab      (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_REL;}

static inline unsigned elf_Dynamic_isReltabSize  (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_RELSZ;}

static inline unsigned elf_Dynamic_isPltReltab   (const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_JMPREL;}

static inline unsigned elf_Dynamic_isPltReltabSize(const elf_Dynamic* dyn)
{return elf_Dynamic_tag(dyn) == DT_PLTRELSZ;}

static inline unsigned elf_Dynamic_value         (const elf_Dynamic* dyn)
{return swp(dyn->d_un.d_val);}

static inline const char* elf_Dynamic_ptr        (const elf_Dynamic* dyn, const char* base)
{return swp(dyn->d_un.d_ptr) + base;}

/** @brief Assuming that the dynamic symbol table is in memory, calculate its address if this
    entry has that information (NULL otherwise).
    @param[in] base The ELF base address, i.e., what you add to a file vaddr to get a real memory address.
*/
static inline const elf_Symbol* elf_Dynamic_symtabAddr (const elf_Dynamic* dyn, const char* base) {
  return elf_Dynamic_isSymtab(dyn) ? (const elf_Symbol*)elf_Dynamic_ptr(dyn, base) : NULL;
}

/** @brief Assuming that the dynamic string table is in memory, calculate its address if this
    entry has that information (NULL otherwise).
    @param[in] base The ELF base address, i.e., what you add to a file vaddr to get a real memory address.
*/
static inline const char* elf_Dynamic_stringtabAddr (const elf_Dynamic* dyn, const char* base) {
  return elf_Dynamic_isStringtab(dyn) ? elf_Dynamic_ptr(dyn, base) : NULL;
}

/** @brief Assuming that the System V hash table is in memory, calculate its address if this
    entry has that information (NULL otherwise).
    @param[in] base The ELF base address, i.e., what you add to a file vaddr to get a real memory address.
*/
static inline const unsigned* elf_Dynamic_sysvHashtabAddr (const elf_Dynamic* dyn, const char* base) {
  return elf_Dynamic_isSysvHashtab(dyn) ? (const unsigned*)elf_Dynamic_ptr(dyn, base) : NULL;
}

/** @brief Assuming that the GNU hash table is in memory, calculate its address if this
    entry has that information (NULL otherwise).
    @param[in] base The ELF base address, i.e., what you add to a file vaddr to get a real memory address.
*/
static inline const unsigned* elf_Dynamic_gnuHashtabAddr (const elf_Dynamic* dyn, const char* base) {
  return elf_Dynamic_isGnuHashtab(dyn) ? (const unsigned*)elf_Dynamic_ptr(dyn, base) : NULL;
}

/** @brief If the dynamic section entry is a NEEDED entry then given the address of the dynamic
    string table return the address of the soname string. For non-NEEDED entries return NULL.
*/
static inline const char* elf_Dynamic_needed     (const elf_Dynamic* dyn, const char* strtab) {
  return elf_Dynamic_isNeeded(dyn) ? elf_Dynamic_value(dyn) + strtab : NULL;
}

/* @brief If the dynamic section entry is the location of the ordinary, non-PLT relocation table
   then return its address else return NULL.
*/
static inline const elf_Rel* elf_Dynamic_reltabAddr(const elf_Dynamic* dyn, const char* base) {
  return elf_Dynamic_isReltab(dyn) ? (const elf_Rel*)elf_Dynamic_ptr(dyn, base) : NULL;
}

/* @brief If the dynamic section entry is the location of the PLT relocation table
   then return its address else return NULL.
*/
static inline const elf_Rel* elf_Dynamic_pltReltabAddr(const elf_Dynamic* dyn, const char* base) {
  return elf_Dynamic_isPltReltab(dyn) ? (const elf_Rel*)elf_Dynamic_ptr(dyn, base) : NULL;
}

/* @brief If the dynamic section entry is the location of the init array
   then return its address else return NULL.
*/
static inline elf_InitFunc** elf_Dynamic_inittabAddr(const elf_Dynamic* dyn, const char* base) {
  return elf_Dynamic_isInittab(dyn) ? (elf_InitFunc**)elf_Dynamic_ptr(dyn, base) : NULL;
}

/* @brief If the dynamic section entry is the size of the ordinary, non-PLT relation table
   then return the number of elements else return 0.
*/
static inline unsigned elf_Dynamic_numRels(const elf_Dynamic* dyn) {
  return elf_Dynamic_isReltabSize(dyn) ? elf_Dynamic_value(dyn) / sizeof(Elf32_Rel) : 0;
}

/* @brief If the dynamic section entry is the size of the PLT relation table
   then return the number of elements else return 0.
*/
static inline unsigned elf_Dynamic_numPltRels(const elf_Dynamic* dyn) {
  return elf_Dynamic_isPltReltabSize(dyn) ? elf_Dynamic_value(dyn) / sizeof(Elf32_Rel) : 0;
}

/* @brief If the dynamic section entry is the size of the init array
   then return the number of elements else return 0.
*/
static inline unsigned elf_Dynamic_numInits(const elf_Dynamic* dyn) {
  return elf_Dynamic_isInittabSize(dyn) ? elf_Dynamic_value(dyn) / sizeof(elf_InitFunc*) : 0;
}

static inline unsigned elf_Symbol_nameIndex (const elf_Symbol* sym) {return swp(sym->st_name);}
static inline unsigned elf_Symbol_hasName   (const elf_Symbol* sym) {return elf_Symbol_nameIndex(sym) != 0;}
static inline unsigned elf_Symbol_value     (const elf_Symbol* sym) {return swp(sym->st_value);}
static inline unsigned elf_Symbol_size      (const elf_Symbol* sym) {return swp(sym->st_size);}
static inline unsigned elf_Symbol_type      (const elf_Symbol* sym) {return ELF32_ST_TYPE(swp(sym->st_info));}
static inline unsigned elf_Symbol_binding   (const elf_Symbol* sym) {return ELF32_ST_BIND(swp(sym->st_info));}
static inline unsigned elf_Symbol_section   (const elf_Symbol* sym) {return swp(sym->st_shndx);}
static inline unsigned elf_Symbol_visibility(const elf_Symbol* sym) {return ELF32_ST_VISIBILITY(sym->st_other);}
static inline unsigned elf_Symbol_isAbsolute(const elf_Symbol* sym) {return elf_Symbol_section(sym) == SHN_ABS;}

static inline unsigned elf_Symbol_isImport  (const elf_Symbol* sym)
  {return elf_Symbol_section(sym) == SHN_UNDEF && elf_Symbol_hasName(sym);}

static inline unsigned elf_Symbol_isDefinition (const elf_Symbol* sym)
  {return elf_Symbol_section(sym) != SHN_UNDEF;}

static inline unsigned elf_Symbol_isLocal   (const elf_Symbol* sym) {return elf_Symbol_binding(sym) == STB_LOCAL;}

static inline unsigned elf_Symbol_isGlobal  (const elf_Symbol* sym)
  {return elf_Symbol_binding(sym) == STB_GLOBAL || elf_Symbol_binding(sym) == STB_WEAK;}

static inline unsigned elf_Symbol_isExport  (const elf_Symbol* sym)
  {return elf_Symbol_section(sym) != SHN_UNDEF && elf_Symbol_isGlobal(sym);}

static inline unsigned  elf_Symbol_isWeak   (const elf_Symbol* sym) {return elf_Symbol_binding(sym) == STB_WEAK;}
static inline unsigned  elf_Symbol_isStrong (const elf_Symbol* sym) {return !elf_Symbol_isWeak(sym);}

static inline const char* elf_Symbol_name(const elf_Symbol* sym, const char* const stringtabAddr) {
  return stringtabAddr + elf_Symbol_nameIndex(sym);
}

static inline unsigned elf_stringeq(const char *c, const char *d) {
  for (; *c == *d && *c != '\0'; ++c, ++d);
  return *c == *d;
}

/** @brief Does the name of an ELF symbol match a given search target? */
static inline unsigned elf_Symbol_nameMatches
  (const elf_Symbol* sym,
   const char *const target,
   const char *const stringtabAddr
   )
{
  const char *nm = elf_Symbol_name(sym, stringtabAddr);
  return elf_stringeq(nm, target);
}

static inline char* elf_Rel_target(const elf_Rel* rel, const char* base) {return swp(rel->r_offset) + (char*)base;}

static inline unsigned elf_Rel_symbolIndex(const elf_Rel* rel) {return ELF32_R_SYM(swp(rel->r_info));}

static inline unsigned elf_Rel_type(const elf_Rel* rel) {return ELF32_R_TYPE(swp(rel->r_info));}

#undef swp
#endif
