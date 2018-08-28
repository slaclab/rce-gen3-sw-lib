#ifndef SwmTrigger_hh
#define SwmTrigger_hh


#include "fm/FmPort.hh"

class FmTahoe;

class SwmTrigger {
public:
  SwmTrigger() {}
  SwmTrigger(unsigned w0, unsigned w1, unsigned w2, unsigned w3);

public:
  static const char* header(unsigned);
  const char* field(unsigned) const;

  bool     enabled() const;
  unsigned id() const;
  unsigned new_priority () const;
  unsigned mirror_port  () const;
  unsigned action       () const;
  bool     src_dst_match() const;
  bool     priority_match() const;
  bool     mcast() const;
  bool     bcast() const;
  bool     ucast() const;
  bool     vlan () const;
  bool     dst_port() const;
  bool     src_port() const;
  bool     dst_miss() const;
  bool     dst_hit () const;
  bool     src_miss() const;
  bool     src_hit() const;
  unsigned priority() const;
  unsigned src_ports() const;
  unsigned dst_ports() const;

private:
  unsigned _data[4];
};

class SwmTriggerSet {
public:
  SwmTriggerSet() {}
  SwmTriggerSet(FmTahoe&);

  SwmTriggerSet& operator=(const SwmTriggerSet&);

  int operator==(const SwmTriggerSet&) const;

  const SwmTrigger& trigger(unsigned i) const { return _triggers[i]; }

public:
  void          print() const;

private:
  SwmTrigger _triggers[16];
};

#endif
