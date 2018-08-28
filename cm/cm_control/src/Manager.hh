#ifndef CimNetManager_hh
#define CimNetManager_hh



class SwmRequest;
class FmTahoeMgr;
class FmConfig;

namespace cm {
  namespace svc { class ChannelMap; };
  namespace net {
    class Manager {
    public:
      Manager(const FmConfig*);
      Manager(int, const FmConfig*);
      ~Manager();

      unsigned          nDevices     () const     { return _nDevices; }
      FmTahoeMgr&       deviceMgr    (unsigned k) { return *_mgrs[k]; }

      const FmConfig& config(unsigned i) const;

      int read     (FmConfig*  fmCfg);
      int configure(FmConfig*  fmCfg);
      int configure(unsigned, const FmConfig&);

      // routine device access
      void process(const SwmRequest& req,
                   void*      reply,
                   unsigned&  reply_len);

      // Access to singleton instance
      static Manager* instance();

      static void board(const char*);

      const cm::svc::ChannelMap& map(unsigned) const;
    private:
      FmTahoeMgr*       deviceManager(unsigned);
      const FmTahoeMgr* deviceManager(unsigned) const;

      unsigned     _nDevices;
      FmTahoeMgr** _mgrs;
    };
  };
};

#endif
