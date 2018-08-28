#ifndef DtmMonitorTask_hh
#define DtmMonitorTask_hh

#include "concurrency/Procedure.hh"

namespace cm { namespace net { class Manager; }; };
class FmConfig;

class DtmMonitorTask : public tool::concurrency::Procedure {
public:
  DtmMonitorTask(cm::net::Manager*);
  ~DtmMonitorTask();
public:
  void run();
private:
  void _execute   ();
  void dumpFm     ();
  void basePhyMgmt();
  void genPacket  ();
  void rwReg      ();
  void rxCheck    ();
private:
  cm::net::Manager* fm;
};
#endif
