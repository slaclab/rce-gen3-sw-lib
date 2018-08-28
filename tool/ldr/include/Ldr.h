// -*-Mode: C;-*-
/**
@file Ldr.h
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
LDR

@author
Michael Huffer, SLAC (mehsys@slac.stanford.edu)

@par Credits:
SLAC
*/

#ifndef TOOL_LDR_LDR_H
#define TOOL_LDR_LDR_H

#include <inttypes.h>

#include "system/statusCode.h"
#include "elf/elf32.h"

typedef Elf32_Ehdr  Ldr_elf;       // Loader only supports ARM 32-bit architecture
typedef const char* Ldr_name;      // Used to identify a sharable image...
typedef const char* Ldr_namespace; // Used to identify an image namespace...
typedef const char* Ldr_path;      // Used to identify a path name for an image...

#define LDR 1 /* Facility no., see configuration/system/faciliy.doc. */
#define LDR_MAX_PATH 256
#define LDR_MAP_ERROR  ((Ldr_path)0)
#define LDR_LOAD_ERROR ((Ldr_elf*)0)

typedef enum {
  LDR_SUCCESS             = STS_K_SUCCESS,       // Zero is always success, non-zero an error...
  LDR_NO_TRANSLATION      = STS_ENCODE(LDR, 1),  // Cannot map SO to FILE name
  LDR_NO_SUCH_FILE        = STS_ENCODE(LDR, 2),  // File cannot be opened (not found or access violation)
  LDR_NOT_ELF_FILE        = STS_ENCODE(LDR, 3),  // File is not an ELF file
  LDR_NOT_LOADABLE        = STS_ENCODE(LDR, 4),  // File does not contain any loadable segments
  LDR_INSFMEM             = STS_ENCODE(LDR, 5),  // Region does not contain enough free space for image 
  LDR_FILEIO_ERROR        = STS_ENCODE(LDR, 6),  // File read function function retuned with error
  LDR_IMAGE_NO_MATCH      = STS_ENCODE(LDR, 7),  // ELF image SO name did not match requested SO name
  LDR_NOT_ELF_IMAGE       = STS_ENCODE(LDR, 8),  // Image is not an ELF image 
  LDR_IMAGE_WITHOUT_NAME  = STS_ENCODE(LDR, 9),  // Specified ELF image did not define an SO name
  LDR_IMAGE_INSTALLED     = STS_ENCODE(LDR, 10), // Loaded image had previously been installed
  LDR_NO_DYN_SEGMENT      = STS_ENCODE(LDR, 11), // Image does not contain a dynamic segment
  LDR_NO_STRING_TABLE     = STS_ENCODE(LDR, 12), // Image does not contain a string table
  LDR_NO_SYMBOL_TABLE     = STS_ENCODE(LDR, 13), // Image does not contain a symbol table
  LDR_NO_HASH_TABLE       = STS_ENCODE(LDR, 14)  // Image does not contain a hash table
 } Ldr_status;

#if defined(__cplusplus)
extern "C" {
#endif

Ldr_path Ldr_Assign(Ldr_namespace, Ldr_path);

Ldr_path Ldr_Map(Ldr_name, char* buffer);

Ldr_path Ldr_Rename(Ldr_namespace, Ldr_path);

Ldr_path Ldr_Remove(Ldr_namespace);

Ldr_elf* Ldr_Load(Ldr_name, Ldr_status*);

Ldr_elf* Ldr_Lookup(Ldr_name);

void Ldr_Fixup(Ldr_elf*);

Ldr_status Ldr_LoadRtems(Ldr_name);

uint32_t Ldr_Sizeof(Ldr_elf*);

Ldr_name Ldr_Name(Ldr_elf*);

Ldr_status Ldr_Install(Ldr_elf*);

inline static uint8_t* Ldr_Offset(Ldr_elf* t) 
{
  return t->e_type == ET_DYN ? (uint8_t*)t : (uint8_t*)0;
} 

inline static uint8_t* Ldr_Entry(Ldr_elf* t) 
{
  return (uint8_t*)(Ldr_Offset(t) + t->e_entry);
} 

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
