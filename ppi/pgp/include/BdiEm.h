// -*-Mode: C;-*-
/**
@file
@brief This is the PUBLIC interface to the PGP BDIEM driver.

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
02/09/2017

@par Credits:
SLAC
*/
 
#ifndef BDIEM
#define BDIEM

#if defined(__cplusplus)
extern "C" {
#endif

/*
**
*/
  
uint32_t BDIEM_Open(uint32_t vc, 
                    uint32_t link,
                    uint32_t count, 
                    uint32_t size);

/*
**
*/

uint32_t BDIEM_IbFrame(SAS_Frame frame,
                       void *ctx,
                       SAS_IbMbx mbx);

/*
**
*/

void BDIEM_Process(uint32_t vc,
                   uint32_t link,
                   uint32_t flags,
                   uint32_t size,
                   void    *data);  

/*
**
*/
  
void BDIEM_Post(uint32_t vc, uint32_t link);

/*
**
*/  

void BDIEM_Dump(uint32_t vc, uint32_t link);

/*
**
*/  

void BDIEM_Reset(uint32_t vc, uint32_t link);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif

