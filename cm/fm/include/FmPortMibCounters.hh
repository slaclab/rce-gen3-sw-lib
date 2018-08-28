#ifndef FmPortMibCounters_hh
#define FmPortMibCounters_hh


#include "fm/FmMibCounter.hh"
#include <stdint.h>

class FmPortMibCounters {
public:
  FmMibCounter rxUcast;
  FmMibCounter rxBcast;
  FmMibCounter rxMcast;
  FmMibCounter rxPause;
  FmMibCounter rxFcsErrors;
  FmMibCounter rxSymbolErrors;
private:
  uint32_t     reserved1[4];
public:
  FmMibCounter rxPri[8];  // 0x70010
  FmMibCounter txUnicast;
  FmMibCounter txBroadcast;
  FmMibCounter txMulticast;
public:
  FmMibCounter txTimeoutDrops;
  FmMibCounter txErrorDrops;
private:
  uint32_t     reserved2[0x56];  // 0x7002a
public:
  FmMibCounter rxMinto63;   // 0x70080
  FmMibCounter rx64;
  FmMibCounter rx65to127;
  FmMibCounter rx128to255;
  FmMibCounter rx256to511;
  FmMibCounter rx512to1023;
  FmMibCounter rx1024to1522;
  FmMibCounter rx1523to2047;
  FmMibCounter rx2048to4095;
  FmMibCounter rx4096to8191;
  FmMibCounter rx8192to10239;
  FmMibCounter rx10240toMax;
  FmMibCounter rxUndersized;
  FmMibCounter rxOversized;
  FmMibCounter rxFragments;
private:
  uint32_t     reserved3[2];
public:
  FmMibCounter rxGoodOctets;  // 0x700a0
  FmMibCounter rxBadOctets;
private:
  uint32_t     reserved4[4];
public:
  FmMibCounter txMinto63;   // 0x700a8
  FmMibCounter tx64;
  FmMibCounter tx65to127;
  FmMibCounter tx128to255;
  FmMibCounter tx256to511;
  FmMibCounter tx512to1023;
  FmMibCounter tx1024to1522;
  FmMibCounter tx1523to2047;
  FmMibCounter tx2048to4095;
  FmMibCounter tx4096to8191;
  FmMibCounter tx8192to10239;
  FmMibCounter tx10240toMax;
private:
  uint32_t     reserved5[0x40]; // 0x700c0
public:
  FmMibCounter fidForwarded;   // 0x70100
  FmMibCounter floodForwarded;
  FmMibCounter stpDrops;
  FmMibCounter reservedTraps;
  FmMibCounter securityViolationDrops;
  FmMibCounter vlanTagDrops;
  FmMibCounter vlanIngressBVDrops;
  FmMibCounter vlanEgressBVDrops;
  FmMibCounter triggerRedirAndDrops;
  FmMibCounter triggerMirrored;
  FmMibCounter dlfDrops;
  FmMibCounter broadcastDrops;
  FmMibCounter cmrxDrops;
private:
  uint32_t     reserved6[6];
public:
  FmMibCounter rxOctetsPri[8];
private:
  uint32_t     reserved7[0xd0];
};

#endif
