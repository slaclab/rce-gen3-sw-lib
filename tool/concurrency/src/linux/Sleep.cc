
#include "concurrency/Sleep.hh"
#include <unistd.h>

namespace tool {
  namespace concurrency {
    void usleep(unsigned us) { ::usleep(us); }
  }
}
