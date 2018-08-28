// -*-Mode: C;-*-
/**
@file
@brief This is a PRIVATE interface to the Socket Abstraction Services (SAS) facility.  
This file provides constants, macros, and memory maps for private structures.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
SAS

@author
S. Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2014/06/30

@par Last commit:
\$Date: $ by \$Author: $.

@par Revision number:
\$Revision: $

@par Credits:
SLAC
*/

#ifndef SAS_TRANSACTION_H
#define SAS_TRANSACTION_H

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- *//*

  This file provides constants and macros for accessing structures used
  for inbound and outbound transactions.
  
  The macros included here support:
    Encoding instructions for the inbound engine.
    Encoding instructions for the outbound engine.
    Decoding values received from pending FIFOs.
    Decoding values received from free-list FIFOs.
    Decoding values received from mailbox FIFOs.
    Decoding values received from the transfer fault FIFO.
      
*//* ---------------------------------------------------------------------- */

/*
**
** These are the macros to build encode and decode macros.
**
*/

#define SAS_ENCODE(n,v,x) \
    (x |= ((v & n##_MASK)  << n##_OFFSET))


#define SAS_DECODE(n,v)\
    ((v >> n##_OFFSET)      & n##_MASK)

/*
**
** These are the bit offsets for fields in the 32-bit 
** inbound and outbound frame instructions.
** 
**
*/

#define FRAME_DESC_OFFSET                     0
#define FRAME_LEN_OFFSET                      18
#define FRAME_TYPE_OFFSET                     26

/*
**
** These are the bit masks for fields in the 32-bit 
** inbound and outbound frame instructions.
** 
**
*/

#define FRAME_DESC_MASK                       0x0003FFF0
#define FRAME_LEN_MASK                        0x000000FF
#define FRAME_TYPE_MASK                       0x0000000F

/*
**
** These are the bit offsets for fields in the 32-bit 
** outbound instruction.
** 
**
*/

#define OB_OPCODE_OFFSET                      30

/*
**
** These are the bit masks for fields in the 32-bit 
** outbound instruction.
** 
**
*/

#define OB_OPCODE_MASK                        0x00000003

/*
**
** These are the set of outbound instruction encode macros.
** 
**
*/

#define OB_INSTR_INIT(a)  \
    (a = 0)   

/*
**
** These are the bit offsets for fields in the 32-bit 
** inbound instruction.
** 
**
*/

#define IB_OPCODE_OFFSET                      28

/*
**
** These are the bit masks for fields in the 32-bit 
** inbound instruction.
** 
**
*/

#define IB_OPCODE_MASK                        0x0000000F

/*
**
** These are the set of outbound instruction encode macros.
** 
**
*/

#define IB_INSTR_INIT(a)  \
    (a &= FRAME_DESC_MASK)   

/*
**
** These are the bit offsets, masks, and macros
** for fields in the 32-bit input entry.
** 
**
*/

#define INPUT_EMPTY_OFFSET                    0
#define INPUT_EMPTY_MASK                      0x00001

/*
**
** These are the bit offsets for fields in the 32-bit 
** inbound pending list entry.
** 
**
*/

#define IB_ERROR_OFFSET                       30
#define IB_PAYLOAD_OFFSET                     31

/*
**
** These are the bit masks for fields in the 32-bit 
** inbound pending list entry.
** 
**
*/

#define IB_ERROR_MASK                         0x00001
#define IB_PAYLOAD_MASK                       0x00001

/*
**
** These are the bit offsets and masks for a 32-bit 
** free-list entry
** 
**
*/

#define FREELIST_VAL_OFFSET                   1

/*
**
** These are the bit masks for fields in the 32-bit 
** free-list entry.
** 
**
*/

#define FREELIST_VAL_MASK                     0xFFFFFFFF

/*
**
** These are the bit offsets for fields in the 32-bit 
** mailbox value.
** 
**
*/

#define MBX_INPUT_ERROR_OFFSET                1

/*
**
** These are the bit masks for fields in the 32-bit 
** mailbox instruction.
** 
**
*/

#define MBX_INPUT_ERROR_MASK                  0x00000001

/*
**
** These are the bit offsets for fields in the 32-bit 
** transfer fault list entry.
** 
**
*/

#define FAULT_ENGINE_OFFSET                   1
#define FAULT_SOCKET_OFFSET                   2
#define FAULT_READ_OFFSET                     6
#define FAULT_WRITE_OFFSET                    7
#define FAULT_TRANSFER_OFFSET                 8
#define FAULT_OVERFLOW_OFFSET                 9

/*
**
** These are the bit masks for fields in the 32-bit 
** transfer fault list entry.
** 
**
*/

#define FAULT_ENGINE_MASK                     0x1
#define FAULT_SOCKET_MASK                     0x3
#define FAULT_READ_MASK                       0x1
#define FAULT_WRITE_MASK                      0x1
#define FAULT_TRANSFER_MASK                   0x1
#define FAULT_OVERFLOW_MASK                   0x1

#endif /* SAS_TRANSACTION_H */
