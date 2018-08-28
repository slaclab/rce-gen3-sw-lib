// -*-Mode: C;-*-
/**
@file
@brief Linux test code for tool::elf.

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
2013/08/26

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/test/linux/testElf_main.c $

@par Credits:
SLAC
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "elf/baseTypes.h"
#include "elf/Object.h"

const char* loadElf(const char* filename);

static void lookupSymbols(const char* hdrAddress);

int main(int argc, const char* argv[]) {
  printf("\nTest program for ELF dynamic object handling.\n");
  if (argc != 2) {
    printf("Give ONE argument, the name of a dynamic object file.\n");
    exit(1);
  }

  lookupSymbols(loadElf(argv[1]));
  return 0;
}

void lookupSymbols(const char* hdrAddress) {
  elf_Object obj;
  static const char* symbol[] = {
    "_end", "spam", "egg", "sausage", "tomato", "lobster",
    "string1", "string2",
    NULL};
  int status = elf_Object_init(&obj, hdrAddress);
  int isym;
  if (status != ELF_OK) {
    printf("Status of ELF object init(): 0x%08x\n", status);
    exit(1);
  }

  printf("\nSymbol information from loaded ELF object.\n");
  printf("\nSymbol            Found?  Value         Ajusted value\n");
  printf(  "----------------  ------  ------------  -------------\n");
  for (isym = 0; symbol[isym]; ++isym) {
    unsigned const hash =  elf_Object_hash(&obj, symbol[isym]);
    const elf_Symbol* sym = elf_Object_lookup(&obj, hash, symbol[isym]);
    if (sym) {
      printf("%16s  Yes     0x%08x", symbol[isym], elf_Symbol_value(sym));
      if (!elf_Symbol_isAbsolute(sym)) {
        printf("    %p", (void*)elf_Object_getSymbolValue(&obj, sym));
      }
      printf("\n");
    }
    else {
      printf("%16s  No\n", symbol[isym]);
    }
 }

  printf("\n");
  /* Check if symbol values are correct by printing the two strings. */
  { const elf_Symbol* sym = elf_Object_lookup(&obj, elf_Object_hash(&obj, "string1"), "string1");
    if (sym) printf("String 1: %s\n", (char*)elf_Object_getSymbolValue(&obj, sym));
  }
  { const elf_Symbol* sym = elf_Object_lookup(&obj, elf_Object_hash(&obj, "string2"), "string2");
    if (sym) printf("String 2: %s\n", (char*)elf_Object_getSymbolValue(&obj, sym));
  }
 
}
