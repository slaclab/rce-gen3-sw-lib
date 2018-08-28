#ifndef CimNetDriver_hh
#define CimNetDriver_hh


#include "cm_net/Handler.hh"
#include "container/List.hh"
#include "concurrency/Semaphore.hh"

class FmTahoeMgr;
struct mbuf;

namespace cm {
  namespace net {
    class Driver {
    public:
      Driver(FmTahoeMgr&);
      ~Driver();

      tool::container::List<HandlerPtr>& handlers();

      void transmit(void*, unsigned dmask);
      void transmit(char*, unsigned len, unsigned dmask);
      void receive ();

      void unblockTx();
      void unblockRx();

    private:
      FmTahoeMgr&                       _mgr;
      tool::container::List<HandlerPtr> _handlers;
      tool::concurrency::Semaphore      _txsem;
      tool::concurrency::Semaphore      _rxsem;
    };
  };
}

#endif
