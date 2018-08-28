#ifndef cm_svc_PlxBridge_hh
#define cm_svc_PlxBridge_hh

#include <semaphore.h>
#include <stdint.h>

namespace cm {
  namespace svc {
    class PlxBridge {
    public:
      static PlxBridge& instance();
    public:
      void reset();
      uint32_t read ();
      uint32_t read (const uint32_t*);
      void     write(uint32_t*, uint32_t);
      void     set_verbose(unsigned);
      void     dump() const;
    private:
      PlxBridge();
      ~PlxBridge();
    private:
      void* _mapped_base;
      sem_t _sem;
      // volatile uint32_t* _reg;
    };
  };
};

#endif
