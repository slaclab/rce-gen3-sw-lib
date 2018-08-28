//
//  Found that the statistics counter readings occasionally "glitch", where they
//  jump up or down for one reading and return to the previous reading.  I try
//  to filter those glitches from the reports.
//

#include "control/PortMonitor.hh"

#include "fm/FmTahoe.hh"
#include "fm/FmTahoeMgr.hh"
#include "fm/FmPort.hh"
#include "fm/SwmAllPortMibCounters.hh"
#include "fm/SwmGlobalMibCounters.hh"

#include "container/Link.hh"
#include "container/List.hh"

#include <stdio.h>
#include <string.h>
#include <time.h>

static char _buff[512];

class PortCounterRef : public tool::container::Link<PortCounterRef> {
public:
  enum { TITLE_LEN=16 };
  enum { VALUE_LEN=6 };
  PortCounterRef(int counter, const char* title) :
    _counter(counter),
    _title  (title),
    _mask   (0) {}
public:
  FmPortMask set(const SwmAllPortMibCounters& a,
		 const SwmAllPortMibCounters& b,
		 const SwmAllPortMibCounters& c)
  {
    _mask = FmPortMask(0);
    for(unsigned k=0; k<=FmPort::MAX_PORT; k++) {
      const SwmMibCounter& c_a = *(reinterpret_cast<const SwmMibCounter*>(&a.port(k))+_counter);
      const SwmMibCounter& c_b = *(reinterpret_cast<const SwmMibCounter*>(&b.port(k))+_counter);
      const SwmMibCounter& c_c = *(reinterpret_cast<const SwmMibCounter*>(&c.port(k))+_counter);
      if ((c_b > c_a) && (c_c >= c_b)) {
	_ports[k] = c_b - c_a;
	_mask |= (1<<k);
      }
    }
    return _mask;
  }
  void report(FmPortMask mask) {
    char* ptr = _buff;
    if (_mask != FmPortMask(0)) {
      ptr += sprintf(ptr,"%*s",TITLE_LEN,_title);
      for(unsigned k=0; k<=FmPort::MAX_PORT; k++) {
	if (mask.contains(FmPort(k))) {
	  unsigned long long v(_ports[k]);
	  if (v > (-1ULL)>>1)
	    ptr += sprintf(ptr," %*s",VALUE_LEN,"-");
	  else if (v==0)
	    ptr += sprintf(ptr," %*s",VALUE_LEN,"");
	  else {
	    int n = sprintf(ptr," %.*llu",VALUE_LEN,v);
	    if (n < VALUE_LEN+2)
	      ptr += n;
	    else
	      ptr += sprintf(ptr," %*s",VALUE_LEN,"+");
	  }
	}
      }
      printf("%s\n",_buff);
    }
  }
private:
  int           _counter;
  const char*   _title;
  SwmMibCounter _ports[FmPort::MAX_PORT+1];
  FmPortMask    _mask;
};

class PortCounters {
public:
  PortCounters() {}
  ~PortCounters() {
    PortCounterRef* r = _list.head();
    const PortCounterRef* const e = _list.empty();
    while ( r != e ) {
      PortCounterRef* v = r->flink();
      r = r->flink();
      v->remove();
      delete v;
    }
  }
public:
  void add(PortCounterRef* ref) { _list.insert(ref); }
public:
  void report(const char* title,
	      const SwmAllPortMibCounters& a,
	      const SwmAllPortMibCounters& b,
	      const SwmAllPortMibCounters& c,
	      FmPortMask upmask)
  {
    FmPortMask mask(0); // mask of ports to report

    //  set counters
    PortCounterRef* r = _list.head();
    const PortCounterRef* const e = _list.empty();
    while ( r != e ) {
      mask |= r->set(a,b,c);
      r = r->flink();
    }

    mask &= upmask;

    // report
    if (mask!=0) {
      char* ptr=_buff;
      ptr += sprintf(ptr,"%*s %*s",strlen(title),title,PortCounterRef::TITLE_LEN-strlen(title)-1,"Counter");
      for(unsigned k=0; k<=FmPort::MAX_PORT; k++)
	if (mask.contains(FmPort(k)))
	  ptr += sprintf(ptr,"  P%2d  ",k);
      printf("%s\n",_buff);
      PortCounterRef* r = _list.head();
      const PortCounterRef* const e = _list.empty();
      while ( r != e ) {
	r->report(mask);
	r = r->flink();
      }
    }
  }
private:
  tool::container::List<PortCounterRef> _list;
};


class GlobalCounters {
public:
  GlobalCounters() {}
  ~GlobalCounters() {}
public:
  unsigned long long delta(const SwmMibCounter& a,
			   const SwmMibCounter& b,
			   const SwmMibCounter& c) {
    if ((b > a) && (c >= b))
      return b-a;
    return 0;
  }
  void report(const char* title,
	      const SwmGlobalMibCounters& a,
	      const SwmGlobalMibCounters& b,
	      const SwmGlobalMibCounters& c)
  {
    unsigned long long v;

#define TEST(counter) {				\
      if ((v=delta(a.counter,			\
		   b.counter,			\
		   c.counter)))			\
	printf("%s:#counter %llx\n",title,v); }

    TEST(cmGlobalLowDrops);
    TEST(cmGlobalHighDrops);
    TEST(cmGlobalPrivilegeDrops);
#undef TEST
#define TEST(counter,i) {			\
      if ((v=delta(a.counter[i],		\
		   b.counter[i],		\
		   c.counter[i])))		\
	printf("%s:#counter[%d] %llx\n",title,i,v); }
    for(int i=0; i<=24; i++)
      TEST(cmTxDrops,i);
#undef TEST
  }
};


PortMonitor::PortMonitor(FmTahoeMgr& mgr0,
			 FmTahoeMgr& mgr1) :
  _mgr0          (mgr0),
  _mgr1          (mgr1),
  _fm0           (*new(0) FmTahoe),
  _fm1           (*new(1) FmTahoe),
  _fm0_counters  (new SwmAllPortMibCounters),
  _fm1_counters  (new SwmAllPortMibCounters),
  _fm0_counters_d(new SwmAllPortMibCounters),
  _fm1_counters_d(new SwmAllPortMibCounters),
  _monitor       (new PortCounters),
  _fm0_globals   (new SwmGlobalMibCounters),
  _fm1_globals   (new SwmGlobalMibCounters),
  _fm0_globals_d (new SwmGlobalMibCounters),
  _fm1_globals_d (new SwmGlobalMibCounters),
  _gmonitor      (new GlobalCounters)
{
}

PortMonitor::~PortMonitor()
{
  delete _fm0_counters;
  delete _fm1_counters;
  delete _fm0_counters_d;
  delete _fm1_counters_d;
  delete _monitor;
  delete _fm0_globals;
  delete _fm1_globals;
  delete _fm0_globals_d;
  delete _fm1_globals_d;
  delete _gmonitor;
}

void PortMonitor::run()
{
  //
  //  Setup port counters
  //
  {
    SwmPortMibCounters c;
    const SwmMibCounter* c0 = reinterpret_cast<const SwmMibCounter*>(&c);
//    _monitor->add(new PortCounterRef(&c.rxPause        - c0, "rxPause"));
   _monitor->add(new PortCounterRef(&c.rxFcsErrors    - c0, "rxFcsErr"));
   _monitor->add(new PortCounterRef(&c.rxSymbolErrors - c0, "rxSymErr"));
   _monitor->add(new PortCounterRef(&c.txTimeoutDrops - c0, "txTimeout"));
   _monitor->add(new PortCounterRef(&c.txErrorDrops   - c0, "txError"));
   _monitor->add(new PortCounterRef(&c.rxUndersized   - c0, "rxUndersized"));
   _monitor->add(new PortCounterRef(&c.rxOversized    - c0, "rxOversized"));
   _monitor->add(new PortCounterRef(&c.rxFragments    - c0, "rxFragments"));
   _monitor->add(new PortCounterRef(&c.rxBadOctets    - c0, "rxBadOctets"));
   _monitor->add(new PortCounterRef(&c.stpDrops       - c0, "stpDrops"));
   _monitor->add(new PortCounterRef(&c.securityViolationDrops - c0, "securityDrops"));
   _monitor->add(new PortCounterRef(&c.vlanTagDrops   - c0, "vlanTagDrops"));
   _monitor->add(new PortCounterRef(&c.vlanIngressBVDrops - c0, "vlanInBVDrops"));
   _monitor->add(new PortCounterRef(&c.vlanEgressBVDrops - c0, "vlanOutBVDrops"));
   _monitor->add(new PortCounterRef(&c.triggerRedirAndDrops - c0, "triggerDrops"));
   _monitor->add(new PortCounterRef(&c.dlfDrops       - c0, "dlfDrops"));
   _monitor->add(new PortCounterRef(&c.broadcastDrops - c0, "bcastDrops"));
   _monitor->add(new PortCounterRef(&c.cmrxDrops      - c0, "cmrxDrops"));
  }

  {
    SwmAllPortMibCounters* c;
    c = new SwmAllPortMibCounters(_fm0);
    *_fm0_counters = *c;
    *_fm0_counters_d = *c;
    delete c;
    c = new SwmAllPortMibCounters(_fm1);
    *_fm1_counters = *c;
    *_fm1_counters_d = *c;
    delete c;
  }


  //
  //  Setup global counters
  //
  {
    SwmGlobalMibCounters* c;
    c = new SwmGlobalMibCounters(_fm0);
    *_fm0_globals = *c;
    *_fm0_globals_d = *c;
    delete c;
    c = new SwmGlobalMibCounters(_fm1);
    *_fm1_globals = *c;
    *_fm1_globals_d = *c;
    delete c;
  }

  //
  //  Start the periodic reporting
  //
  timespec ts;
  ts.tv_nsec = 0;
  ts.tv_sec  = 2;

  while(1) {
    nanosleep(&ts, 0);

    update_counters("Base"  , _fm0, _mgr0.portsUp(),
		    *_fm0_counters,*_fm0_counters_d,
		    *_fm0_globals ,*_fm0_globals_d );
    update_counters("Fabric", _fm1, _mgr1.portsUp(),
		    *_fm1_counters,*_fm1_counters_d,
		    *_fm1_globals ,*_fm1_globals_d );
  }
}

void PortMonitor::update_counters(const char* title,
				  FmTahoe& fm,
				  FmPortMask mask,
				  SwmAllPortMibCounters& counters,
				  SwmAllPortMibCounters& counters_d,
				  SwmGlobalMibCounters& globals,
				  SwmGlobalMibCounters& globals_d)
{
  {
    SwmAllPortMibCounters* c = new SwmAllPortMibCounters(fm);
    _monitor->report(title,counters_d,counters,*c,mask);
    counters_d = counters;
    counters   = *c;
    delete c;
  }
  {
    SwmGlobalMibCounters* c = new SwmGlobalMibCounters(fm);
    _gmonitor->report(title,globals_d,globals,*c);
    globals_d = globals;
    globals   = *c;
    delete c;
  }
}

