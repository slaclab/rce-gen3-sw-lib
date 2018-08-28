// -*-Mode: C;-*-
/**
* @file      sdhci_register.c
*
* @brief     Register the SD driver given a configuration.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      July 30, 2013 -- Created
*
* $Revision: 2116 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <rtems.h>


#include "system/gen/sd.h"
#include "sd/mmc.h"
#include "sd/sdhci.h"


const rtems_sdhci_config rtems_sdhci_configuration[]    = { SDHCI_CONFIG_TABLE };
size_t                   rtems_sdhci_configuration_size = sizeof(rtems_sdhci_configuration) / sizeof(rtems_sdhci_configuration[0]);

rtems_status_code sd_card_register()
{
  const rtems_driver_address_table sdhci_ops = SDHCI_DRIVER_TABLE_ENTRY;

  return rtems_mmc_attach(&sdhci_ops);
}
