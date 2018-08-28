#ifndef FmHandler_hh
#define FmHandler_hh

class FmHandler {
public:
  virtual ~FmHandler() {}
  virtual void call(unsigned)=0;
};

#endif
