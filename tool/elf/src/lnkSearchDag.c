// -*-Mode: C;-*-
/**
@file
@brief Implement the ELF object dependency graph for the dynamic linker.
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
\$Date: 2014-04-07 21:30:01 -0700 (Mon, 07 Apr 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3125 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/elf/src/lnkSearchDag.c $

@par Credits:
SLAC
*/
#include <stdio.h> /* For NULL. */


#include "elf/lnkStatus.h"

#include "lnkConstants.h"
#include "lnkSearchDag.h"

void lnk_SearchDag_init(lnk_SearchDag* self, int rootDbIndex) {
  lnk_SearchNode_init(self->nodes + 1, rootDbIndex);
  self->next = 2;
}

lnk_SearchNode* lnk_SearchDag_get(lnk_SearchDag* self, int link, lnk_Status* status) {
  lnk_Status_init1(status, LNK_OK);
  if (link > 0 && link <= LNK_MAX_DAG_NODES) return self->nodes + link;
  lnk_Status_init1(status, LNK_INTERNAL_ERROR2);
  return NULL;
}

int lnk_SearchDag_addChild(lnk_SearchDag* self, int parentLink, int childDbIndex, lnk_Status* status) {
  lnk_SearchNode* child;
  lnk_SearchNode* parent = lnk_SearchDag_get(self, parentLink, status);
  if (status->status != LNK_OK) return 0;
  if (self->next > LNK_MAX_DAG_NODES) {
    lnk_Status_init1(status, LNK_TOO_MANY_DEPENDENCIES);
    return 0;
  }
  child = self->nodes + self->next++;
  lnk_SearchNode_init(child, childDbIndex);
  if (!parent->lastChild) {
    parent->child = parent->lastChild = self->next - 1;
  }
  else {
    lnk_SearchNode* sib = lnk_SearchDag_get(self, parent->lastChild, status);
    if (status->status != LNK_OK) return 0;
    sib->sib = parent->lastChild = self->next - 1;
  }
  return parent->lastChild;
}

