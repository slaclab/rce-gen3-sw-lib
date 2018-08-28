#ifndef cm_svc_ClusterIpInfo_hh
#define cm_svc_ClusterIpInfo_hh

namespace cm {
  namespace svc {
    class ClusterIpInfo {
    public:
      ClusterIpInfo();
    public:
      bool     use_vlan       () const;
      unsigned vlan_id        () const;
      bool     reject_tagged  () const;
      bool     reject_untagged() const;
      bool     reject_boundary_violation() const;
      unsigned ip         () const { return _ip; }
      unsigned netmask    () const { return _netmask; }
      unsigned netmask_len() const { return _netmask_len; }
      unsigned ip_begin   () const { return _ip_begin; }
      unsigned ip_end     () const { return _ip_end; }
      unsigned gateway    () const { return _gateway; }
    private:
      unsigned _vlan;
      unsigned _ingress_rules;
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
