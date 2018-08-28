// -*-Mode: C;-*-
/**
@file
@brief Declare the dependency graph object for the dynamic linker.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(LNK_SEARCHDAG_H)
#define      LNK_SEARCHDAG_H


#include "elf/lnkStatus.h"

#include "lnkConstants.h"

/* Represents an ELF object to search for symbol definitions. */
typedef struct {
  ObjIndex  dbIndex;    /* The index of the object in the object database. */
  DagLink   sib;        /* Pointer to the next search node with the same parent. */
  DagLink   child;      /* Pointer to the first child of this node. */
  DagLink   lastChild;  /* Used only during DAG construction, for efficiency. */
} lnk_SearchNode;

static inline void lnk_SearchNode_init(lnk_SearchNode* this, int dbIndex) {
  this->dbIndex   = dbIndex;
  this->sib       = 0;
  this->child     = 0;
  this->lastChild = 0;
}


/* Storage for the search DAG. 0 is the null link. */
typedef struct {
  unsigned next; /* The next free node slot. */
  int16_t  visitLevel;
  lnk_SearchNode nodes[1 + LNK_MAX_DAG_NODES];
} lnk_SearchDag;

/* Create the root node of the DAG. */
void lnk_SearchDag_init(lnk_SearchDag* this, int rootDbIndex);

/* Given a link get the addrress of the search node. */
lnk_SearchNode* lnk_SearchDag_get(lnk_SearchDag* this, int link, lnk_Status* status);

/* Add a child node to a parent whose link is given. return the link of the new node. */
int lnk_SearchDag_addChild(lnk_SearchDag* this, int parentLink, int childDbIndex, lnk_Status* status);

/* Increment and return the current search number. */
static inline int lnk_SearchDag_incVisitLevel(lnk_SearchDag* this) {return ++this->visitLevel;}

#endif
