// -*-Mode: C;-*-
/**
@file
@brief This is the PUBLIC interface to the PGP BDI driver.

@verbatim
                               Copyright 2015
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
PGP

@author
Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2015/11/09

@par Credits:
SLAC
*/
 
#ifndef BDI
#define BDI

#include <inttypes.h>
#include <stddef.h>

#define BDI_SUCCESS      0
#define BDI_INSFMEM      1
#define BDI_RSERR        2
#define BDI_LNKERR       3
#define BDI_SASERR       4
#define BDI_VCERR        5
#define BDI_TASKERR      6
#define BDI_SVTERR       7

#define BDI_FIRST       (1<<0) /* first page in sequence */
#define BDI_LAST        (1<<1) /* last page in sequence */
#define BDI_SEQ         (1<<2) /* error in page sequence */
#define BDI_OVERFLOW    (1<<3) /* page contains overflow bytes */

/*
**
*/

typedef void* BDI_Arg;

/*
**
*/
  
typedef struct {
  uint8_t  vc;       // source virtual channel
  uint8_t  link;     // source link
  uint16_t status;   // first/last/seq/error bits
  uint32_t seq;      // page sequence value
  uint32_t size;     // total size of page data in bytes
  uint8_t  *data;    // page data storage
} BDI_Page;


/*
**
*/

typedef int (*BDI_Processor)(BDI_Page*, BDI_Arg);

#if defined(__cplusplus)
extern "C" {
#endif

/*
**
*/
  
uint32_t BDI_Open(uint32_t vc, 
                  uint32_t link,
                  uint32_t count, 
                  uint32_t size, 
                  uint32_t cached);

/*
**
*/
  
void BDI_Bind(uint32_t vc, uint32_t link, BDI_Processor, BDI_Arg arg);

/*
**
*/
    
BDI_Page* BDI_Wait(uint32_t vc, uint32_t link);

/*
**
*/
  
void BDI_Free(BDI_Page*);

/*
**
*/
  
void BDI_Close(void);

/*
**
*/  

void BDI_Dump(uint32_t vc, uint32_t link);

/*
**
*/  

void BDI_Reset(uint32_t vc, uint32_t link);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif

