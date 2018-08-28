#ifndef FmPortMibCounters_h
#define FmPortMibCounters_h



#include "FmMibCounter.h"

struct FmPortMibCounters_s {
  FmMibCounter rxUcast;
  FmMibCounter rxBcast;
  FmMibCounter rxMcast;
  FmMibCounter rxPause;
  FmMibCounter rxFcsErrors;
  FmMibCounter rxSymbolErrors;
  uint32_t     reserved1[2];
  FmMibCounter txMulticast;
  FmMibCounter rxPri[8];
  FmMibCounter txUnicast;
  FmMibCounter txBroadcast;
  uint32_t     reserved0[2];
  FmMibCounter txTimeoutDrops;
  FmMibCounter txErrorDrops;
  uint32_t     reserved2[0x56];
  FmMibCounter rxMinto63;
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
  uint32_t     reserved3[2];
  FmMibCounter rxGoodOctets;
  FmMibCounter rxBadOctets;
  uint32_t     reserved4[4];
  FmMibCounter txMinto63;
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
  uint32_t     reserved5[0x40];
  FmMibCounter fidForwarded;
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
  uint32_t     reserved6[6];
  FmMibCounter rxOctetsPri[8];
  uint32_t     reserved7[0xd0];
};

typedef struct FmPortMibCounters_s FmPortMibCounters;

#endif
