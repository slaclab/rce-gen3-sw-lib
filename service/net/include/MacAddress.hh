 // -*-Mode: C++;-*-
/**
@file MacAddress.hh
@brief Encapsulate a hardware (or MAC) address and provide a useful presentation.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#ifndef SERVICE_NET_MACADDRESS
#define SERVICE_NET_MACADDRESS

#include <inttypes.h>
#include <stddef.h>

namespace service {
namespace net  {

class MacAddress {
public:
  MacAddress(uint64_t value);                                             
public:
 ~MacAddress() {}
public:  
   const char* string() const {return _string;}
   uint64_t    value() const  {return _value;}
private:
  uint64_t _value;
  char     _string[32];
};

}}

#endif

