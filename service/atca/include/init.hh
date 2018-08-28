// -*-Mode: C++;-*-
/*!@file     init.hh
*
* @brief     Initialization code for launching the ATCA server.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      July 3, 2013 -- Created
*
* $Revision: 2961 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef SERVICE_ATCA_INIT_HH
#define SERVICE_ATCA_INIT_HH

#if defined(__cplusplus)
extern "C" {
#endif

namespace service {

  namespace atca {

    void initialize();
  }

}

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
