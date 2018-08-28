// -*-Mode: C;-*-
/**
@file
@brief Declaration of the ELF object database for the dynamic linker.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(LNK_OBJDB_H)
#define      LNK_OBJDB_H


#include "elf/Object.h"

#include "lnkConstants.h"

/* Describes an object seen in this run of the linker. */
typedef struct {
  elf_Object    elfObject; /* Pointers into the ELF object and other stuff.*/
  const char*   soname;    /* The soname of the object. */
  lnk_DagState  dagState;  /* State of the dependecy graph below this object. */
  DagLink       dag;       /* Link to the first child in the dependency graph. */
  void*         prefsOverride; /* if non-null, the preferences pointer to use. */
  bool          bound;     /* Have symbolic references been bound? */
  bool          installable; /* Is object installable? */
} lnk_ObjInfo;


/* All objects seen in this run of the linker. -1 is the null index. */
typedef struct {
  lnk_ObjInfo info[LNK_MAX_ELF_OBJECTS];
  ObjIndex next;  /* Next free info slot. */
} lnk_ObjDb;



void lnk_ObjDb_init(lnk_ObjDb* this);

/* Unconditionally add a new object. Return its DB index. */
int lnk_ObjDb_add
(lnk_ObjDb* this, const char* objAddr, int onPath, int bound, const char* soname, lnk_Status* status);

/* Add a needed object, possibly loading it first. Return its DB index. */
int lnk_ObjDb_addNeeded(lnk_ObjDb* this, const char* neededSoname, lnk_Status* status);

int lnk_ObjDb_lookup(const lnk_ObjDb* this, const char* objAddr);

lnk_ObjInfo* lnk_ObjDb_get(lnk_ObjDb* this, int index, lnk_Status* status);
#endif
