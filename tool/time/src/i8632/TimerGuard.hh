// -*-Mode: C++;-*-
/**
@file i86/TimerGuard.hh
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@par Abstract:
Declares and implements TimerGuard for i86.

@author
Steve Tether <tether@slac.stanford.edu>

@par Date created:
2011/02/01

@par Credits:
SLAC
*/
#if !defined(TOOL_TIME_TIMERGUARD_HH)
#define      TOOL_TIME_TIMERGUARD_HH

#warning A dummy implementation of TimerGuard is being used for the i86.

namespace tool {

  namespace time {

    template <typename Accumulator>
    class TimerGuard {

    public:
      TimerGuard(Accumulator &accum, unsigned rptc, unsigned id, const char *comment)
	__attribute__((always_inline));

      ~TimerGuard() __attribute__((always_inline));
    };


    template<typename Accumulator>
    inline TimerGuard<Accumulator>::TimerGuard(Accumulator&,
					       unsigned,
					       unsigned,
					       const char*)
    {}

    template<typename Accumulator>
    inline TimerGuard<Accumulator>::~TimerGuard()
    {}

  }
}
#endif
