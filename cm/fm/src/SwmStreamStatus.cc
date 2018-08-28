
#include "fm/SwmStreamStatus.hh"

#include "fm/FmTahoe.hh"

SwmStreamStatus::SwmStreamStatus( const FmTahoe& t )
{
  _sharedStream = t.stream_status_2;
  for(unsigned k=0; k<=24; k++)
    _portStream[k] = t.stream_status_1[k];
}

