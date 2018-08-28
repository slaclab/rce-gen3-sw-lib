#ifndef BaliShell_hh
#define BaliShell_hh

#include "cm_shell/ShellCommon.hh"

class FmConfig;
class DtmInterface;

class BaliShell : public cm::shell::Command {
public:
  BaliShell(FmConfig&,DtmInterface*);
public:
  void open   (cm::shell::AbsConsole* console);
  void process(int argc, char** argv, cm::shell::AbsConsole* console);
};

#endif
