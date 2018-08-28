// -*-Mode: C;-*-
/**
@file
@brief Implement the ELF object database for the dynamic linker.
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
2013/11/25

@par Last commit:
\$Date: 2014-06-10 15:41:51 -0700 (Tue, 10 Jun 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3382 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/src/lnkObjDb.c $

@par Credits:
SLAC
*/
#include <stdio.h> /* For NULL. */


#include "elf/Object.h"
#include "elf/lnkStatus.h"
#include "ldr/Ldr.h"
#include "svt/Svt.h"

#include "exidx.h"
#include "lnkConstants.h"
#include "lnkObjDb.h"


void lnk_ObjDb_init(lnk_ObjDb* this) {this->next = 0;}

/* Unconditionally add a new object. Return its DB index. */
int lnk_ObjDb_add
(lnk_ObjDb* this, const char* objAddr, int dagState, int bound, const char* soname, lnk_Status* status) {
  if (this->next >= LNK_MAX_ELF_OBJECTS) {
    lnk_Status_init1(status, LNK_TOO_MANY_OBJECTS);
    return -1;
  }
  { lnk_ObjInfo* const info = this->info + this->next;
    unsigned const elfstatus = elf_Object_init(&info->elfObject, objAddr);
    if (LNK_OK != elfstatus) {
      lnk_Status_init1(status, elfstatus);
      return -1;
    }
    info->soname        = soname;
    info->dagState      = dagState;
    info->dag           = 0;
    info->prefsOverride = NULL;
    info->bound         = bound;
    info->installable   = TRUE;
  }
  lnk_Status_init1(status, LNK_OK);
  return this->next++;
}

/* Add a needed object, possibly loading it first. Return its DB index. */
int lnk_ObjDb_addNeeded(lnk_ObjDb* this, const char* neededSoname, lnk_Status* status) {
  const char* addr;
  bool        loadNow;
  bool        known;
  int         index;
  lnk_Status_init1(status, LNK_OK);
  /* Is the object already installed? */
  addr = (char*)Ldr_Lookup(neededSoname);
  loadNow = !addr;
  /* Have we encountered this object before during this link? Always false if addr is NULL. */
  index = lnk_ObjDb_lookup(this, addr);
  known   = (index >= 0);
  if (!known) {
    if (!loadNow) {
      /* Previously installed object which we'll assume is already
         bound and initialized.  We still need to trace dependencies
         since that info is not saved between links. The needed-list
         soname is guaranteed to be identical to the soname embedded
         in the object.
      */
      return lnk_ObjDb_add(this, addr, LNK_DAG_NOT_STARTED, TRUE /*bound*/, neededSoname, status);
    }
    else {
      /* Object that needs to be loaded and so will need to be bound and initialized afterward. */
      Ldr_status  ldrstat;
      Ldr_elf*    elfaddr;
      elfaddr  = Ldr_Load(neededSoname, &ldrstat);
      if (!elfaddr) {
        lnk_Status_init2(status, ldrstat, neededSoname);
        return -1;
      }
      index = lnk_ObjDb_add(this, (const char*)elfaddr, LNK_DAG_NOT_STARTED, FALSE, neededSoname, status);

      /* Remember where to find the EXIDX table for the newly loaded object. */
      rememberExidx(&this->info[index].elfObject);

      return index;
    } 
  } /* if (!known) ... */
  /* At this point the object is always known, i.e., recorded in objDb. */
  return index;
}

int lnk_ObjDb_lookup(const lnk_ObjDb* this, const char* objAddr) {
  int i;
  for (i = 0; i < this->next; ++i) {
    if (objAddr == (const char*)this->info[i].elfObject.header) return i;
  }
  return -1;
}

lnk_ObjInfo* lnk_ObjDb_get(lnk_ObjDb* this, int index, lnk_Status* status) {
  lnk_Status_init1(status, LNK_OK);
  if (index >= 0 && index < this->next) return this->info + index;
  lnk_Status_init1(status, LNK_INTERNAL_ERROR1);
  return NULL;
}
