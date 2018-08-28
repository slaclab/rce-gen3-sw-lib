// -*-Mode: C;-*-
/**
@file
@brief Test code for the ARM dynamic linker.

The test uses shared objects one, two, three, four, five and six with the
following dependencies:

one
  four
    five
  two
    four
      five
  three
    four
      five

six
  four

Object four is like the C library; everybody needs it. Object one doesn't
depend even indirectly on object six; the latter is loaded by lnk_loadNeeded()
after object one and its dependencies are fully loaded and linked.

The following symbols are exported by the objects:

one:   alpha
two:   beta
three: gamma
four:  delta
five:  alpha beta gamma delta epsilon

The following search scopes should be used for the objects:

one:   one four two three
two:   two four
three: three four
four:  four five
five:  five
six:   six four

Each shared object will refer to these exported symbols
which they should find in the following objects:

one: alpha(one) beta(two) gamma(three) delta(four)

two: beta(two) delta(four)

three: gamma(three) delta(four)

four: alpha(five) beta(five) gamma(five) delta(four) epsilon(five)

five: alpha(five) beta(five) gamma(five) delta(five) epsilon(five)

six: delta(four)

The contents of the relocation targets for each object are printed
both before and after linking so that one can see whether the
relocations have been performed correctly.

Each object will have a single dummy .init_array entry.
After loading but before linking that entry will be replaced by a
pointer to a function defined in this file, so that we can test the
handling of .init_array under Intel Linux. In fact no code in any of
the linked objects is executed so that we can run the entire test
under Intel Linux.

All the objects will be pre-loaded from the current working directory
but at the start none are considered as either loaded or
installed. The test provides local versions of the needed Ldr and Sym
functions.

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
2013/11/27

@par Last commit:
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/test/linux/testLinker_main.c $

@par Credits:
SLAC
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "elf/linker.h"
#include "elf/Object.h"

#include "ldr/Ldr.h"
#include "svt/Svt.h"

#include "../../src/lnkConstants.h"

void initOne(void)   {printf("Called init function for object one.\n");}
void initTwo(void)   {printf("Called init function for object two.\n");}
void initThree(void) {printf("Called init function for object three.\n");}
void initFour(void)  {printf("Called init function for object four.\n");}
void initFive(void)  {printf("Called init function for object five.\n");}
void initSix(void)   {printf("Called init function for object six.\n");}

int prelNone(void* prefs, void* elf) {
  printf("A prelude function was called by mistake.\n");
  exit(1);
  return 0;
}

int prelFour(void* prefs, void* elf) {
  printf("Called prelude function for object four. prefs = %p, elf = %p\n", prefs, elf);
  return LNK_OK;
}

int prelSix(void* prefs, void* elf) {
  printf("Called prelude function for object six. prefs = %p, elf = %p\n", prefs, elf);
  return LNK_OK;
}


void* prefsNone(void* prefsIn) {
  printf("A preferences function was called by mistake.\n");
  exit(1);
  return NULL;
}

void* prefsSix(void* prefsIn) {
  printf("Called preferences function for object six. prefsIn = %p, returning same.\n", prefsIn);
  return prefsIn;
}

typedef struct {
  const char* soname;
  const char* pathname;
  elf_Object  elfobj;
  void  (*    initFunc)(void);
  int   (*    preludeFunc)(void*, void*);
  void* (*    prefsFunc)(void*);
  Ldr_elf*    address;
  unsigned    memsize;
  bool        loaded;
  bool        installed;
} LoadedObject;

const int trueVal = 1;

LoadedObject directory[] = {
  {.soname = "one",   .pathname = "./one.exe",   .initFunc = initOne,    .preludeFunc = prelNone,
   .prefsFunc = prefsNone},

  {.soname = "two",   .pathname = "./two.so",    .initFunc = initTwo,    .preludeFunc = prelNone,
   .prefsFunc = prefsNone},

  {.soname = "three", .pathname = "./three.so",  .initFunc = initThree,  .preludeFunc = prelNone,
   .prefsFunc = prefsNone},

  {.soname = "four",  .pathname = "./four.so",   .initFunc = initFour,   .preludeFunc = prelFour,
   .prefsFunc = prefsNone},

  {.soname = "five",  .pathname = "./five.so",   .initFunc = initFive,   .preludeFunc = prelNone,
   .prefsFunc = prefsNone},

  {.soname = "six",   .pathname = "./six.so",    .initFunc = initSix,    .preludeFunc = prelSix,
   .prefsFunc = prefsSix},

  {.soname = NULL}
};

int searchBySoname(const char* soname) {
  int i;
  for (i = 0; directory[i].soname; ++i) {
    if (0 == strcmp(soname, directory[i].soname)) return i;
  }
  return -1;
}

int searchByPathname(const char* pathname) {
  int i;
  for (i = 0; directory[i].soname; ++i) {
    if (0 == strcmp(pathname, directory[i].pathname)) return i;
  }
  printf("Error: searchByPathname() could not find %s.\n", pathname);
  exit(1);
}

int searchByAddress(Ldr_elf* address) {
  int i;
  for (i = 0; directory[i].soname; ++i) {
    if (directory[i].address == address) return i;
  }
  printf("Error: searchByAddress() could not find %p.\n", address);
  exit(1);
}

const char* objectContaining(uintptr_t addr) {
  int i;
  for (i = 0; directory[i].soname; ++i) {
    uintptr_t const start = (uintptr_t)directory[i].address;
    uintptr_t const end   = start + directory[i].memsize;
    if (addr >= start && addr < end) return directory[i].soname;
  }
  return "?";
}

void Ldr_Fixup(Ldr_elf* header) {}


const char* sixPrefs = "";

void* Svt_Translate(Svt_symbol soname, uint32_t tableSet) {
  /* Special hack for the preferences of object six. */
  if (0 == strcmp(soname, sixPrefs)) {
    printf("%s translates to %p.\n", soname, (void*)0xfeedcabb);
    return (void*)0xfeedcabb;
  }
  int const i = searchBySoname(soname);
  if (i < 0) {printf("No translation for %s.\n", soname); return NULL;}
  printf("%s translates to %s.\n", soname, directory[i].pathname);
  return (void*)directory[i].pathname;
}

Ldr_elf* Ldr_Load(Ldr_name soname, Ldr_status* status) {
  int const i = searchBySoname(soname);
  printf("Loading %s.\n", soname);
  directory[i].loaded = TRUE;
  *status = LDR_SUCCESS;
  return directory[i].address;
}

Ldr_status Ldr_Install(Ldr_elf* elf) {
  int const i = searchByAddress(elf);
  if (i < 0) {
    printf("Error: Attempt to install the ELF object at %p, which doesn't exist.\n", elf);
    exit(1);
  }
  if (i == 0) { // Object one should not be installed.
    printf("Error: Attempt to install object \"%s\".\n", directory[i].soname);
    exit(1);
  }
  directory[i].installed = TRUE;
  printf("Installing %s.\n", directory[i].soname);
  return LDR_SUCCESS;
}

Ldr_elf* Ldr_Lookup(Ldr_name soname) {
  int const i = searchBySoname(soname);
  printf("Looking up %s: %s.\n", soname, directory[i].installed ? "Installed" : "Not installed");
  return directory[i].installed ? directory[i].address : NULL;
}


const char* loadElf(const char* filename, unsigned* memsize);

void dumpRelocations(const char* comment, int printSource);

void dumpSymbols(const char* comment);

const char* const symbol[] = {"alpha", "beta", "gamma", "delta", "epsilon", "lnk_prelude", NULL};

int main(int argc, const char* argv[]) {

  /* Preload everything, set init, prelude and preferences functions. */
  int i;
  for (i = 0; directory[i].soname; ++i) {
    directory[i].address = (Ldr_elf*)loadElf(directory[i].pathname, &directory[i].memsize);
    unsigned status = elf_Object_init(&directory[i].elfobj, (const char*)directory[i].address);
    if (status != ELF_OK) {
      printf("Bad ELF object, elf_Object_init() error code 0x%08x.\n", status);
    }
    
    /* Only one init function, alter .init_array directly. */
    directory[i].elfobj.inittab[0] = directory[i].initFunc;

    /* If the object defines lnk_prelude() change the symbol table entry to an absolute
       symbol pointing to the function we want to call.
    */
    { unsigned     const hash   = elf_Object_hash(&directory[i].elfobj, "lnk_prelude");
      elf_Symbol*  const elfsym = (elf_Symbol*)elf_Object_lookup(&directory[i].elfobj, hash, "lnk_prelude");
      if (elfsym) {
        elfsym->st_value = (Elf32_Addr)directory[i].preludeFunc;
        elfsym->st_shndx = SHN_ABS;
      }
    }

    /* The same deal for the preferences function. */
    { unsigned     const hash   = elf_Object_hash(&directory[i].elfobj, "lnk_preferences");
      elf_Symbol*  const elfsym = (elf_Symbol*)elf_Object_lookup(&directory[i].elfobj, hash,
                                                                 "lnk_preferences");
      if (elfsym) {
        elfsym->st_value = (Elf32_Addr)directory[i].prefsFunc;
        elfsym->st_shndx = SHN_ABS;
      }
    }
  }

  dumpSymbols("before linking");

  dumpRelocations("before linking", FALSE);

  /* Start linking with the "main" object. */
  printf("\nStarting link with lnk_load() on soname 'one'.\n");
  lnk_Status status;
  unsigned s;
  (void)lnk_load("one", NULL, &s, &status);
  printf("Return code from lnk_load() = 0x%08x\n", status.status);
  if (status.status) {
    if (status.soname) printf("  soname: %s\n", status.soname);
    if (status.otherName) printf(" Other name: %s\n", status.otherName);
  }

  /* Now load the extra object which is not used by any of the others. */
  printf("\nStarting link with lnk_load() on soname 'six'.\n");
  (void)lnk_load("six", NULL, &s, &status);
  printf("Return code from lnk_load() = 0x%08x\n", status.status);
  if (status.status) {
    if (status.soname) printf("  soname: %s\n", status.soname);
    if (status.otherName) printf(" Other name: %s\n", status.otherName);
  }

  printf("\nStarting link with lnk_load() on soname 'six' again, setting prefs = 0xdeadbeef.\n");
  (void)lnk_load("six", (void*)0xdeadbeef, &s, &status);
  printf("Return code from lnk_load() = 0x%08x\n", status.status);
  if (status.status) {
    if (status.soname) printf("  soname: %s\n", status.soname);
    if (status.otherName) printf(" Other name: %s\n", status.otherName);
  }

  printf("\nNote: The relocations for object six will be incorrect because\n"
         "they were done multiple times - we only really loaded each object once.\n");
  dumpRelocations("after all linking", TRUE);
  return 0;
}


void dumpSymbols(const char* comment) {
  /* Dump the symbol definitions adjusted for the locations of the objects. */
  int i;
  printf("\nSymbol values %s.", comment);
  for (i = 0; directory[i].soname; ++i) {
    int j;
    elf_Object elf;
    unsigned status = elf_Object_init(&elf, (const char*)directory[i].address);
    if (status != ELF_OK) {
      printf("ELF error for %s: 0x%08x\n", directory[i].soname, status);
      exit(1);
    }
    printf("\n%s symbols:\n", directory[i].soname);
    for (j = 0; symbol[j]; ++j) {
      unsigned hash;
      const elf_Symbol* elfsym;
      hash = elf_Object_hash(&elf, symbol[j]);
      elfsym = elf_Object_lookup(&elf, hash, symbol[j]);
      if (elfsym) printf("    %-10s = %p\n", symbol[j], (void*)elf_Object_getSymbolValue(&elf, elfsym));
    }
  }
}

void dumpRelocations(const char* comment, int printSource) {
  int i;
  /* Examine the altered contents of the relocation targets. */
  printf("\nRelocation targets %s.", comment);
  for (i = 0; directory[i].soname; ++i) {
    elf_Object elf;
    unsigned status = elf_Object_init(&elf, (const char*)directory[i].address);
    if (status != ELF_OK) {
      printf("ELF error for %s: 0x%08x\n", directory[i].soname, status);
      exit(1);
    }
    printf("\n%s:\n", directory[i].soname);
    const elf_Rel*          pltRelArray = NULL;   /* Relocations targeting the PLT. */
    unsigned                numPltRels  = 0;
    const elf_Rel*          relArray    = NULL;   /* Relocations targeting ordinary data. */
    unsigned                numRels     = 0;
    const char*             baseAddress; /* Add this to an Elf32_Addr value to get the actual address. */
    const elf_Dynamic*      dyn;
    baseAddress = elf.baseAddress;
    for (dyn = elf.dyntab; !elf_Dynamic_isSentinel(dyn); ++dyn) {
      switch (elf_Dynamic_tag(dyn)) {
      case DT_JMPREL:
        pltRelArray = (const elf_Rel*)elf_Dynamic_ptr(dyn, baseAddress);
        break;
      case DT_PLTRELSZ:
        numPltRels = elf_Dynamic_value(dyn) / sizeof(elf_Rel);
        break;
      case DT_REL:
        relArray = (const elf_Rel*)elf_Dynamic_ptr(dyn, baseAddress);
        break;
      case DT_RELSZ:
        numRels = elf_Dynamic_value(dyn) / sizeof(elf_Rel);
        break;
      }
    }
    int j;
    printf("Offset      Value       Symbol referenced\n");
    printf("----------  ----------  -----------------\n");
    for (j = 0; j < numRels; ++j) {
      const elf_Rel* rel = relArray + j;
      unsigned          const symIndex = elf_Rel_symbolIndex(rel);
      unsigned*         const where    = (unsigned*)elf_Rel_target(rel, elf.baseAddress);
      unsigned          const offset   = (unsigned) elf_Rel_target(rel, 0);
      const elf_Symbol* const symbol   = elf.dynSymboltab + symIndex;
      if (!printSource) 
        printf("0x%08x  0x%08x  %s\n",
               offset, *where, symIndex ? elf_Symbol_name(symbol, elf.dynStringtab) : "*none*");
      else
        printf("0x%08x  0x%08x  %s(%s)\n",
               offset, *where, symIndex ? elf_Symbol_name(symbol, elf.dynStringtab) : "*none*",
               objectContaining(*where));
    }
    for (j = 0; j < numPltRels; ++j) {
      const elf_Rel* rel = pltRelArray + j;
      unsigned          const symIndex = elf_Rel_symbolIndex(rel);
      unsigned*         const where    = (unsigned*)elf_Rel_target(rel, elf.baseAddress);
      unsigned          const offset   = (unsigned) elf_Rel_target(rel, 0);
      const elf_Symbol* const symbol   = elf.dynSymboltab + symIndex;
      if (!printSource) 
        printf("0x%08x  0x%08x  %s\n",
               offset, *where, symIndex ? elf_Symbol_name(symbol, elf.dynStringtab) : "*none*");
      else
        printf("0x%08x  0x%08x  %s(%s)\n",
               offset, *where, symIndex ? elf_Symbol_name(symbol, elf.dynStringtab) : "*none*",
               objectContaining(*where));
    }
  }
}
