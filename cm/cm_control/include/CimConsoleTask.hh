#ifndef CimConsoleTask_hh
#define CimConsoleTask_hh


#include "console/ConsoleHandler.hh"

#include "exception/Exception.hh"
#include "container/List.hh"
#include "control/SocketTest.hh"

namespace cm {
  namespace net {
    class Manager;
  }
}


namespace cm {
  namespace ipm {
    class IpmTask;
  }
}


class FmTahoe;
class SwmRemoteMgr;
class RceSlaveTest;
class AbsConsole;

class CimConsoleTask : public service::console::ConsoleHandler {
public:
  CimConsoleTask(cm::net::Manager& mgr,
		 cm::ipm::IpmTask& ipmt);
  ~CimConsoleTask();

  virtual bool handle(service::console::RemoteConsole& o,
		      const char* cmd,
		      const char* args);
  virtual void help  (AbsConsole& o);

private:
  unsigned _getPort(const char*) const throw(tool::exception::Event);
  void     _dumpUserPorts(AbsConsole& o) const;

  enum { UserMode, ExpertMode } _mode;
  cm::net::Manager&   _mgr;
  FmTahoe*            _dev;
  SwmRemoteMgr*       _rmgr;
  char*               _reply;
  char*               _cache;
  cm::ipm::IpmTask&   _ipmt;
  // "Snake" configuration
  unsigned            _snports[24];
  unsigned            _nsnports;
  //  Socket/multicast tests
  tool::container::List<cm::control::SocketTest> _sockets;
  RceSlaveTest*       _addendum;
};

#endif

