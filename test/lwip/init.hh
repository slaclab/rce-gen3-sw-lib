// -*-Mode: C++;-*-
/*!@file     init.hh
*
* @brief     Initialization code for starting LWIP networking.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      December 9, 2013 -- Created
*
* $Revision: 2912 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TEST_LWIP_INIT_HH
#define TEST_LWIP_INIT_HH

#include "datCode.hh"
#include DAT_PUBLIC(tool,  exception, Exception.hh)

namespace test {

  namespace lwip {

    void initialize()
      throw (tool::exception::Error);

  } // lwip

} // test

#endif
