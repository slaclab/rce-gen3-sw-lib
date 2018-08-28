// -*-Mode: C;-*-
/**
@file
@brief This is the PUBLIC interface to the PGP BDIEM test driver.

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
03/21/2017

@par Credits:
SLAC
*/
 
#ifndef BDIEM_TEST
#define BDIEM_TEST

#include "xaui/Xaui.h"

#define BDIEM_UDP_PORT     9930
#define BDIEM_UDP_MAGIC    0x11223344

#define BDIEM_FILENAME_STRLEN 256

typedef struct
  {
  uint32_t magic;
  uint32_t link;
  uint32_t vc;
  char filename[BDIEM_FILENAME_STRLEN];
  } BDIEM_Header;

/*
**
*/
    
typedef struct  __attribute__((__packed__)) {
  uint8_t   version;
  uint8_t   tos;
  uint16_t  length; 
  uint16_t  id;
  uint16_t  offset;
  uint8_t   ttl;
  uint8_t   protocol;
  uint16_t  checksum;
  uint32_t  src;
  uint32_t  dst;
  } IpHeader;

/*
**
*/
  
typedef struct {
  uint16_t src;
  uint16_t dst;
  uint16_t length; 
  uint16_t checksum;
  } UdpHeader;
  
/*
**
*/

typedef struct {
 Xaui_Header ethernet;
 IpHeader    ip;
 UdpHeader   udp;
 Pgp_Header  pgp;
 } UdpPacket;


#if defined(__cplusplus)
extern "C" {
#endif

/*
**
*/  

int BDIEMT_Vc(uint32_t vc);

/*
**
*/  

int BDIEMT_Ip(char *ip);

/*
**
*/
  
int BDIEMT_File(char *filename, uint32_t link);

/*
**
*/
  
int BDIEMT_Open(uint32_t link);

/*
**
*/
  
int BDIEMT_Trigger(uint32_t link);

/*
**
*/

int BDIEMT_Remote(uint32_t link);

/*
**
*/

int BDIEMT_Local(uint32_t link);

/*
**
*/
  
void BDIEMT_Dump(uint32_t link);

/*
**
*/
  
int BDIEMT_Reset(uint32_t link);

/*
**
*/
  
void BDIEMT_Close(uint32_t link);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif

