#ifndef CimBPDUParameters_hh
#define CimBPDUParameters_hh

namespace cm {
  namespace bpdu {

    static const unsigned MigrateTime = 3;
    static const unsigned TxHoldCount = 6;

    class TimeParameters {
    public:
      static const TimeParameters& internal();
    public:
      TimeParameters& operator=(const TimeParameters&);
      bool operator==(const TimeParameters&) const;
      bool operator!=(const TimeParameters&) const;
      void dump() const;
    public:
      unsigned bridgeHelloTime;
      unsigned bridgeMaxAge;
      unsigned bridgeFwdDelay;
      unsigned messageAge;
    };
  };
};

#endif
