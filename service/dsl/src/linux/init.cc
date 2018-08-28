// -*-Mode: C++;-*-
/*!@file     init.cc
*
* @brief     Initialization code for launching the DSL server.
*
* @author    S. Maldonado -- REG/DRD - (smaldona@slac.stanford.edu)
*
* @date      August 24, 2013 -- Created
*
* $Revision: 1993 $
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/


#include "dsl/Server.hh"      

using service::dsl::Server;

namespace service {

  namespace dsl {

    void initialize()
    {
      Server::startup();
      Server::run();
    }

  }

}
