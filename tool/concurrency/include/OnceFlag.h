// -*-Mode: C;-*-
/**
@file
@brief Declare and implement the OnceFlag type.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#if !defined(TOOL_CONCURRENCY_ONCEFLAG_H)
#define      TOOL_CONCURRENCY_ONCEFLAG_H

#include <stdbool.h>

typedef struct {
  /** @privatesection */
  unsigned volatile notdone;

} OnceFlag;

#define ONCE_FLAG_INITIALIZER {0}

#if defined(__cplusplus)
extern "C" {
#endif

/** @public @memberof OnceFlag */
static inline
bool OnceFlag_testAndSet(OnceFlag* flag) {return __sync_bool_compare_and_swap(flag->notdone, 0, 1);}


/** @public @memberof OnceFlag */
static inline
void OnceFlag_init(OnceFlag* flag) {
  OnceFlag const f = ONCE_FLAG_INITIALIZER;
  *flag = f;
}

#if defined(__cplusplus)
} // extern "C"
#endif


#endif
