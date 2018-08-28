/*!@file     bsi.hh
*
* @brief     BootStrap Interface configuration information for Gen 3 CMBs
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      May 9, 2013 -- Created
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
#ifndef CONFIGURATION_CMB_GEN3_BSI_HH
#define CONFIGURATION_CMB_GEN3_BSI_HH


namespace configuration {

  namespace cmb {

    static unsigned const
    BSI_K_ADDRESS = 0x84000000;         //! Address of the BSI BRAM

    // Note: It is important _NOT_ to use the values for the version
    //       number found in the BSI_cfg.h header file since those belong to
    //       the IPMI side of the fence.  Those values have a different life-
    //       time from expected values recorded here.  In other words, if a new
    //       development causes one of the values on the IPMI side to be changed
    //       it won't actually take affect until that change is deployed.
    //       Before then the CMB code continues to expect the old values.

    static unsigned const
    BSI_K_VERSION         = 0x01;       //! Expected BSI structure version

    static unsigned const
    BSI_K_DTM_VERSION     = 0x00000000; //! Expected DTM BSI structure version
    static unsigned const
    BSI_K_DPM_VERSION     = 0x00000000; //! Expected DPM BSI structure version
  }

}

#endif
