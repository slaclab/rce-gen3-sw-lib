/*!@file     init.hh
*
* @brief     Initialization of the COB Mezzanine Board
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      March 2, 2012 -- Created
*
* $Revision: 3123 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <assert.h>


#include "cmb/bsi.hh"


namespace service {

  namespace cmb {

    inline void initialize()
    {
      // Initialize the BootStrap Interface
      BSI::initialize();

      //assert(BSI::instance()->size() != 0); // Can't usefully go on w/ BSI == 0
    }

    inline void bootMark(unsigned value)
    {
      BSI::instance()->booted(value);
    }

    inline void booted()
    {
      // Indicate that this CMB has booted to the IPMC
      BSI::instance()->booted();
    }

  } // cmb

} // service
