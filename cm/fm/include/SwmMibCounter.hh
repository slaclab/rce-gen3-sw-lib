#ifndef SwmMibCounter_hh
#define SwmMibCounter_hh


#include "fm/FmMibCounter.hh"

class SwmMibCounter {
public:
  SwmMibCounter();
  SwmMibCounter(unsigned long long);
  SwmMibCounter(const FmMibCounter&);
  ~SwmMibCounter();

  SwmMibCounter& operator=( const FmMibCounter& );

  operator unsigned long long() const;

  unsigned data[2];
};


inline SwmMibCounter::SwmMibCounter()
{
}

inline SwmMibCounter::SwmMibCounter(unsigned long long v)
{
  data[0] = v & 0xffffffff;
  data[1] = v >> 32;
}

inline SwmMibCounter::SwmMibCounter(const FmMibCounter& c)
{
  data[0] = c.data[0];
  data[1] = c.data[1];
}

inline SwmMibCounter::~SwmMibCounter()
{
}

inline SwmMibCounter& SwmMibCounter::operator=(const FmMibCounter& c)
{
  data[0] = c.data[0];
  data[1] = c.data[1];
  return *this;
}

inline SwmMibCounter::operator unsigned long long() const
{
  unsigned long long v = data[1];
  v <<= 32;
  v |= data[0];
  return v;
}

inline SwmMibCounter operator-(const SwmMibCounter& a,
			       const SwmMibCounter& b)
{
  unsigned long long v_a = a;
  unsigned long long v_b = b;
  return SwmMibCounter(v_a - v_b);
}


#endif
