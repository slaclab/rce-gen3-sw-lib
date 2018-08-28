#ifndef CimNetService_hh
#define CimNetService_hh

namespace cm {
  namespace net {
    class Service {
    public:
      static int in_cksum(void*,unsigned);
    };
  };
};

#endif
