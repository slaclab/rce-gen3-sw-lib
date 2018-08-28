// -*-Mode: C;-*-
/**
@file
@brief Return codes and status struct for the dynamic linker.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(LINKER_STATUS_H)
#define      LINKER_STATUS_H



#include "system/statusCode.h"

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h> /* For NULL. */

/** @cond development */
#define LNK 4 /* Facility no., see configuration/system/facility.doc.*/
/** @endcond */

typedef enum {
  LNK_OK                    = STS_K_SUCCESS,
  LNK_SUCCESS               = STS_K_SUCCESS,
  LNK_BAD_OBJECT            = STS_ENCODE(LNK, 1),
  LNK_UNDEFINED_SYMBOLS     = STS_ENCODE(LNK, 2),
  LNK_DEPENDENCY_CYCLE      = STS_ENCODE(LNK, 3),
  LNK_TOO_MANY_OBJECTS      = STS_ENCODE(LNK, 4),
  LNK_TOO_MANY_DEPENDENCIES = STS_ENCODE(LNK, 5),
  LNK_BAD_RELOCATION_TYPE   = STS_ENCODE(LNK, 7),
  LNK_INTERNAL_ERROR1       = STS_ENCODE(LNK, 8),
  LNK_INTERNAL_ERROR2       = STS_ENCODE(LNK, 9),
} lnk_StatusCode;
#undef LNK

typedef struct {
  int         status;
  const char* soname;
  const char* otherName;
} lnk_Status;

/** @cond development */
static inline void lnk_Status_init3
(lnk_Status* t, int status, const char* const soname, const char* const otherName) {
  t->status = status;
  t->soname = soname;
  t->otherName = otherName;
}

static inline void lnk_Status_init1(lnk_Status* t, int status)
{lnk_Status_init3(t, status, NULL, NULL);}

static inline void lnk_Status_init2
(lnk_Status* t, int status, const char* const soname)
{lnk_Status_init3(t, status, soname, NULL);}

/** @endcond */

#if defined(__cplusplus)
}
#endif

#endif
