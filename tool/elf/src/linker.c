// -*-Mode: C;-*-
/**
@file
@brief Implementation of the dynamic linker for ARM.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Group:
PLT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2013/11/25

@par Last commit:
\$Date: 2015-01-12 10:57:01 -0800 (Mon, 12 Jan 2015) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3929 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/src/linker.c $

@par Credits:
SLAC

@if development
Implementation restrictions and requirements:

    R1) The scope searched for each object's references is the
    object itself then, in order, those objects on its needed
    list. There is no global scope.  The scope can be this small since
    every object satisfying a reference for the current object is
    required to be on the needed list for the current object.

    A side effect of R1 is that interposition is impossible.

    R2) Symbol versioning is not supported.

    R3) The ELF header and program header table are in the
    loaded object and the latter can be found using the
    p_offset value in the former as an offset in memory from
    the header address.

    R4) The ELF header is at offset zero in the loaded object.

    R5) Lazy completion of PLT entries is not supported; an
    object's PLT is filled during the relocation stage.

    R6) Only PIC code may be relocated. This reduces the set of
    relocation types to a handful of 32-bit data relocations.

    R7) All ELF objects must have a dynamic section.

    R8) A GNU-style hash table must be present.

Example list of objects with their NEEDED lists:
A: [B C D]
B: [C E]
C: [E]
D: []
E: []
#
The resulting DAG is: |-> child   -> sibling
A
|-> B -> C -> D
    |    |--------
    |            |
    | -> C -> E  |
         |       |
         | -> E <-|

There have to be multiple nodes referring to objects C and E because
they are referred to in multiple NEEDED lists and we have to
remember the order of each such list. However, in order to save
space the child links of all the nodes that refer to a given object
are the same. This is possible because each object only has a single NEEDED
list. This sharing makes the graph a DAG instead of a tree.

The DAG must be constructed depth-first so that all dependencies
of an object are bound and initialized before the object itsel.
In the example above E must be bound and initialized before C and
C must be done before either B or A. The entire init sequence
would be [E C B D A].

The scope for E would be [E], for C it would be [C E]
and for A it would be [A B C D].
@endif
*/
#include <stdarg.h>
#include <stdio.h> /* For NULL. */

#include "debug/print.h"

#include "elf/baseTypes.h"
#include "elf/linker.h"
#include "elf/lnkStatus.h"

#include "ldr/Ldr.h"
#include "svt/Svt.h"

#include "exidx.h"
#include "lnkConstants.h"
#include "lnkObjDb.h"
#include "lnkSearchDag.h"

/** @cond development */

/**
    @brief Return as a void* the transfer address from the ELF header. The
    returned value will be NULL if there is no transfer address set.
    @param[in] elf a pointer to an ELF shared object (such as returned by lnk_load()).
    @return the transfer address (or NULL)
*/
void* lnk_go(Ldr_elf* elf) {
  /* Assume that only ELF shared objects linked starting at vaddr zero
     are being used so that the true entry point address is found by
     adding the header address to the e_entry value in the header.
  */  
  return elf->e_entry ? (char*)elf+elf->e_entry : NULL;
}


static void innerLink
(lnk_ObjDb* objDb, lnk_SearchDag* searchDag, int currentLink, lnk_Status* status);

static void bind
(lnk_ObjDb* objDb, const lnk_SearchDag* searchDag, const lnk_ObjInfo* currentObj, lnk_Status* status);

/** @endcond */


/**
   @brief Load but never install the ELF object with the given soname then run the dynamic
   linker on it, which may result in the installation of some of its dependencies.
   @param[in] soname The soname for the ELF object.
   @param[in] rootPrefs A pointer which, if non-null, is passed to the lnk_prelude() function
   of the object named by the soname, and only that object.
   @param[out] status The address in which to place the status code resulting from the link.
   @param[out] extra  A pointer to a struct which holds extra information about how well the link went. 
   Can be null in which case that information is not passed back up.
   @return A pointer to the loaded ELF header or, in case of failure, NULL.
*/
Ldr_elf* lnk_load(Ldr_name soname, void* rootPrefs, uint32_t* status, lnk_Status* extra) {
  lnk_Status myExtra;
  Ldr_status ldStatus;
  if (!extra) extra = &myExtra;
  lnk_Status_init1(extra, LNK_OK);
  *status = LNK_OK;

  /* Load the initial shared object. */
  Ldr_elf* const elf = Ldr_Load(soname, &ldStatus);
  if (!elf) {
    lnk_Status_init2(extra, (unsigned)ldStatus, soname);
    *status = (uint32_t)ldStatus;
    return NULL;
  }

  /* Initialize the database of shared objects and insert its first node. */
  lnk_ObjDb objDb;
  lnk_ObjDb_init(&objDb);
  lnk_ObjDb_add(&objDb, (const char*)elf, LNK_DAG_NOT_STARTED, FALSE/*Not yet bound*/, soname, extra);
  *status = extra->status;
  if (*status) return NULL;

  /* Remember where to find the EXIDX table for the newly loaded object. */
  rememberExidx(&objDb.info[0].elfObject);

  /* Initialize the dependency graph ans start linking. */
  objDb.info[0].prefsOverride = rootPrefs;
  objDb.info[0].installable   = FALSE;
  lnk_SearchDag searchDag;
  int currentLink;
  currentLink = 1;
  lnk_SearchDag_init(&searchDag, 0/*DB index of first object*/);
  innerLink(&objDb, &searchDag, currentLink, extra);
  *status = extra->status;

  return *status ? NULL : elf;
}

/** @cond development */

/* Recursive linking routine, private.
   objDb: database of ELF objects seen since lnk_link() was called.
   searchDag: The DAG of SearchNodes reflecting the dependencies among the
   ELF objects in objDb.
   currentLink: The index in the search DAG of the ELF object whose
   needed-list is about to be examined.

   The search DAG at currentLink is extended with the direct
   dependencies of the current ELF object, loading each child as we
   discover it. We descend into each child's dependencies right after
   adding each child and before the remaining children are added.

   After all children have been added, if the current ELF object has
   not already been bound, it is bound using the part of the search
   DAG rooted at currentLink to resolve symbolic references. Just
   after binding the linker calls init code of the object using the
   pointers in the .init_array section, if any. Then the linker will
   call the prelude function of the image. 
*/
void innerLink(lnk_ObjDb* objDb, lnk_SearchDag* searchDag, int currentLink, lnk_Status* status) {
  /* Get the needed-list for the current object, making sure all the objects
     named are installed.
*/
  lnk_SearchNode*  currentSearch;
  lnk_ObjInfo*     currentObj;
  currentSearch = lnk_SearchDag_get(searchDag, currentLink, status);
  if (status->status != LNK_OK) return;
  currentObj = lnk_ObjDb_get(objDb, currentSearch->dbIndex, status);
  if (status->status != LNK_OK) return;

  /* If we come around to a search node whose sub-DAG is started but not finished
     then we have a dependency cycle.
  */
  if (currentObj->dagState == LNK_DAG_UNDER_CONSTRUCTION) {
    lnk_Status_init1(status, LNK_DEPENDENCY_CYCLE);
    return;
  }

  /* If we encounter an object whose sub-DAG is known, no need for
     further recursion. The ELF objects referred to in the sub-DAG
     will have been installed.
  */
  if (currentObj->dagState == LNK_DAG_DONE) {
    currentSearch->child = currentObj->dag;
    return;
  }

  /* We have an object whose sub-DAG has not yet been started. Add
     child nodes using the NEEDED list and descend from each child as
     soon as it's been added.
  */
  currentObj->dagState = LNK_DAG_UNDER_CONSTRUCTION;
  { const elf_Dynamic* const dyn = currentObj->elfObject.dyntab;
    int i;
    for (i = 0; !elf_Dynamic_isSentinel(dyn+i); ++i) {
      const char* const soname = elf_Dynamic_needed(dyn+i, currentObj->elfObject.dynStringtab);
      if (soname) {
	const int dbIndex = lnk_ObjDb_addNeeded(objDb, soname, status);
	if (status->status != LNK_OK) return;
	int childLink = lnk_SearchDag_addChild(searchDag, currentLink, dbIndex, status);
	if (status->status != LNK_OK) return;
        /* Now descend to the next level of loading and binding. */
        innerLink(objDb, searchDag, childLink, status);
        if (status->status != LNK_OK) return;
      }
    }
  }

  /* The sub-DAG rooted at the current search node is complete. */
  currentObj->dag = currentSearch->child;
  currentObj->dagState = LNK_DAG_DONE;

  /* Now bind, if necessary, the current object using the search DAG
     rooted at the currentSearch node.
  */
  if (!currentObj->bound) {
    bind(objDb, searchDag, currentObj, status);
    if (status->status != LNK_OK) return;
    currentObj->bound = TRUE;
  }
}

static void relocate
(const lnk_ObjDb*     objDb,
 const lnk_SearchDag* searchDag,
 const lnk_ObjInfo*   currentObject,
 const elf_Rel*       relArray,
 unsigned             numRels,
 lnk_Status*          status);

typedef void  (*InitArrayElement) (void);
typedef int   (*PreludeFunc)      (const void* imagePrefs, const void* elfHeader);
typedef void* (*PrefsFunc)        (const void* prefsIn);

static const void* lookup(const lnk_ObjInfo* obj, const char* name);

static void bind
(lnk_ObjDb* objDb, const lnk_SearchDag* searchDag, const lnk_ObjInfo* currentObj, lnk_Status* status) {
  /* Perform relocations. */
  int i;

  lnk_Status_init1(status, LNK_OK);

  relocate(objDb, searchDag, currentObj, currentObj->elfObject.reltab,
           currentObj->elfObject.numRels, status);
  if (status->status == LNK_OK)
    relocate(objDb, searchDag, currentObj, currentObj->elfObject.pltReltab,
             currentObj->elfObject.numPltRels, status);

  if (status->status != LNK_OK) return;

  /* Call the initialization code. */
  for (i = 0; i < currentObj->elfObject.numInits; ++i) currentObj->elfObject.inittab[i]();

  /* Retrieve linker options, if present, from the image. The default is to have no options set. */
  const unsigned* const popts   = (unsigned*)lookup(currentObj, "lnk_options");
  unsigned        const options = popts ? *popts : 0;

  /* Is there a prelude function to call? */
  PreludeFunc const preludeFunc = (PreludeFunc)lookup(currentObj, "lnk_prelude");
  if (preludeFunc) {
    /* There is. Find out what its prefs argument should be then make the call. */
    PrefsFunc       const prefsFunc = (PrefsFunc)lookup(currentObj, "lnk_preferences");
    void*                 prefs     = currentObj->prefsOverride;
    if (prefsFunc)        prefs     = prefsFunc(prefs);
    int             const stat      = preludeFunc(prefs, currentObj->elfObject.header);
    lnk_Status_init1(status, stat);
    if (stat != LNK_OK) return;
  }

  /* Change memory attributes. */
  Ldr_Fixup((Ldr_elf*)currentObj->elfObject.header);

  /* Install if appropriate. */
  if (currentObj->installable && (options & LNK_INSTALL))
    Ldr_Install((Ldr_elf*)currentObj->elfObject.header);
}
/** @endcond */ // development

/**
    @brief Look up a symbol in a loaded ELF image. Returns NULL if not
    found, else returns the relocated symbol value.
    @param[in] elf a pointer to an ELF shared object (such as returned by lnk_load()).
    @param[in] name the symbol to look up
    @return the relocated symbol value (or NULL)
*/
void* lnk_lookup(Ldr_elf* elf, const char* name) {
  // Use just the elfObject field of the object-info, enough for lookup().
  lnk_ObjInfo info;
  elf_Object_init(&info.elfObject, (const char*)elf);
  return (void*)lookup(&info, name);
}

/** @cond development */
const void* lookup(const lnk_ObjInfo* obj, const char* name) {
  unsigned          const hash   = elf_Object_hash  (&obj->elfObject, name);
  const elf_Symbol* const symbol = elf_Object_lookup(&obj->elfObject, hash, name);
  return symbol ? (void*)elf_Object_getSymbolValue(&obj->elfObject, symbol) : NULL;
}

/*
     The following relocations will appear in PIC ELF objects. Note
     that all operate on word-aligned 32-bit targets. The addend A, if
     needed, for each relocation will be the 32-bit value at the
     relocation target address.

     For relocations which support ARM/Thumb interworking
     and which refer to symbols of type STT_FUNC:
         The effective symbol value is sym_eff = (sym_value & ~1).
         The value of T is (sym_value & 1).

     R_ARM_GLOB_DAT
         target = (target + sym_eff) | T.

     R_ARM_JUMP_SLOT
         target = sym_eff | T.
         It's this simple because we don't do lazy linking; we
         always look up the symbol.  We therefore ignore the addend
         which is an offset back to PLT slot 0. 

     R_ARM_RELATIVE
         target += the module load address.
         The module is loaded in one piece and relocated in place.
         Therefore we don't care whether the a symbol is referenced
         or not; all segments are relocated by the same amount which
         is the module load address.

     R_ARM_ABS32
         target = (target  + sym_eff) | T.
         Generated when the absolute address of a weakly defined
         item is needed. Usually it's for type info or vtable data.

     R_ARM_REL32
         target = (sym_eff + target - target address) | T.
         Generated when an EXTAB entry must refer to a weakly
         defined item, usually a typeinfo object or a vtable.

     I don't know why the ABS32 relocation is used instead of
     GLOB_DAT unless it's to provide an easy way to tell if the
     relocation is for a weak definition, but then one can tell that
     by looking at the symbol table entry.

     Single relocation.
     rtype: The relocation type from the ElfRel.
     where: The address at which to put the new address-related value.
     symbolValue: The relocated value of the symbol. 
       For interworking addresses symbolValue is
       (symbol value & ~1U) rather than the exact symbol value.
     thumb: 1 if the symbol was for a Thumb interworking address, else 0.
     baseaddr: The ELF base address of the containing object.
*/
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

static void modifyObject
(int       const rtype,
 unsigned* const where,
 unsigned        symbolValue,
 unsigned        thumb,
 unsigned        baseaddr,
 lnk_Status*     status);



/* Use to avoid repeating expensive searches in other ELF objects. */
typedef struct {
  unsigned    symIndex;
  unsigned    symval;
  unsigned    thumb;
} SymbolCache;

void relocate
(const lnk_ObjDb*     objDb,
 const lnk_SearchDag* searchDag,
 const lnk_ObjInfo*   currentObject,
 const elf_Rel*       relArray,
 unsigned             numRels,
 lnk_Status* status)
{
  const elf_Object* const obj      = &currentObject->elfObject;
  unsigned                i;
  SymbolCache             symCache = {.symIndex = 0xffffffff};

  lnk_Status_init1(status, LNK_OK);

  for (i = 0; i < numRels; ++i) {
    const elf_Rel*    const rel      = relArray + i;
    unsigned          const type     = elf_Rel_type(rel);
    unsigned          const symIndex = elf_Rel_symbolIndex(rel);
    unsigned*         const where    = (unsigned*)elf_Rel_target(rel, obj->baseAddress);
    const elf_Symbol* const symbol    = obj->dynSymboltab + symIndex;
    unsigned                symval;
    unsigned                thumb;
    if (symIndex == 0) {
      /* This relocation doesn't refer to any kind of symbol. */
      modifyObject(type, where, 0, 0, (unsigned)obj->baseAddress, status);
    }
    else if (elf_Symbol_isDefinition(symbol)) {
      /* This relocation refers to a locally defined symbol. */
      splitInterwork
        (elf_Object_getSymbolValue(obj, symbol), elf_Symbol_type(symbol), &symval, &thumb);
      modifyObject(type, where, symval, thumb, (unsigned)obj->baseAddress, status);
    }
    else {
      /* This relocation refers to an undefined symbol.
         Search the objects in the scope of the current object.
      */
      const elf_Object*       otherObj    = NULL;
      const elf_Symbol*       otherSymbol = NULL;
      unsigned                symval;
      unsigned                thumb;
      if (symIndex == symCache.symIndex) {
        /* Same external reference as last time. */
        symval = symCache.symval;
        thumb  = symCache.thumb;
      }
      else {
        /* External reference different from the last one. */
        const char* const name = elf_Symbol_name(symbol, obj->dynStringtab);
        unsigned    const hash = elf_Object_hash(obj, name);

        /* Search the objects that were on the NEEDED list. */
        int nextLink = currentObject->dag;
        while (nextLink) {
          const lnk_SearchNode* const search =
            lnk_SearchDag_get((lnk_SearchDag*)searchDag, nextLink, status);
          if (status->status != LNK_OK) return;
          const lnk_ObjInfo* const inf = lnk_ObjDb_get((lnk_ObjDb*)objDb, search->dbIndex, status);
          if (status->status != LNK_OK) return;
          otherObj    = &inf->elfObject;
          otherSymbol = elf_Object_lookup(otherObj, hash, name);
          if (otherSymbol) break;
          nextLink = search->sib;
        }

        /* Only a weak reference may go unsatisfied, in which case we assume a symbol value
           of zero.
        */
        if (!otherSymbol && !elf_Symbol_isWeak(symbol)) {
          lnk_Status_init1(status, LNK_UNDEFINED_SYMBOLS);
          dbg_printv("Undefined symbol %s in %s.\n", name, currentObject->soname);
          return;
        }
        if (!otherSymbol && elf_Symbol_isWeak(symbol)) {symval = 0; thumb = 0;}
        else {
          splitInterwork
            (elf_Object_getSymbolValue(otherObj, otherSymbol),
             elf_Symbol_type(otherSymbol), &symval, &thumb);
        }
        symCache.symIndex = symIndex;
        symCache.symval   = symval;
        symCache.thumb    = thumb;
      } /* Different external reference. */
      modifyObject(type, where, symval, thumb, (unsigned)obj->baseAddress, status);
    } /* External reference. */
  } /* Loop over relocations. */
}

void modifyObject
(int       const rtype,
 unsigned* const where,
 unsigned        symbolValue,
 unsigned        thumb,
 unsigned        baseaddr,
 lnk_Status*     status)
{
  lnk_Status_init1(status, LNK_OK);
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
    lnk_Status_init1(status, LNK_BAD_RELOCATION_TYPE);
  }
}
/** @endcond */ // development

// ========== Additional documentation ==========


/**
    @enum lnk_linkerOptions
    @brief Dynamic linker option bits which may be combined into a bitmask
    and placed in a const int variable named "lnk_options" in the shared object.

    @var LNK_INSTALL
    @brief Install this ELF object after loading it (unless it's
    an object whose soname was passed to lnk_load() as an argument).
*/


/**
   @enum lnk_StatusCode
   @brief Status codes belonging to the dynamic linker (facility no. 4).
   @details The 32-bit status code returned by lnk_load() has the facility
   number in the most significant 16 bits and the error code in the lower
   16 bits. If the facility code is not that for the linker it means that
   some facility it called generated the error (see facility.doc).

   @var LNK_OK
   @brief Facility-independent success code

   @var LNK_SUCCESS
   @brief Facility-independent success code

   @var LNK_BAD_OBJECT
   @brief An ELF object was found to be malformed.

   @var LNK_UNDEFINED_SYMBOLS
   @brief Can't find a definition in the object's search scope

   @var LNK_DEPENDENCY_CYCLE
   @brief There's a circular dependency among the loaded objects

   @var LNK_TOO_MANY_OBJECTS
   @brief Too many objects involved in this call to the linker

   @var LNK_TOO_MANY_DEPENDENCIES
   @brief Ran out of space for the dependency graph

   @var LNK_BAD_RELOCATION_TYPE
   @brief Didn't recognize the type of a relocation

   @var LNK_INTERNAL_ERROR1
   @brief Bad index into the object database

   @var LNK_INTERNAL_ERROR2
   @brief Bad index into the dependency graph
*/

/**
   @struct lnk_Status
   @brief Linker status code plus other information pertinent to an error.

   @var lnk_Status::status
   @brief The same status code returned via the status argument of lnk_load()

   @var lnk_Status::soname
   @brief If non-null, the soname of the object for which the error was detected

   @var lnk_Status::otherName
   @brief May be null or, depending on the error, the name of a symbol
   whose definition could not be found or the soname of another object
   involved in the error
*/
