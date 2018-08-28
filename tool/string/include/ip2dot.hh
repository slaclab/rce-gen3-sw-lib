// -*-Mode: C++;-*-
/*!
*
* @brief   Function for converting a binary IP address to dot notation
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    March 15, 2012 -- Created
*
* $Revision: 2961 $
*
* @verbatim
*                               Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_STRING_IP2DOT_HH
#define TOOL_STRING_IP2DOT_HH

#include <string>

namespace tool {
  namespace string {

  /*! Convert a binary IP address to a dot separated string
   *
   *  Note that the caller is expected to provide a buffer large enough to
   *  fit the entire string.
   *
   *  @param[in] ip The 32 bit IP address to convert
   *  @param[in] buffer The string buffer to fill with the result
   *  @return @buffer filled with the dot separated IP address string
   */
  const char* const ip2dot(unsigned ip, char* const buffer);

  /*! Convert a binary IP address to a dot separated std::string
   *
   *  @param[in] ip The 32 bit IP address to convert
   *  @return a std::string filled with the dot separated IP address string
   */
  std::string ip2dot(unsigned ip);

  /*! Maximum size of a dot separated IP address string */
  enum {MaxDotted = sizeof(unsigned) * (3 + 1)};

  }
}
#endif
