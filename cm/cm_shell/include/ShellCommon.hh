#ifndef CimSvc_ShellCommon_hh
#define CimSvc_ShellCommon_hh

namespace cm {
  namespace shell {
    class AbsConsole {
    public:
      virtual ~AbsConsole() {}
      virtual void printv(const char* msgformat, ...) = 0;
      virtual void reserve(unsigned) = 0;
    };

    class Command {
    public:
      virtual ~Command() {}
      virtual void open   (AbsConsole*) = 0;
      virtual void process(int, char**, AbsConsole*) = 0;
    };

    void* initialize( void* );  // Argument is Command*
  }
}

#endif
