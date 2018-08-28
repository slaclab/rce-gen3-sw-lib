/*!@file     cmb.hh
*
* @brief     COB Mezzanine Board information
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      April 8, 2012 -- Created
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
#ifndef SERVICE_CMB_CMB_HH
#define SERVICE_CMB_CMB_HH


namespace service {

  namespace cmb {

    //! Enumeration of CMB bay identifiers
    enum ModuleBay  { CMB_K_BAY_0, CMB_K_BAY_1, CMB_K_BAY_2, CMB_K_BAY_3,
                      CMB_K_BAY_4, };
    //! Enumeration of CMB site identifiers
    enum ModuleSite { CMB_K_SITE_0, CMB_K_SITE_1, CMB_K_SITE_2, CMB_K_SITE_3 };
    //! Enumeration of CMB types
    enum ModuleType { CMB_K_UNRECOGNIZED, CMB_K_DTM, CMB_K_DPM };

  }

}

#endif
