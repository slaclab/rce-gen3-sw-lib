/* Defines the prototypes for all shell commands
 *
 *  Author:  <Jim Panetta: panetta@slac.stanford.edu>
 *  Created: <2009-03-24>
 *  Time-stamp: <2011-01-12 18:22:36 panetta>
 *
 *  Command prototypes:

 *
*/

#ifndef CM_SHELL_SHELLCOMMANDS_HH
#define CM_SHELL_SHELLCOMMANDS_HH


#include "shell/Command.hh"
#include <string>

namespace cm {
  namespace shell {

    class EtherStat : public service::shell::Command {
    public:
      EtherStat() : service::shell::Command("etherstat", "network") {}
      virtual ~EtherStat() {}
      virtual std::string usage();
      virtual MainFunc main();
    };

    class Priority : public service::shell::Command {
    public:
      Priority() : service::shell::Command("priority", "network") {}
      virtual ~Priority() {}
      virtual std::string usage();
      virtual MainFunc main();
    };

    void addCommands();

  }
}

#endif
