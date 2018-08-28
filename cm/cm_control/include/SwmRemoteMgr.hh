#ifndef SwmRemoteMgr_hh
#define SwmRemoteMgr_hh

class AbsConsole;
class SwmReply;

class PortMap;

class SwmRemoteMgr {
public:
  SwmRemoteMgr() : _port_map(0) {}

  void process(const SwmReply&, AbsConsole&);

  void setPortMap(PortMap* map) { _port_map = map; }
private:
  unsigned _map_ports(unsigned) const;

  PortMap* _port_map;
};

#endif
