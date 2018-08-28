// -*-Mode: C++;-*-
/**
@cond development
@file MacAddress.cc
@brief Implement the encapsulation of the hardware address.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include <stdio.h>
#include <arpa/inet.h>



#include "conversion/Endianness.h"
#include "net/MacAddress.hh"

using namespace service::net;

const char* CONVERTER = "%02X:%02X:%02X:%02X:%02X:%02X";

/*
** ++
**
**
** --
*/

MacAddress::MacAddress(uint64_t value) :
  _value(value)
  {
  
#if defined (ENDIANNESS_IS_LITTLE)
  uint64_t address = value >> 16;

  uint8_t a = address;
  uint8_t b = address >> 8;
  uint8_t c = address >> 16;
  uint8_t d = address >> 24;
  uint8_t e = address >> 32;
  uint8_t f = address >> 40;
#else
  uint64_t address = value;

  uint8_t f = address;
  uint8_t e = address >> 8;
  uint8_t d = address >> 16;
  uint8_t c = address >> 24;
  uint8_t b = address >> 32;
  uint8_t a = address >> 40;
#endif 
  sprintf(_string, CONVERTER, a, b, c, d, e, f); 
  
  }



/* @endcond */


// =========== Documentation ================

namespace service {
  namespace net {
/**
   @class MacAddress
   @brief Encapsulate a hardware (or MAC) address and provide a useful presentation.

   @fn MacAddress::MacAddress(uint64_t value)
   @brief Construct from a 64 bit representation of a hardware address.
   @param[in] value  The hardware address in a 64 bit representation.
   @note The value of the hardware address is typically retrieved from
   Interface::hwaddr().


   @fn const char* MacAddress::string() const
   @brief Format the MAC address for printing.
   @return The address in colon notation (A:B:C:D:E:F)

   @fn uint64_t MacAddress::value() const
   @brief Return the 64 bit MAC address.
   @return The MAC address.


*/

  }
}
