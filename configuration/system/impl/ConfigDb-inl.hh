/*!
*
* @brief   Class for accessing the static configuration database
*          N.B. This implementation is a stand-in until we decide how
*               to handle the persistant store
*
* @author  Ric Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    April 5, 2011 -- Created
*
* $Revision: 2961 $
*
* @verbatim:
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef CONFIGURATION_SYSTEM_CONFIGDB_INL_HH
#define CONFIGURATION_SYSTEM_CONFIGDB_INL_HH

namespace configuration {
  namespace system {

    inline const ConfigDb& ConfigDb::instance()
    {
      return *_configDb;
    }

    inline int ConfigDb::get(unsigned entry, unsigned* value) const
    {
      *value = entry;
      return SUCCESS;
    }

    inline int ConfigDb::get(unsigned entry, void** value) const
    {
      *value = (void*)entry;
      return SUCCESS;
    }

  }
}

#endif // CONFIGURATION_SYSTEM_CONFIGDB_INL_HH
