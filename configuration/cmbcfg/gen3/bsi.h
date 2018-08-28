/*!@file     bsi.h
*
* @brief     BootStrap Interface configuration information for Gen 3 CMBs
*
* @author    S. Maldonado -- (smaldona@slac.stanford.edu)
*
* @date      August 10, 2013 -- Created
*
* $Revision: 1861 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef CONFIGURATION_CMB_GEN3_BSI_H
#define CONFIGURATION_CMB_GEN3_BSI_H

#define BSI_K_ADDRESS 0x84000000         //! Address of the BSI BRAM

// Note: It is important _NOT_ to use the values for the version
//       number found in the BSI_cfg.h header file since those belong to
//       the IPMI side of the fence.  Those values have a different life-
//       time from expected values recorded here.  In other words, if a new
//       development causes one of the values on the IPMI side to be changed
//       it won't actually take affect until that change is deployed.
//       Before then the CMB code continues to expect the old values.

#define BSI_K_VERSION          0x01       //! Expected BSI structure version

#define BSI_K_DTM_VERSION      0x00000000 //! Expected DTM BSI structure version

#define BSI_K_DPM_VERSION      0x00000000 //! Expected DPM BSI structure version

#endif
