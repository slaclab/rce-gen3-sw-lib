// -*-Mode: C;-*-
/**
@file
@brief This is the PUBLIC interface to the Xaui facility.

@verbatim
                               Copyright 2015
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
XAUI

@author
S.Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2014/08/19

@par Credits:
SLAC
*/

#ifndef XAUI
#define XAUI (9 <<  16)

#include <inttypes.h>
#include <sas/Sas.h>

/* @brief

As seen by the SAS services, This is the name of the XAUI plugin.

*/

#define XAUI_NAMEOF_PLUGIN ("Xaui")

#define XAUI_FRAME_TYPE    0
#define XAUI_HDR_SHIFT     2      /* required IP header alignment byte shift from start of ethernet header to dest MAC address */
#define XAUI_MAX_PAYLOAD   1500   /* max frame payload size, in bytes, allocated via Xaui_Alloc */
#define XAUI_MAX_BUFFERS   64     /* max count of device buffers */

typedef uint32_t (*Xaui_RxProcessor) (void* ctx, void* frame, uint32_t size);
typedef uint32_t (*Xaui_TxProcessor) (void* ctx);

/* @brief

These are the possible status values which are returned by Xaui functions 
described below...

*/

typedef enum {
 XAUI_SUCCESS        = 0,  // Zero is always success, non-zero an error...
 XAUI_INSFMEM        = 1 | XAUI,
 XAUI_NOSESSION      = 2 | XAUI,
 XAUI_NOMBX          = 3 | XAUI,
 XAUI_NOATTRS        = 4 | XAUI
 } Xaui_Status;
 
/* @brief

Ethernet frame header as delivered by the plug-in

*/

typedef struct __attribute__((__packed__)) {
  uint64_t dst;     // 64-bit destination address
  uint16_t src[3];  // 48-bit source address 
  uint16_t type;    // 16-bit Layer 3 (L3) packet
  uint8_t  data[];  // Start of frame data
} Xaui_Header;
 
/* @brief

Ethernet frame info as delivered by Xaui_Wait

*/

typedef struct {
  uint32_t     size;  // sizeof frame data
  void*        ctx;   // context argument from Xaui_RxBind
  Xaui_Header* frame; // frame data
} Xaui_Fd;

/*
**
*/

typedef struct {
  SAS_IbHandler     processor;
  void             *ctx;
} Xaui_Protocol;

/*
**
*/

typedef void* Xaui_RxDevice;
typedef void* Xaui_TxDevice;

#if defined(__cplusplus)
extern "C" {
#endif

Xaui_RxDevice Xaui_RxOpen(uint32_t numof_buffers);

Xaui_TxDevice Xaui_TxOpen(uint32_t numof_buffers);

Xaui_Fd *Xaui_Wait(Xaui_RxDevice);

Xaui_Header *Xaui_Alloc(Xaui_TxDevice, uint32_t size);

Xaui_Protocol *Xaui_Lookup(uint32_t protocol);

uint64_t Xaui_Mac();

void Xaui_Post(Xaui_TxDevice, Xaui_Header*, uint32_t size);

void Xaui_Free(Xaui_RxDevice reference, void *buf, uint32_t size);

void Xaui_TxBind(Xaui_TxDevice, Xaui_TxProcessor, void *ctx);

void Xaui_RxBind(uint32_t protocol, Xaui_RxDevice, Xaui_RxProcessor, void *ctx);

void Xaui_RxDebug(uint32_t level);

void Xaui_TxDebug(uint32_t level);

void Xaui_Bind(uint32_t protocol, SAS_IbHandler, void *ctx);

void Xaui_Dump();

void Xaui_Stats();

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
