// -*-Mode: C++;-*-
/*!@file     bsiIO-inl.hh
*
* @brief     Class BsiIO inline implementation
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      May 9, 2013 -- Created
*
* $Revision: 3123 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef SERVICE_CMB_GEN3_BSIIO_INL_HH
#define SERVICE_CMB_GEN3_BSIIO_INL_HH


#include "cmb/bsi.hh"


namespace service {

  namespace cmb {

    inline BsiIO::BsiIO() : _base((uint32_t*)configuration::cmb::BSI_K_ADDRESS)
    {
    }

    inline BsiIO::~BsiIO()
    {
    }

    inline unsigned BsiIO::read(const unsigned& addr, uint32_t* value) const
    {
      *value = ((uint32_t* const)(_base))[addr];

      return 0;                         // @todo - Condition codes useful?
    }

    inline void BsiIO::write(const unsigned& addr, const uint32_t& value) const
    {
      ((uint32_t* const)(_base))[addr] = value;
    }

  }

}

#endif
