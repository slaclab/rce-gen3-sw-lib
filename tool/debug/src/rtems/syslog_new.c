// -*-Mode: C;-*-
/**
@file
@brief Implement the syslog management for RTEMS.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2014/02/09

@par Last commit:
\$Date: 2014-07-31 17:05:57 -0700 (Thu, 31 Jul 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3597 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/debug/src/rtems/syslog_new.c $

@par Credits:
SLAC

Maintain a circular buffer of printout in memory, add time stamps to
the beginning of lines, etc. For our buffer we use all the memory in
the SYSLOG Region.

*/
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



#include "debug/print.h"

#include "debug/os/syslog.h"
#include "debug/os/SyslogTracker.h"

#include "memory/mem.h"

static dbg_SyslogTracker* dbg_tracker = 0;

static dbg_SyslogTracker dbg_trackerInstance;



void dbg_syslogPutcWithTimestamp(char c) {
  if (dbg_tracker) dbg_SyslogTracker_stamp(dbg_tracker, c);
}



void dbg_syslogPutcNoTimestamp(char c) {
  if (dbg_tracker) dbg_SyslogTracker_capture(dbg_tracker, c);
}



void dbg_initSyslog(void) {

  // Swallow the SYSLOG region whole.
  int   const buffsize  = mem_Region_size(MEM_REGION_SYSLOG);
  char* const buffaddr  = mem_Region_alloc(MEM_REGION_SYSLOG, buffsize);

  if (!buffaddr)
    dbg_bugcheck("dbg_initSyslog() couldn't allocate all of MEM_REGION_SYSLOG.\n");

  dbg_tracker = &dbg_trackerInstance;
  dbg_SyslogTracker_construct(dbg_tracker, buffaddr, buffsize);
}



void dbg_clearSyslog(void) {
  char* const buffaddr  = mem_Region_startAddress(MEM_REGION_SYSLOG);
  int   const buffsize  = mem_Region_size(MEM_REGION_SYSLOG);

  rtems_interrupt_level level;
  rtems_interrupt_disable(level);
  dbg_SyslogTracker_construct(dbg_tracker, buffaddr, buffsize);
  rtems_interrupt_enable(level);
}


// This is provided for use by the micro-shell in the fatal error handler.
void dbg_getSyslogState(char const **start, char const **next, char const **end, int *wrapped) {
  if (dbg_tracker) {
    *start   =                dbg_tracker->bufferAddress;
    *next    = *start +       dbg_tracker->inext;
    *end     = *start +       dbg_tracker->bufferSize;
    *wrapped = BUFFER_FULL == dbg_tracker->bufferState;
  }
  else {
    *start = *next = *end = NULL;
    *wrapped = 0;
  }
}



/* The state of chunk fetching. The first chunk is everything from the
   write pointer to the end. The second is everything before the write
   pointer.
*/
typedef enum {
  CHUNK_FIRST,    // advance() finds the first chunk (if any).
  CHUNK_SECOND,   // advance() finds the second chunk (if any).
  CHUNK_FINISHED  // advance() sets up an empty chunk.
} ChunkState;



/* Class dbg_SyslogChunker */



struct dbg_SyslogChunkerStruct {
  const char* bufferAddress;
  BufferState bufferState;
  int         bufferSize;
  int         inext; // Copy of the write pointer of the syslog buffer.

  const char* chunkAddress;
  int         chunkSize;
  ChunkState  chunkState;
};



dbg_SyslogChunker* dbg_SyslogChunker_constructWithTracker(dbg_SyslogTracker* tracker) {
  if (!tracker) return NULL;
  dbg_SyslogChunker* self = (dbg_SyslogChunker*)malloc(sizeof(dbg_SyslogChunker));
  if (self) {
    self->bufferAddress = tracker->bufferAddress;
    self->bufferState   = tracker->bufferState;
    self->bufferSize    = tracker->bufferSize;
    self->inext         = tracker->inext;
    self->chunkState    = CHUNK_FIRST;
    self->chunkAddress  = self->bufferAddress;
    self->chunkSize     = 0;
    dbg_SyslogChunker_advance(self);
  }
  return self;
}


dbg_SyslogChunker* dbg_SyslogChunker_construct(void){
  return dbg_SyslogChunker_constructWithTracker(dbg_tracker);
}


void dbg_SyslogChunker_destroy(dbg_SyslogChunker* self) {free(self);}


const char* dbg_SyslogChunker_chunkAddress(const dbg_SyslogChunker* self) {
  return self->chunkAddress;
}


int dbg_SyslogChunker_chunkSize(const dbg_SyslogChunker* self) {
  return self->chunkSize;
}


void dbg_SyslogChunker_advance(dbg_SyslogChunker* self) {
  switch(self->bufferState) {

  case BUFFER_EMPTY:
    self->chunkAddress = 0;
    self->chunkSize    = 0;
    self->chunkState   = CHUNK_FINISHED;
    break;

  case BUFFER_FILLING:
    // There is only one chunk since the write pointer hasn't wrapped around yet.
    if (self->chunkState == CHUNK_FIRST) {
      self->chunkAddress = self->bufferAddress;
      self->chunkSize    = self->inext;
      self->chunkState   = CHUNK_FINISHED;
    }
    else {
      self->chunkAddress = 0;
      self->chunkSize    = 0;
    }
    break;

  case BUFFER_FULL:
    // The first possible chunk is everything above inext and
    // the next is everything below.
    switch(self->chunkState) {
    case CHUNK_FIRST:
      if (self->inext == (self->bufferSize - 1)) {
        // Upper chunk is empty.
        self->chunkState = CHUNK_SECOND;
        dbg_SyslogChunker_advance(self);
      }
      else {
        // Return upper chunk.
        self->chunkAddress = self->bufferAddress + self->inext + 1;
        self->chunkSize    = self->bufferSize - self->inext - 1;
        self->chunkState   = CHUNK_SECOND;
      }
      break;
    case CHUNK_SECOND:
      if (self->inext == 0) {
        // Lower chunk is empty.
        self->chunkState = CHUNK_FINISHED;
        dbg_SyslogChunker_advance(self);
      }
      else {
        // Return lower chunk.
        self->chunkAddress = self->bufferAddress;
        self->chunkSize    = self->inext;
        self->chunkState   = CHUNK_FINISHED;
      }
      break;
    case CHUNK_FINISHED:
      self->chunkAddress = 0;
      self->chunkSize    = 0;
      break;
    } // switch(chunk state)
    break;


  } // switch(buffer state)
} // void advance()



/* Class dbg_SyslogTracker */



void dbg_SyslogTracker_construct(dbg_SyslogTracker* self, char* buffadd, int buffsize) {
  self->bufferAddress = buffadd;
  self->bufferSize    = buffsize;
  self->inext         = 0;
  self->bufferState   = BUFFER_EMPTY;
  self->lineState     = LINE_EMPTY;
  buffadd[0]          = '\0';
}


void dbg_SyslogTracker_destroy(dbg_SyslogTracker* self) {}


void dbg_SyslogTracker_capture(dbg_SyslogTracker* self, char ch) {
  self->bufferAddress[self->inext] = ch;
  switch(self->bufferState) {
  case BUFFER_EMPTY:
    self->inext += 1;
    self->bufferState = BUFFER_FILLING;
    break;
  case BUFFER_FILLING:
    self->inext += 1;
    if (self->inext == self->bufferSize) {
      self->inext = 0;
      self->bufferState = BUFFER_FULL;
    }
    break;
  case BUFFER_FULL:
    self->inext += 1;
    if (self->inext == self->bufferSize) self->inext = 0;
    break;
  }
  self->bufferAddress[self->inext] = '\0';
}


void dbg_SyslogTracker_stamp(dbg_SyslogTracker* self, char ch) {
  switch(self->lineState) {
  case LINE_EMPTY:
    if (ch != '\n') { // New, non-empty line.
      self->lineState = LINE_NONEMPTY;
      // Most library routines are not callable from exception handlers.
      if (dbg_fatalNestingLevel() == 0) {
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        struct tm result;
        localtime_r(&now.tv_sec, &result);
        char buf[100];
        size_t nt = strftime(buf, sizeof buf, "%Y/%m/%d %H:%M:%S", &result);
        nt += sprintf(buf + nt, ".%06ld: ", (now.tv_nsec + 500L) / 1000L);
        size_t i;
        for (i = 0; i < nt; ++i) dbg_SyslogTracker_capture(self, buf[i]);
      }
    }
    break;
  case LINE_NONEMPTY:
    if (ch == '\n') self->lineState = LINE_EMPTY;
    break;
  }
  dbg_SyslogTracker_capture(self, ch);
}
