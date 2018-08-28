#ifndef CimNetManagerB_hh
#define CimNetManagerB_hh


namespace Bali {
class FmBaliMgr;
};
class SwmRequest;
class FmConfig;

namespace cm {
  namespace svc { class ChannelMap; };
  namespace net {
    class ManagerB {
    public:
      ManagerB(const FmConfig*);
      ManagerB(int, const FmConfig*);
      ~ManagerB();

      unsigned          nDevices     () const     { return _nDevices; }
      Bali::FmBaliMgr&  deviceMgr    (unsigned k) { return *_mgrs[k]; }

      const FmConfig& config(unsigned i) const;

      /**
      int read     (FmConfig*  fmCfg);
      **/
      int configure(FmConfig*  fmCfg);
      int configure(unsigned, const FmConfig&);

      // routine device access
      void process(const SwmRequest& req,
                   void*      reply,
                   unsigned&  reply_len);

      // Access to singleton instance
      static ManagerB* instance();

      static void board(const char*);

      const cm::svc::ChannelMap& map(unsigned) const;
    private:
      Bali::FmBaliMgr*       deviceManager(unsigned);
      const Bali::FmBaliMgr* deviceManager(unsigned) const;

      unsigned     _nDevices;
      Bali::FmBaliMgr** _mgrs;
    };
  };
};

#endif
