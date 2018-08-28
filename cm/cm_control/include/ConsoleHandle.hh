#ifndef ConsoleHandle_hh
#define ConsoleHandle_hh


#include "control/AbsConsole.hh"

#include "console/RemoteConsole.hh"


class ConsoleHandle : public AbsConsole {
public:
  ConsoleHandle(service::console::RemoteConsole& o) : _o(o) {}
  ~ConsoleHandle() {}

  void printv (const char* msgformat, ...) { _o.printv(msgformat); }
  void reserve(unsigned v                ) { _o.reserve(v); }
private:
  service::console::RemoteConsole& _o;
};

#endif
