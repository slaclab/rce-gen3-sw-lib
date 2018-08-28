/*!
*
* @brief     Dump the BSI contents.
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      October 17, 2013 -- Created
*
* $Revision: 3123 $
*
* @verbatim:
*                               Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include <stdio.h>
#include <string.h>

#include <string>
using std::string;

#include <rtems.h>


#include "shell/os/BaseCommands.hh"
#include "cmb/bsi.hh"

namespace service {
namespace shell {

  static string bsiUsageLong =  "Usage:\n"
    "  bsi \n"
    "  Show the contents of the BSI structure.\n";

  static string bsiUsage = "Show the contents of the BSI structure";


  int bsi_main(int argc, char **argv) {
    service::cmb::BSI::instance()->dump();

    return 0;
  }

  std::string BSI::usage() {
    return bsiUsage;
  }

  Command::MainFunc BSI::main() {
    return bsi_main;
  }

}
}
