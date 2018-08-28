#ifndef cm_svc_ClusterIpInfo_hh
#define cm_svc_ClusterIpInfo_hh

namespace cm {
  namespace svc {
    class ClusterIpInfo {
    public:
      ClusterIpInfo();
    public:
      unsigned ip         () const { return _ip; }
      unsigned netmask    () const { return _netmask; }
      unsigned netmask_len() const { return _netmask_len; }
      unsigned ip_begin   () const { return _ip_begin; }
      unsigned ip_end     () const { return _ip_end; }
      unsigned gateway    () const { return _gateway; }
    private:
      unsigned _ip;
      unsigned _netmask;
      unsigned _netmask_len;
      unsigned _ip_begin;
      unsigned _ip_end;
      unsigned _gateway;
    };
  };
};

#endif
