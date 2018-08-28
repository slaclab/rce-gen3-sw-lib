// -*-Mode: C++;-*-
/*!@file     configure.hh
*
* @brief     Initialization code for starting networking.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      June 26, 2012 -- Created
*
* $Revision: 3820 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef DRIVER_NETWORK_CONFIGURE_HH
#define DRIVER_NETWORK_CONFIGURE_HH



#include "exception/Exception.hh"
#include "network/BsdNet_Config.h"

namespace driver {

  namespace network {

    class EthernetConfig;

    void configure(BsdNet_Attributes *prefs, unsigned iface, void *attach)
      throw (tool::exception::Error);

    void configure(BsdNet_Attributes *prefs, const EthernetConfig& ethConfig, unsigned iface)
      throw (tool::exception::Error);
  }

}

#endif
