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
#ifndef CONFIGURATION_SYSTEM_CONFIGDB_HH
#define CONFIGURATION_SYSTEM_CONFIGDB_HH

namespace configuration
{
  namespace system
  {

    class ConfigDb
    {
    public:

      enum {SUCCESS = 0, NOT_FOUND = -1};

    public:
      static const ConfigDb& create();
      inline static const ConfigDb& instance();

      inline int get(unsigned entry, unsigned* value) const;
      inline int get(unsigned entry, void**    value) const;

    private:
      ConfigDb();                           // Prevent uncontrolled construction
      ConfigDb(const ConfigDb&);            // Prevent copy-construction
      ConfigDb& operator=(const ConfigDb&); // Prevent assignment
      virtual ~ConfigDb();

    private:
      static ConfigDb* _configDb;
    };

  } // system
} // configuration


#include "system/impl/ConfigDb-inl.hh"

#endif // CONFIGURATION_SYSTEM_CONFIGDB_HH
