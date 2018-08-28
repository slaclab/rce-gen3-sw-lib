#ifndef SwmPortMibCounters_hh
#define SwmPortMibCounters_hh


#include "fm/SwmMibCounter.hh"

class FmPortMibCounters;

class SwmPortMibCounters {
public:
  SwmPortMibCounters();
  SwmPortMibCounters( const FmPortMibCounters& );
  ~SwmPortMibCounters();

  SwmPortMibCounters& operator=(const SwmPortMibCounters& c);

  SwmMibCounter rxUcast;
  SwmMibCounter rxBcast;
  SwmMibCounter rxMcast;
  SwmMibCounter rxPause;
  SwmMibCounter rxFcsErrors;
  SwmMibCounter rxSymbolErrors;
private:
  //  unsigned      reserved1[4];
public:
  SwmMibCounter rxPri[8];
  SwmMibCounter txUnicast;
  SwmMibCounter txBroadcast;
  SwmMibCounter txMulticast;
  SwmMibCounter txTimeoutDrops;
  SwmMibCounter txErrorDrops;
private:
  //  unsigned      reserved2[0x56];
public:
  SwmMibCounter rxMinto63;
  SwmMibCounter rx64;
  SwmMibCounter rx65to127;
  SwmMibCounter rx128to255;
  SwmMibCounter rx256to511;
  SwmMibCounter rx512to1023;
  SwmMibCounter rx1024to1522;
  SwmMibCounter rx1523to2047;
  SwmMibCounter rx2048to4095;
  SwmMibCounter rx4096to8191;
  SwmMibCounter rx8192to10239;
  SwmMibCounter rx10240toMax;
  SwmMibCounter rxUndersized;
  SwmMibCounter rxOversized;
  SwmMibCounter rxFragments;
private:
  //  unsigned      reserved3[2];
public:
  SwmMibCounter rxGoodOctets;
  SwmMibCounter rxBadOctets;
private:
  //  unsigned      reserved4[4];
public:
  SwmMibCounter txMinto63;
  SwmMibCounter tx64;
  SwmMibCounter tx65to127;
  SwmMibCounter tx128to255;
  SwmMibCounter tx256to511;
  SwmMibCounter tx512to1023;
  SwmMibCounter tx1024to1522;
  SwmMibCounter tx1523to2047;
  SwmMibCounter tx2048to4095;
  SwmMibCounter tx4096to8191;
  SwmMibCounter tx8192to10239;
  SwmMibCounter tx10240toMax;
private:
  //  unsigned      reserved5[0x40];
public:
  SwmMibCounter fidForwarded;
  SwmMibCounter floodForwarded;
  SwmMibCounter stpDrops;
  SwmMibCounter reservedTraps;
  SwmMibCounter securityViolationDrops;
  SwmMibCounter vlanTagDrops;
  SwmMibCounter vlanIngressBVDrops;
  SwmMibCounter vlanEgressBVDrops;
  SwmMibCounter triggerRedirAndDrops;
  SwmMibCounter triggerMirrored;
  SwmMibCounter dlfDrops;
  SwmMibCounter broadcastDrops;
  SwmMibCounter cmrxDrops;
private:
  //  unsigned      reserved6[6];
public:
  SwmMibCounter rxOctetsPri[8];
private:
  //  unsigned      reserved7[0xd0];
};

SwmPortMibCounters operator-(const SwmPortMibCounters&,
			     const SwmPortMibCounters&);

#endif
