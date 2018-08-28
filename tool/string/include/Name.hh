// -*-Mode: C++;-*-
/*!
*
* @file
* @brief   Function for converting a word to a %string
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 2961 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_STRING_NAME_HH
#define TOOL_STRING_NAME_HH

namespace tool {
  namespace string {
    /** @brief Convert an unsigned int into a C-%string.

    Requires at least sizeof(unsigned)+1 bytes of storage for the %string. The
    most significant byte of the word becomes the first char in the %string.
    @param[in] word
    @param[out] name
    @return A pointer to the %string.
    */
    const char* word_to_name(unsigned word, char* name);
  }; // string
}; // tool

#endif // TOOL_STRING_NAME_HH
