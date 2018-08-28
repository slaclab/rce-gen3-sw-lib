#include "bldInfo.h"

#include "rtems/devel.hh"
#include "debug/Debug.hh"
#include "debug/Print.hh"
#include "shell/ShellCommon.hh"
#include "control/DtmInit.hh"


extern "C" void init_executive()
{
  service::debug::clearMessages();
  service::debug::printv("dtm: SVN %s, built %s",
                         cm::control::bldRevision(),
                         cm::control::bldDate());

  service::shell::addCommands();

  cm::control::dtmInitialize();

  service::shell::initialize();
}
