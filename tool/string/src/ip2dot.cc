// -*-Mode: C++;-*-
/*!
*
* @brief   Function for converting a binary IP address to dot notation
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
#include <stdio.h>


#include "string/ip2dot.hh"


const char* const tool::string::ip2dot(unsigned ip, char* const buffer)
{
  sprintf(buffer, "%u.%u.%u.%u", (ip >> 24) & 0xff,
                                 (ip >> 16) & 0xff,
                                 (ip >>  8) & 0xff,
                                 (ip      ) & 0xff);
  return buffer;
}


std::string tool::string::ip2dot(unsigned ip)
{
  char buffer[MaxDotted];
  return std::string(ip2dot(ip, buffer));
}
