#ifndef FmLocalMgr_hh
#define FmLocalMgr_hh


#include "concurrency/Procedure.hh"
#include "fm/FmPort.hh"

class SwmRequest;
class FmTahoeMgr;
class PhyAutoNegCallback;
class FmConfig;
class rtems_bsdnet_ifconfig;
class FmInterruptTask;
class FmFifoInterruptTask;

class FmLocalMgr {
public:
  FmLocalMgr(int nDevices, FmConfig*);
  ~FmLocalMgr();

  unsigned          nDevices     () const     { return _nDevices; }
  FmTahoeMgr&       deviceMgr    (unsigned k) { return *_mgrs[k]; }

  // Extract switch configurations for saving to flash
  int extract(FmConfig* fmCfg) const;

 // routine device access
  void process(const SwmRequest& req,
	       void*      reply,
	       unsigned&  reply_len);

  // interrupt-level service routine
  void isr();

  // interrupt-level service routine for LCI FIFOs
  void fifo_isr();

  // callback for autonegotiation interrupts
  PhyAutoNegCallback* autonegCallback(unsigned dev, FmPort port);

  // RTEMS BSDNET interface
  int lciRtemsAttach(struct rtems_bsdnet_ifconfig* aBsdConfig,
		     int attaching,
		     unsigned device);

  // Access to singleton instance
  static FmLocalMgr* instance();

private:
  FmTahoeMgr*       deviceManager(unsigned);
  const FmTahoeMgr* deviceManager(unsigned) const;

  unsigned     _nDevices;
  FmTahoeMgr** _mgrs;

  FmInterruptTask*      _interruptTask;
  FmFifoInterruptTask*  _fifoInterruptTask;

  friend class CimConsoleTask;
};

#endif
