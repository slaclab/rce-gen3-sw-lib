#ifndef CmFwUploadTask_hh
#define CmFwUploadTask_hh


#include "concurrency/Procedure.hh"

class CmFwUploadTask : public tool::concurrency::Procedure {
public:
  CmFwUploadTask();
  ~CmFwUploadTask();

  virtual void run();
};

#endif

