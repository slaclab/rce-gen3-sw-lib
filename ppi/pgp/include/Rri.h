// -*-Mode: C;-*-
/**
@file
@brief This is the PUBLIC interface to the PGP RRI driver.

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
 
#ifndef RRI
#define RRI

#include <inttypes.h>
#include <stddef.h>

#define RRI_GET ((uint32_t)0)  // Read a register
#define RRI_PUT ((uint32_t)1)  // Write a register
#define RRI_BIS ((uint32_t)2)  // Set bits of a register
#define RRI_BIC ((uint32_t)3)  // Clear bits of a register

/*
** "Address", "Fault" and "Opcode" are embedded within an "operation" value as three different bit-fields. 
** This is the field definition (offset and length) of its ADDRESS field. 
*/

#define RRI_ADDRESS_OFFSET ((uint32_t)0)
#define RRI_ADDRESS_LENGTH ((uint32_t)24)
#define RRI_ADDRESS_MASK   ((1 << RRI_ADDRESS_LENGTH) - 1)

/*
** "Address", "Fault" and "Opcode" are embedded within an "operation" value as three different bit-fields. 
** This is the field definition (offset and length) of its FAULT field. 
*/

#define RRI_FAULT_OFFSET ((uint32_t)24)
#define RRI_FAULT_LENGTH ((uint32_t)2)
#define RRI_FAULT_MASK   ((1 << RRI_FAULT_LENGTH) - 1)

/*
** "Address", "Fault" and "Opcode" are embedded within an "operation" value as three different bit-fields. 
** This is the field definition (offset and length) of its OPCODE field. 
*/

#define RRI_OPCODE_OFFSET ((uint32_t)30)
#define RRI_OPCODE_LENGTH ((uint32_t)2)
#define RRI_OPCODE_MASK   ((1 << RRI_OPCODE_LENGTH) - 1)

/*
** These three macros extract (decode) the three different bit-fields of an "operation" value
*/

#define RRI_OPCODE(operation)  ((operation >> RRI_OPCODE_OFFSET)  & RRI_OPCODE_MASK) 
#define RRI_FAULT(operation)   ((operation >> RRI_FAULT_OFFSET)   & RRI_FAULT_MASK)  
#define RRI_ADDRESS(operation) ((operation >> RRI_ADDRESS_OFFSET) & RRI_ADDRESS_MASK)  

/*
** This macro encodes "opcode", "address" and assumes fault is zero (0) into a single "operation" value...
*/ 

#define RRI_ENCODE2(opcode, address) (((RRI_OPCODE_MASK & opcode) << RRI_OPCODE_OFFSET) | ((RRI_ADDRESS_MASK & address) << RRI_ADDRESS_OFFSET))

/*
** This macro encodes "opcode", "address" and "fault" into a single "operation" value...
*/ 

#define RRI_ENCODE3(opcode, address, fault) (((RRI_OPCODE_MASK & opcode) << RRI_OPCODE_OFFSET) | ((RRI_ADDRESS_MASK & address) << RRI_ADDRESS_OFFSET) | ((RRI_FAULT_MASK & fault) << RRI_FAULT_OFFSET))

/*
** Three, single bit fields are embedded within an "fault" value. These are its two different offsets.. 
*/

#define RRI_INVADDRESS_OFFSET ((uint32_t)0)
#define RRI_TIMEOUT_OFFSET    ((uint32_t)1)

/*
** These three macros extract the two different errors (potentially) contained within a "fault" field...
*/

#define RRI_INVADDRESS(fault) ((fault >> RRI_INVADDRESS_OFFSET) & 0x1)
#define RRI_TIMEOUT(fault)    ((fault >> RRI_TIMEOUT_OFFSET) & 0x1)

/*
**
*/
  
typedef struct {
  uint32_t operation; // Contains opcode and address (see above)
  uint32_t operand;   // Value and interpretation depend on opcode
} RRI_Instruction;

/*
**
*/
  
typedef struct {
  RRI_Instruction instr; // Return instruction
  uint32_t        link;  // Source link
  uint32_t        tid;   // Transaction ID
} RRI_Result;

/*
**
*/

typedef void* RRI_Device;

/*
**
*/

typedef void* RRI_Arg;

/*
**
*/

typedef int (*RRI_Processor)(RRI_Result*, RRI_Arg);

#if defined(__cplusplus)
extern "C" {
#endif

/*
**
*/
  
RRI_Device RRI_Open(uint32_t socket);

/*
**
*/
  
void RRI_Bind(RRI_Device, RRI_Processor, RRI_Arg);

/*
**
*/

uint32_t RRI_Read(RRI_Device, uint32_t link, uint32_t address, uint32_t* value);

/*
**
*/

uint32_t RRI_Write(RRI_Device, uint32_t link, uint32_t address, uint32_t value);

/*
**
*/

uint32_t RRI_Bis(RRI_Device, uint32_t link, uint32_t address, uint32_t mask);

/*
**
*/

uint32_t RRI_Bic(RRI_Device, uint32_t link, uint32_t address, uint32_t mask);

/*
**
*/
  
void RRI_Post(RRI_Device, uint32_t link, uint32_t operation, uint32_t operand, uint32_t tid);

/*
**
*/
  
void RRI_PostVc(RRI_Device reference, 
                uint32_t link, 
                uint32_t operation, 
                uint32_t operand, 
                uint32_t tid,
                uint32_t vc);

/*
**
*/
    
RRI_Result* RRI_Wait(RRI_Device);

/*
**
*/
  
void RRI_Close(RRI_Device);

/*
**
*/  

void RRI_Dump(RRI_Device);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif

