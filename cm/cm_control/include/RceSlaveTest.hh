#ifndef RceSlaveTest_hh
#define RceSlaveTest_hh

namespace service {
  namespace console {
    class RemoteConsole;
  };
};


class RceSlaveTest {
public:
  RceSlaveTest();
  ~RceSlaveTest();
public:
  bool handle(service::console::RemoteConsole& o,
              const char* cmd,
              const char* args);
private:
};

#endif
