#ifndef FmLciRtems_hh
#define FmLciRtems_hh

class FmLciRtems {
public:
  struct arpcom iArpcom;

  rtems_id iRxThread;
  rtems_id iTxThread;
  rtems_id iniThread;
};

#endif
