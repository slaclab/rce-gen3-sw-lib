#ifndef AbsConsole_hh
#define AbsConsole_hh

class AbsConsole {
public:
  virtual ~AbsConsole() {}
  
  virtual void printv(const char* msgformat, ...) = 0;
  virtual void reserve(unsigned) = 0;
};

#endif
