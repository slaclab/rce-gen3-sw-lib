// -*-Mode: C;-*-
/**
@file
@brief Low-level hardware mutex.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(TOOL_CONCURRENCY_BASIC_MUTEX_H)
#define      TOOL_CONCURRENCY_BASIC_MUTEX_H

#include "system/exclusion.h"


typedef EXCLUSION_ALIGNED struct BasicMutex BasicMutex;

struct BasicMutex {
  /** @privatesection */
  volatile unsigned lock;
  char filler[EXCLUSION_GRANULARITY - sizeof(unsigned)];
};


/** @public @memberof BasicMutex */
static inline unsigned BasicMutex_trylock(BasicMutex *mutex) {
  return !__sync_lock_test_and_set(&mutex->lock, 1);
}


/** @public @memberof BasicMutex */
static inline void BasicMutex_unlock(BasicMutex *mutex) {
  __sync_lock_release(&mutex->lock);
}

#define BASIC_MUTEX_INITIALIZER {0}

/** @public @memberof BasicMutex */
static inline void BasicMutex_init(BasicMutex *mutex) {
  static const BasicMutex init = BASIC_MUTEX_INITIALIZER;
  *mutex = init;
}
#endif
