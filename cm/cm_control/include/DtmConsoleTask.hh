#ifndef DtmConsoleTask_hh
#define DtmConsoleTask_hh


#include "console/ConsoleHandler.hh"

namespace cm {
  namespace net {
    class Manager;
  };
};

class FmConfig;
class FmPortMask;
class AbsConsole;

class DtmConsoleTask : public service::console::ConsoleHandler {
public:
  DtmConsoleTask(cm::net::Manager*);
  ~DtmConsoleTask();
public:
  bool handle(service::console::RemoteConsole& o,
              const char* cmd,
              const char* args);
  void help  (AbsConsole& o);

private:
  void _replAll    (AbsConsole&);
  void _dumpFm     (AbsConsole&);
  void _basePhyMgmt(AbsConsole&);
  void _genPacket  (AbsConsole&,const char*);
  void _rwReg      (AbsConsole&,unsigned,unsigned);
  void _reboot     ();
  void _porttest   ();
  void _dumpSerdes (const FmPortMask&);
  void _addMac     (AbsConsole&,const char*);
  void _readMac    (AbsConsole&,const char*);
  void _chain      (AbsConsole&,const char*);
private:
  cm::net::Manager* fm;
  FmConfig* fmCfg;
  char*     reply;
};
#endif
