/*!@file     sd.h
*
* @brief     SD driver parameters
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      June 25, 2013 -- Created
*
* $Revision: 2269 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef CONFIGURATION_SYSTEM_GEN3_SD_H
#define CONFIGURATION_SYSTEM_GEN3_SD_H


#include "sd/mmc.h"
#include "sd/sdhci.h"
#include "xilinxcfg/xparameters.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SDHCI_CONFIG_TABLE                                              \
  {                                                                     \
    XPAR_PS7_SD_0_S_AXI_BASEADDR,                  /* .ioaddr     */    \
    XPAR_XSDIOPS_0_INTR,                           /* .irqno      */    \
    MMC_MODE_HC,                                   /* .flags      */    \
    SDHCI_QUIRK_NO_CD /*| SDHCI_QUIRK_WAIT_SEND_CMD*/, /* .quirks */    \
    52000000,                                /* .max_clk    */    \
    52000000 >> 9,                           /* .min_clk    */    \
    false,                                         /* .verbose    */    \
  }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
