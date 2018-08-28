// -*-Mode: C++;-*-
/*!
*
* @brief   Function for converting a binary MAC address to colon notation
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    March 15, 2012 -- Created
*
* $Revision: 3125 $
*
* @verbatim
*                               Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include "string/mac2colon.hh"


const char* const
tool::string::mac2colon(uint8_t const mac[6], char* const buffer)
{
  int j = 0;

  for (int i = 0; i < 6; ++i)
  {
    const char* hex = "0123456789abcdef";
    buffer[j++] = hex[(mac[i] >> 4) & 0xf];
    buffer[j++] = hex[(mac[i]     ) & 0xf];
    buffer[j++] = ':';
  }
  buffer[j - 1] = '\0';   // Overwrite the last buffer with the null terminator
  return buffer;
}


std::string
tool::string::mac2colon(uint8_t const mac[6])
{
  char buffer[MaxColon];
  return std::string(mac2colon(mac, buffer));
}


const char* const
tool::string::mac2colon(uint64_t const mac, char* const buffer)
{
  uint8_t value[6];
  value[0] = (mac >> 40) & 0xff;
  value[1] = (mac >> 32) & 0xff;
  value[2] = (mac >> 24) & 0xff;
  value[3] = (mac >> 16) & 0xff;
  value[4] = (mac >>  8) & 0xff;
  value[5] = (mac      ) & 0xff;
  return mac2colon(value, buffer);
}


std::string tool::string::mac2colon(uint64_t const mac)
{
  char buffer[MaxColon];
  return std::string(mac2colon(mac, buffer));
}
