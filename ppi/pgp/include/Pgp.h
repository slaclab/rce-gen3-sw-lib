// -*-Mode: C;-*-
/**
@file
@brief This is the PUBLIC interface to the PGP Plugin driver.

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
 
#ifndef PGP
#define PGP

#include <inttypes.h>
#include <stddef.h>

#include "sas/Sas.h"

#define PGP0_PLUGIN_NAME "Pgp0"
#define PGP1_PLUGIN_NAME "Pgp1"
#define PGP2_PLUGIN_NAME "Pgp2"

#define PGP_SUCCESS     0
#define PGP_INSFMEM     1
#define PGP_NO_SESSION  2
#define PGP_NO_IBMBX    3
#define PGP_NO_ATTRS    4

#define PGP_NUMOF_PLUGINS 3
#define PGP_NUMOF_LANES   4
#define PGP_NUMOF_VCS     4
#define PGP_NUMOF_LINKS   12

#define PGP_SOF        (1<<1)  // User start of frame         

#define PGP_OVF        (1<<0)  // Control inbound overflow    
#define PGP_HDR        (1<<1)  // Control header only frame   
#define PGP_EOF        (1<<2)  // Control end of frame        

#define PGP_DECODE_LANE(link) (link & 0x3)
#define PGP_DECODE_SOCK(link) (link >> 2 )
#define PGP_ENCODE_LINK(sock,lane) ((sock << 2) | (lane & 0x3))

/* @brief

PGP frame header as delivered by the plug-in

*/

typedef struct {
  uint8_t     dst;   // destination virtual channel
  uint8_t     fuser; // first user word
  uint8_t     luser; // last user word
  uint8_t     ctl;   // control word
  uint32_t    size;  // data length in bytes, mbz on transmit
} Pgp_Header;

/* @brief

PGP frame as transmitted to the plug-in

*/

typedef struct {
  Pgp_Header  hdr;
  uint8_t     payload[];
} Pgp_Frame;

/*
**
*/

typedef SAS_IbOpcode (*PGP_Processor)(SAS_Frame, SAS_IbMbx, uint32_t arg);

#if defined(__cplusplus)
extern "C" {
#endif

/*
**
*/

void Pgp_Bind(PGP_Processor processor, uint32_t vc, uint32_t arg);

/*
**
*/

void Pgp_Unbind(uint32_t vc);

/*
**
*/

void Pgp_Dump(uint32_t link);

/*
**
*/

int Pgp_LinkIsReady(uint32_t link);

/*
**
*/

uint32_t Pgp_GetRemoteLinkData(uint32_t link);

/*
**
*/

uint32_t Pgp_GetSideband(uint32_t link);

/*
**
*/

void Pgp_SetSideband(uint32_t link, uint32_t value);

/*
**
*/

void Pgp_SetLoopback(uint32_t link, uint32_t value);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
