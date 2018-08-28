/*!@file     Ethernet.hh
*
* @brief     Ethernet public declarations
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      July 10, 2012 -- Created
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
#ifndef DRIVER_ETHPIO_ETHERNET_HH
#define DRIVER_ETHPIO_ETHERNET_HH

extern "C" {
#include <rtems/rtems_bsdnet.h>
}



namespace driver {

  namespace ethPio {

    extern "C" int ethPio_driver_attach(struct rtems_bsdnet_ifconfig* cfg,
                                        int                           dmy);

  }

}

#endif
