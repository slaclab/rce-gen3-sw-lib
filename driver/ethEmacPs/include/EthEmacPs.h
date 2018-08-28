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
#ifndef DRIVER_ETHEMACPS_ETHERNET_H
#define DRIVER_ETHEMACPS_ETHERNET_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct rtems_bsdnet_ifconfig;

int zynq_eth_attach_detach(struct rtems_bsdnet_ifconfig* cfg,
                           int                           dmy);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
