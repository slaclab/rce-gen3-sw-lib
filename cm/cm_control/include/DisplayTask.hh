#ifndef DisplayTask_hh
#define DisplayTask_hh


#include "concurrency/Procedure.hh"
#include "cm_svc/ChannelMap.hh"

namespace cm {
  namespace ffs {
    class IdFile;
  }
}


class ChrDisplay;

class FmTahoeMgr;

class DisplayTask : public tool::concurrency::Procedure {
public:
  DisplayTask(cm::ffs::IdFile& idf,
	      FmTahoeMgr& fm0,
	      FmTahoeMgr& fm1);
public:
  virtual void run();
private:
  FmTahoeMgr& _fm0;
  FmTahoeMgr& _fm1;
  unsigned   _hasBx;
  cm::svc::ChannelMap _baseMap;
  cm::svc::ChannelMap _fabrMap;
  ChrDisplay* _display;
};
#endif
