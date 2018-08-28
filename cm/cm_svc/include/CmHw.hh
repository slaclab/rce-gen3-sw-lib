#ifndef CmHw_hh
#define CmHw_hh

namespace cm {
  namespace svc {

    class CmHw {
    public:
      static unsigned get_version_time();
      static const char* get_version_tag ();
    };

  };
};

#endif
