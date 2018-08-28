#ifndef CimNetManager_hh
#define CimNetManager_hh

class SwmRequest;
class FmTahoeMgr;
class FmConfig;

namespace cm {
  namespace net {
    class Manager {
    public:
      Manager(int nDevices, const FmConfig*);

      unsigned          nDevices     () const;
      FmTahoeMgr&       deviceMgr    (unsigned);

      const FmConfig& config(unsigned i) const;

      int read     (FmConfig*);
      //      int configure(FmConfig*  fmCfg);
      int configure(unsigned, const FmConfig&);

      void initialize(unsigned interface);

      // routine device access
      void process(const SwmRequest& req,
                   void*      reply,
                   unsigned&  reply_len);

      // interrupt-level service routine
      void isr();

      // interrupt-level service routine for LCI FIFOs
      void fifo_isr();

      void dump() const;

      // Static initialization of board version
      static void board(const char*);

      // Access to singleton instance
      static Manager* instance();
      
      cm::svc::ChannelMap& map(unsigned) const;
    private:
      ~Manager();

    private:
      class Impl;
      Impl* _impl;
    };
  };
};

#endif
