#ifndef TahoeShell_hh
#define TahoeShell_hh

#include "cm_shell/ShellCommon.hh"

class FmConfig;
class DtmInterface;

class TahoeShell : public cm::shell::Command {
public:
  TahoeShell(FmConfig&, DtmInterface*);
public:
  void open   (cm::shell::AbsConsole* console);
  void process(int argc, char** argv, cm::shell::AbsConsole* console);
};

#endif
