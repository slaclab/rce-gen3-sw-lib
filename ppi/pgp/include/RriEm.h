// -*-Mode: C;-*-
/**
@file
@brief This is the PUBLIC interface to the PGP RRI Emulator driver (RRIEM).

@verbatim
                               Copyright 2017
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
02/10/2017

@par Credits:
SLAC
*/
 
#ifndef RRIEM
#define RRIEM

#include "pgp/Rri.h"

/* error codes */
#define RRIEM_ALLOC     1 // Error allocating page memory
#define RRIEM_INSTALL   2 // Error installing SVT
#define RRIEM_TRANSLATE 3 // Error in SVT lookup
#define RRIEM_OPEN      4 // Error opening device

/* RRIEM SVT table ID */
#define SVT_RRIEM_TABLE_NUM    27
#define SVT_RRIEM_TABLE        (1 << SVT_RRIEM_TABLE_NUM)

#if defined(__cplusplus)
extern "C" {
#endif

/** 

This is the SVT resident register default value table entry

*/

typedef struct {
  uint32_t    reg;    /*!< Register */
  uint32_t    val;    /*!< Default value */
  } RRIEM_Entry;
/** 

This is the SVT resident RRIEM configuration

*/

typedef struct {
  uint32_t    count;        /*!< Count of reg val entries */
  const RRIEM_Entry *entry; /*!< Array of reg val entries */
  } RRIEM_Cfg;

/*
**
*/

typedef void* RRIEM_Device;

/*
**
*/

typedef uint32_t (*RRIEM_Processor)(RRI_Result*);

/*
** Open a handle to the device for socket 0-2.
** These handles are singletons and cannot be multiply allocated.
** Calling with an in use socket value will result in a NULL device.
**
*/

RRIEM_Device RRIEM_Open(uint32_t socket, RRIEM_Cfg **cfg);

/*
** Bind a processor to register access
**
*/
    
void RRIEM_Bind(RRIEM_Device, uint32_t link, RRIEM_Processor, uint32_t reg);


/*
** Wait for change to RRIEM register space (blocking call)
**
*/
    
RRI_Result* RRIEM_Wait(RRIEM_Device);

/*
** Read a register value from the RRIEM
**
*/

uint32_t RRIEM_Read(RRIEM_Device, uint32_t link, uint32_t reg, uint32_t* value);

/*
** Write a register value in the RRIEM
**
*/

uint32_t RRIEM_Write(RRIEM_Device, uint32_t link, uint32_t reg, uint32_t value);

/*
** Set a bit in a register in the RRIEM
**
*/

uint32_t RRIEM_Bis(RRIEM_Device, uint32_t link, uint32_t reg, uint32_t mask);

/*
** Clear a bit in a register in the RRIEM
**
*/

uint32_t RRIEM_Bic(RRIEM_Device, uint32_t link, uint32_t reg, uint32_t mask);

/*
** Close the handle to the RRIEM device.
**
*/
  
void RRIEM_Close(RRIEM_Device);

/*
** Print the RRIEM statistics block per socket
**
*/
  
void RRIEM_Dump(uint32_t socket);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif

