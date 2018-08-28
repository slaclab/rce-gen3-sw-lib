#ifndef DtmDisplayTask_hh
#define DtmDisplayTask_hh


#include "concurrency/Procedure.hh"
#include "cm_svc/ChannelMap.hh"

namespace cm {
  namespace ffs {
    class IdFile;
  }
}

class ChrDisplay;

class FmTahoeMgr;

class DtmDisplayTask : public tool::concurrency::Procedure {
public:
  DtmDisplayTask(cm::ffs::IdFile& idf);
public:
  virtual void run();
private:
  ChrDisplay* _display;
};
#endif
