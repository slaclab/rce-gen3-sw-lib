#ifndef SwmAbsReplyMgr_hh
#define SwmAbsReplyMgr_hh

//
//  Abstract base class for receiving replies (over the network)
//

class SwmReply;

class SwmAbsReplyMgr {
public:
  virtual void process(const SwmReply&)=0;
};

#endif
