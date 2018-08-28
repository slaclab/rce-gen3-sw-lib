#ifndef FmBaliMgr_hh
#define FmBaliMgr_hh

//
//  This class is responsible for configuring/managing the FM4224
//  and keeping the FmConfig object (configuration description)
//  concurrent with the device.  Note that dynamic entries to the
//  FM4224 MAC table will not be reflected in the FmConfig object.
//

#include "fm/FmConfig.hh"

#include "fm/FmPort.hh"

class FmEplHandler;
class FmMacTable;
class FmHandler;
class FmMReg;
class mbuf;

namespace Bali {
  class FmBali;

class FmBaliMgr {
public:
  FmBaliMgr( FmBali&, const FmConfig& );
  ~FmBaliMgr();

  void reset(const FmConfig&);
  int configure(const FmConfig&);

  /**
  int read     (FmConfig&);
  **/

  FmPortMask portsUp() const;
  cm::fm::PortSpeed portSpeed(FmPort) const;

  /**
  const FmMacTable& macTable() const;
  FmMacTable& macTable();

  void handle_interrupt();

  enum InterruptType { LCI=0, TRG, FCTL, MGR, EPL, SEC, VLN2, VLN1, PERR, nInterrupts };
  FmHandler* registerHandler(InterruptType, FmHandler*);
  **/
  unsigned char* getMacAddress() const;
  void getMacAddress(unsigned char*) const;
  void setMacAddress(unsigned char*);
  void setSAFMatrix ();
  void releasePort(int port);
  void dumpEplPortCtrl(int line);
  void configurePort(FmPort port, cm::fm::PortSpeed,
		     unsigned rxFlip=0, unsigned txFlip=0, unsigned drive=0x84,
                     unsigned trunk=0);
  void configureLCI ();

  //
  /**
  void changePortMulticastGroupMembership(FmPortMask ports, unsigned mcast, bool isListening);
  void trapIGMP(bool);

  void hop(FmPortMask, bool);
  **/
  unsigned id() const;
  FmBali& dev();

private:
  FmBali&    _dev;
  /**
  FmMacTable* _macTable;

  void _genHandle(InterruptType, FmMReg&, FmMReg&);
  FmHandler* _handler[nInterrupts];

  // Special handler for EPL interrupts
  FmEplHandler* _eplHandler;

  void _trunk_set   (FmPortMask, unsigned);
  void _trunk_add   (FmPort, unsigned);
  void _trunk_remove(FmPort, unsigned);
  **/
public:
  FmConfig _config;

  /**
  //  triggers
  enum { NoUnusedTrigger = 0x3f };
  unsigned               unusedTriggers() const;
  unsigned               lowestPriorityUnusedTrigger () const;
  unsigned               highestPriorityUnusedTrigger() const;
  const FmTriggerConfig& triggerConfig () const;

  void                   configureTrigger(unsigned id, const FmTriggerConfig&);
  **/

private:
  unsigned               _unusedTriggers;

private:
  void _logFifo(unsigned*,unsigned) const;
};
};
#endif
