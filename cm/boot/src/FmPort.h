#ifndef FmPort_h
#define FmPort_h

//
//  These classes are defined to clarify the numbering convention
//  used for port ids in function calls.
//

enum Fm_PortSpeed { None, X10Mb, X100Mb, X1Gb, X2_5Gb, X4Gb, X10Gb };

//
//  class FmPort
//    Port number with the convention that the logical CPU port
//  is at index 0.
//
enum { CPU_PORT = 0 };
enum { MAX_PORT = 24 };
enum { ALL_PORTS = (2<<MAX_PORT)-1 };
enum { PHY_PORTS = (2<<MAX_PORT)-2 };

typedef uint32_t FmPort;
typedef uint32_t FmPortMask;

#endif
