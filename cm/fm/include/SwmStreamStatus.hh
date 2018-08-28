#ifndef SwmStreamStatus_hh
#define SwmStreamStatus_hh

class FmTahoe;

class SwmStreamStatus {
public:
  SwmStreamStatus( const FmTahoe& );

  unsigned _sharedStream;
  unsigned _portStream[25];
};


#endif
