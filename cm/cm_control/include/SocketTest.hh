#ifndef SocketTest_hh
#define SocketTest_hh


#include "container/Link.hh"

#include <netinet/in.h>

namespace cm {
  namespace control {
    class SocketTest : public tool::container::Link<SocketTest> {
    public:
      SocketTest(unsigned interface, unsigned group, unsigned short port);
      ~SocketTest();

      unsigned address() const;

      int join();
      int resign();

    private:
      struct in_addr _groupAddr;
      struct in_addr _interfaceAddr;
      int _sockfd;
    };
  }
}


inline unsigned cm::control::SocketTest::address() const
{
  return _groupAddr.s_addr;
}

#endif
