// -*-Mode: C++;-*-
/*!
* @file
* @brief   Function for converting a word to a string
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 3125 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include "string/Name.hh"

const char* tool::string::word_to_name(unsigned word, char* name)
{
  name[0] = (word >> 24) & 0xff;
  name[1] = (word >> 16) & 0xff;
  name[2] = (word >>  8) & 0xff;
  name[3] = (word      ) & 0xff;
  name[4] = 0;
  return name;
}
