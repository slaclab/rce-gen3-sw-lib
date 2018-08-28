// -*-Mode: C++;-*-
/*!
*
* @brief   Function for converting a binary MAC address to colon notation
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
#ifndef TOOL_STRING_MAC2COLON_HH
#define TOOL_STRING_MAC2COLON_HH

#include <stdint.h>
#include <string>

namespace tool {

  namespace string {

  /*! Convert a binary MAC address to a colon separated string
   *
   *  Note that the caller is expected to provide a buffer large enough to
   *  fit the entire string.
   *
   *  @param[in] mac The six byte MAC address to convert passed in an array
   *  @param[in] buffer The string buffer to fill with the result
   *  @return @buffer filled with the colon separated MAC address string
   */
  const char* const mac2colon(uint8_t const mac[6], char* const buffer);

  /*! Convert a binary MAC address to a dot separated std::string
   *
   *  @param[in] mac The six byte MAC address to convert
   *  @return a std::string filled with the colon separated MAC address string
   */
  std::string mac2colon(uint8_t const mac[6]);

  /*! Convert a binary MAC address to a colon separated string
   *
   *  Note that the caller is expected to provide a buffer large enough to
   *  fit the entire string.
   *
   *  @param[in] mac The quadword MAC address to convert
   *  @param[in] buffer The string buffer to fill with the result
   *  @return @buffer filled with the colon separated MAC address string
   */
  const char* const mac2colon(uint64_t const mac, char* const buffer);

  /*! Convert a binary MAC address to a dot separated std::string
   *
   *  @param[in] mac The quadword MAC address to convert
   *  @return a std::string filled with the colon separated MAC address string
   */
  std::string mac2colon(uint64_t const mac);

  /*! Maximum size of a colon separated MAC address string */
  enum {MaxColon = 6 * (2 + 1)};

  }

}
#endif
