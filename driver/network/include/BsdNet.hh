#if !defined(DRIVER_NETWORK_BSDNET_HH)
#define      DRIVER_NETWORK_BSDNET_HH



#include "exception/Exception.hh"

#include "network/BsdNet_Config.h"
#include "network/Params.hh"

namespace driver {

  namespace network {

    static const unsigned IfaceBufCount = 256;

    void initialize();

    void configure(const BsdNet_Attributes *prefs);

    void create(const char*   name,
                const uint8_t hwaddr[HwAddrLen],
                AttachFn      attach,
                const char*   ip_dotted = 0,
                const char*   netmask   = 0,
                int           rbuf_count = IfaceBufCount,
                int           xbuf_count = IfaceBufCount);

  }
}

#endif
