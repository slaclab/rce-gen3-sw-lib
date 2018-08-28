#ifndef CimNetIGMPv2Handler_hh
#define CimNetIGMPv2Handler_hh

namespace cm {
  namespace net {
    class IGMPv2Handler : public Handler {
    public:
      IGMPv2Handler ();
      ~IGMPv2Handler();

      bool handle(struct   ether_header*,
                  char*    payload,
                  unsigned bytes);
    };
  };
};

#endif
