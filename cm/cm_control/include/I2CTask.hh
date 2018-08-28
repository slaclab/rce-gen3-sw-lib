#ifndef I2CTask_hh
#define I2CTask_hh


#include "concurrency/Procedure.hh"
#include "concurrency/Semaphore.hh"

class I2CTask : public tool::concurrency::Procedure {
public:
  I2CTask();
public:
  virtual void run();
private:
  tool::concurrency::Semaphore _sem;
};
#endif
