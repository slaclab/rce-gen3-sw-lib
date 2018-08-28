// -*-Mode: C++;-*-
/*!@file     bsiIO.hh
*
* @brief     Class BsiIO definitions
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
#ifndef SERVICE_CMB_GEN3_BSIIO_HH
#define SERVICE_CMB_GEN3_BSIIO_HH




namespace service {

  namespace cmb {

    class BsiIO
    {
    public:
      BsiIO();
      BsiIO(uint32_t* base) : _base(base) {};
      ~BsiIO();

    public:
      //! Read a BSI BRAM location
      inline unsigned read(const unsigned& addr, uint32_t* value) const;

      //! Write a BSI BRAM location
      inline void write(const unsigned& addr, const uint32_t& value) const;
      
    private:
      uint32_t* _base;
    };

  }

}

#include "cmb/gen/bsiIO-inl.hh"

#endif
