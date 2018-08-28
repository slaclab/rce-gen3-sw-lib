// -*-Mode: C;-*-
/**
@file
@brief Miscellaneous constants and related types used by the dynamic linker.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(LNK_CONSTANTS_H)
#define      LNK_CONSTANTS_H

#include <stdint.h>




typedef enum {
  LNK_DAG_NOT_STARTED,         /* No DAG yet started for the object. */
  LNK_DAG_UNDER_CONSTRUCTION,  /* Dependency info is being collected. */
  LNK_DAG_DONE                 /* DAG completed, search scope constructed. */
} lnk_DagState;

enum {
  LNK_MAX_ELF_OBJECTS   = 32,
  LNK_MAX_DAG_NODES     = 255,
  LNK_MAX_SONAME_LENGTH = 128
};

typedef uint32_t ObjSet;    /* uintNN_t where NN == LNK_MAX_ELF_OBJECTS. */

typedef uint8_t  DagLink;   /* NN = log2(LNK_MAX_DAG_NODES + 1). */

typedef int8_t   ObjIndex;  /* NN >= 1 + log2(LNK_MAX_ELF_OBJECTS). */

typedef uint8_t  ObjCount;

typedef uint8_t  bool;

enum {FALSE, TRUE};

#endif
