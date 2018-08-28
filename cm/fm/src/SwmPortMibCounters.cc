
#include "fm/SwmPortMibCounters.hh"

#include "fm/FmPortMibCounters.hh"

SwmPortMibCounters::SwmPortMibCounters()
{
}

SwmPortMibCounters::SwmPortMibCounters( const FmPortMibCounters& c )
{
	rxUcast = c.rxUcast;
	rxBcast = c.rxBcast;
	rxMcast = c.rxMcast;
	rxPause = c.rxPause;
	rxFcsErrors = c.rxFcsErrors;
	rxSymbolErrors = c.rxSymbolErrors;

	for(unsigned j=0; j<8; j++)
	  rxPri[j] = c.rxPri[j];

	txUnicast = c.txUnicast;
	txBroadcast = c.txBroadcast;
	txMulticast = c.txMulticast;
	txTimeoutDrops = c.txTimeoutDrops;
	txErrorDrops = c.txErrorDrops;

	rxMinto63 = c.rxMinto63;
	rx64 = c.rx64;
	rx65to127 = c.rx65to127;
	rx128to255 = c.rx128to255;
	rx256to511 = c.rx256to511;
	rx512to1023 = c.rx512to1023;
	rx1024to1522 = c.rx1024to1522;
	rx1523to2047 = c.rx1523to2047;
	rx2048to4095 = c.rx2048to4095;
	rx4096to8191 = c.rx4096to8191;
	rx8192to10239 = c.rx8192to10239;
	rx10240toMax = c.rx10240toMax;
	rxUndersized = c.rxUndersized;
	rxOversized = c.rxOversized;
	rxFragments = c.rxFragments;

	rxGoodOctets = c.rxGoodOctets;
	rxBadOctets = c.rxBadOctets;

	txMinto63 = c.txMinto63;
	tx64 = c.tx64;
	tx65to127 = c.tx65to127;
	tx128to255 = c.tx128to255;
	tx256to511 = c.tx256to511;
	tx512to1023 = c.tx512to1023;
	tx1024to1522 = c.tx1024to1522;
	tx1523to2047 = c.tx1523to2047;
	tx2048to4095 = c.tx2048to4095;
	tx4096to8191 = c.tx4096to8191;
	tx8192to10239 = c.tx8192to10239;
	tx10240toMax = c.tx10240toMax;

	fidForwarded = c.fidForwarded;
	floodForwarded = c.floodForwarded;
	stpDrops = c.stpDrops;
	reservedTraps = c.reservedTraps;
	securityViolationDrops = c.securityViolationDrops;
	vlanTagDrops = c.vlanTagDrops;
	vlanIngressBVDrops = c.vlanIngressBVDrops;
	vlanEgressBVDrops = c.vlanEgressBVDrops;
	triggerRedirAndDrops = c.triggerRedirAndDrops;
	triggerMirrored = c.triggerMirrored;
	dlfDrops = c.dlfDrops;
	broadcastDrops = c.broadcastDrops;
	cmrxDrops = c.cmrxDrops;

	for(unsigned j=0; j<8; j++)
	  rxOctetsPri[j] = c.rxOctetsPri[j];
}

SwmPortMibCounters::~SwmPortMibCounters()
{
}

SwmPortMibCounters& SwmPortMibCounters::operator=(const SwmPortMibCounters& c)
{
	rxUcast = c.rxUcast;
	rxBcast = c.rxBcast;
	rxMcast = c.rxMcast;
	rxPause = c.rxPause;
	rxFcsErrors = c.rxFcsErrors;
	rxSymbolErrors = c.rxSymbolErrors;

	for(unsigned j=0; j<8; j++)
	  rxPri[j] = c.rxPri[j];

	txUnicast = c.txUnicast;
	txBroadcast = c.txBroadcast;
	txMulticast = c.txMulticast;
	txTimeoutDrops = c.txTimeoutDrops;
	txErrorDrops = c.txErrorDrops;

	rxMinto63 = c.rxMinto63;
	rx64 = c.rx64;
	rx65to127 = c.rx65to127;
	rx128to255 = c.rx128to255;
	rx256to511 = c.rx256to511;
	rx512to1023 = c.rx512to1023;
	rx1024to1522 = c.rx1024to1522;
	rx1523to2047 = c.rx1523to2047;
	rx2048to4095 = c.rx2048to4095;
	rx4096to8191 = c.rx4096to8191;
	rx8192to10239 = c.rx8192to10239;
	rx10240toMax = c.rx10240toMax;
	rxUndersized = c.rxUndersized;
	rxOversized = c.rxOversized;
	rxFragments = c.rxFragments;

	rxGoodOctets = c.rxGoodOctets;
	rxBadOctets = c.rxBadOctets;

	txMinto63 = c.txMinto63;
	tx64 = c.tx64;
	tx65to127 = c.tx65to127;
	tx128to255 = c.tx128to255;
	tx256to511 = c.tx256to511;
	tx512to1023 = c.tx512to1023;
	tx1024to1522 = c.tx1024to1522;
	tx1523to2047 = c.tx1523to2047;
	tx2048to4095 = c.tx2048to4095;
	tx4096to8191 = c.tx4096to8191;
	tx8192to10239 = c.tx8192to10239;
	tx10240toMax = c.tx10240toMax;

	fidForwarded = c.fidForwarded;
	floodForwarded = c.floodForwarded;
	stpDrops = c.stpDrops;
	reservedTraps = c.reservedTraps;
	securityViolationDrops = c.securityViolationDrops;
	vlanTagDrops = c.vlanTagDrops;
	vlanIngressBVDrops = c.vlanIngressBVDrops;
	vlanEgressBVDrops = c.vlanEgressBVDrops;
	triggerRedirAndDrops = c.triggerRedirAndDrops;
	triggerMirrored = c.triggerMirrored;
	dlfDrops = c.dlfDrops;
	broadcastDrops = c.broadcastDrops;
	cmrxDrops = c.cmrxDrops;

	for(unsigned j=0; j<8; j++)
	  rxOctetsPri[j] = c.rxOctetsPri[j];

	return *this;
}

SwmPortMibCounters operator-(const SwmPortMibCounters& a,
			     const SwmPortMibCounters& b)
{
  SwmPortMibCounters v;
  SwmMibCounter* p_a = (SwmMibCounter*)&a;
  SwmMibCounter* p_b = (SwmMibCounter*)&b;
  SwmMibCounter* p_v = (SwmMibCounter*)&v;
  SwmMibCounter* p_end = (SwmMibCounter*)(&v + 1);
  while( p_v < p_end )
    *p_v++ = *p_a++ - *p_b++;
  return v;
}
