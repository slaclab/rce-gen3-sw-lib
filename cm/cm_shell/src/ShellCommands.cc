/* Support code for configuring shell commands in an RTEM
 *
 *  Author:  <Jim Panetta: panetta@slac.stanford.edu>
 *  Created: <2009-03-24>
 *  Time-stamp: <2009-05-01 14:38:28 panetta>
 *
 *
 *
 *
*/


#include "shell/ShellCommon.hh"
#include "shell/ShellCommands.hh"


// Add all shell commands
void cm::shell::addCommands() {
  cm::shell::EtherStat     etherStat;      etherStat.initialize();
  cm::shell::Priority      priority;       priority.initialize();
}
