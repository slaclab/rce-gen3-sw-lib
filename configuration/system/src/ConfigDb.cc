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
* $Revision: 3120 $
*
* @verbatim:
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/


#include "system/ConfigDb.hh"
#include "debug/print.h"

namespace configuration {
  namespace system {

    ConfigDb* ConfigDb::_configDb = 0;


    ConfigDb::ConfigDb() {}
    ConfigDb::ConfigDb(const ConfigDb&) {}
    ConfigDb& ConfigDb::operator=(const ConfigDb&) {return *this;}
    ConfigDb::~ConfigDb() {}


    const ConfigDb& ConfigDb::create()
    {
      if (_configDb == 0)  _configDb = new ConfigDb();
      else                 dbg_bugcheck("Creation of new ConfigDb failed\n");
      return *_configDb;
    }


  }
}
