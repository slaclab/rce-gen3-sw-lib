/*!@file     ethXaui.h
*
* @brief     Xaui Ethernet public declarations
*
* @author    S.Maldonado (smaldona@slac.stanford.edu)
*
* @date      October 30, 2014 -- Created
*
* $Revision: $
*
* @verbatim                    Copyright 2014
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#ifndef DRIVER_ETHXAUI_ETHERNET_H
#define DRIVER_ETHXAUI_ETHERNET_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct rtems_bsdnet_ifconfig;

int xaui_eth_attach_detach(struct rtems_bsdnet_ifconfig* cfg,
                           int                           dmy);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
