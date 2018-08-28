// -*-Mode: C++;-*-
/*!@file     init.hh
*
* @brief     Initialization code for the COB Mezzanine Board.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      August 25, 2012 -- Created
*
* $Revision: 2961 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef SERVICE_CMB_INIT_HH
#define SERVICE_CMB_INIT_HH



namespace service {

  namespace cmb {

    inline void initialize();

    inline void bootMark(unsigned value);

    inline void booted();

  } // cmb

} // service

#include "cmb/gen/init-inl.hh"

#endif
