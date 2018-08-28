// -*-Mode: C++;-*-
/**
@file Base.hh
@brief 
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(SERVICE_DSLUTIL_BASE_H)
#define      SERVICE_DSLUTIL_BASE_H

#include <inttypes.h>

#if defined(__cplusplus)
extern "C" {
#endif

enum {OS_NOTSET  = 0,
      OS_RTEMS   = 1,
      OS_LINUX   = 2,
      OS_RESCUE  = 3,
      OS_UNKNOWN = 255};

enum {LOADBIT_NOTSET = 0xdeadbeef};
    
unsigned os_strtoint(const char*);

enum {DSLUTIL_MAX_SLOTS = 16};
enum {DSLUTIL_MAX_RCE_PER_COB = 17};
enum {DSLUTIL_MAX_BAY_PER_COB = 5};
enum {DSLUTIL_MAX_RCE_PER_BAY = 4};
enum {DSLUTIL_MAX_ID_STR = 32};
enum {DSLUTIL_MAX_MSG_SIZE = 256};

typedef struct DSL_slot_rce_state {
  uint32_t responded;
  uint32_t rces;  
  uint8_t  bay[DSLUTIL_MAX_RCE_PER_COB];
  uint8_t  rce[DSLUTIL_MAX_RCE_PER_COB];  
  uint8_t  os[DSLUTIL_MAX_RCE_PER_COB];
  uint32_t ip[DSLUTIL_MAX_RCE_PER_COB];
  char     id[DSLUTIL_MAX_RCE_PER_COB*DSLUTIL_MAX_ID_STR];
} DSL_slot_rce_state;

typedef struct DSL_slot_rce_status {
  uint16_t           nslots;
  uint16_t           slot_response;
  DSL_slot_rce_state state[DSLUTIL_MAX_SLOTS];
} DSL_slot_rce_status;     

typedef struct DSL_slot_update_state {
  uint32_t responded;
  uint32_t rces;  
  uint8_t  bay[DSLUTIL_MAX_RCE_PER_COB];
  uint8_t  rce[DSLUTIL_MAX_RCE_PER_COB];  
  uint8_t  os[DSLUTIL_MAX_RCE_PER_COB];
  uint32_t id[DSLUTIL_MAX_RCE_PER_COB];
  uint32_t status[DSLUTIL_MAX_RCE_PER_COB];
} DSL_slot_update_state;

typedef struct DSL_slot_update_status {
  uint16_t           nslots;
  uint16_t           slot_response;
  DSL_slot_update_state state[DSLUTIL_MAX_SLOTS];
} DSL_slot_update_status;     

enum {SLOT_ALL_WC             = 0xfeed,
      CMB_DPM_WC              = 0xf00f,
      CMB_DTM_WC              = 0x0f0f,
      CMB_ALL_WC              = 0xbaaa,
      ELEM_ALL_WC             = 0xbeef,
      UNDEFINED               = 0xffff};
  

#if defined(__cplusplus)
} // extern "C"
#endif
  
#endif
