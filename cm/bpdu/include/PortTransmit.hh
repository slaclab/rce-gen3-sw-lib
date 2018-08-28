#ifndef CimBPDUPortTransmit_hh
#define CimBPDUPortTransmit_hh

//
//  asserts rcvdMsg
//

namespace cm {
  namespace bpdu  {
    class PortVariables;
    class PortTransmit {
    public:
      PortTransmit(PortVariables& v);
      ~PortTransmit();
    public:   // signals
      bool transition();
    private:
      void periodic();
      void config();
      void tcn();
      void rstp();
    private:
      PortVariables& _v;
    };
  };
};

#endif
