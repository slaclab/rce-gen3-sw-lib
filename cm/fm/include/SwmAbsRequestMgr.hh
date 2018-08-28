#ifndef SwmAbsRequestMgr_hh
#define SwmAbsRequestMgr_hh

//
//  Abstract base class for receiving requests (over the network)
//

class SwmRequest;

class SwmAbsRequestMgr {
public:
  virtual void process(const SwmRequest&)=0;
};

#endif
