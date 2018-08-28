#ifndef SwmPortMscCounters_hh
#define SwmPortMscCounters_hh

class FmPortMscCounters;

class SwmPortMscCounters {
public:
  SwmPortMscCounters();
  SwmPortMscCounters( FmPortMscCounters& );
  ~SwmPortMscCounters();

private:
  unsigned reserved1[0x26];
public:
  unsigned txPause;      // 0x80026+0x400*(N-1)
  unsigned txFcsErrors;
private:
  unsigned reserved2;
public:
  unsigned rxJabbers;    // 0x80029+0x400*(N-1)
private:
  unsigned reserved3[0x3d6];
};

SwmPortMscCounters operator-(const SwmPortMscCounters& a,
			     const SwmPortMscCounters& b);

#endif
