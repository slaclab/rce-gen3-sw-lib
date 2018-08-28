// -*-Mode: C++;-*-
/*!
*
* @brief   Function for converting a binary identifier to a node name
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    March 21, 2012 -- Created
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


#include "string/id2node.hh"


const char* const tool::string::id2node(const char* const group,
                                        unsigned          cluster,
                                        unsigned          element,
                                        char* const       buffer)
{
  sprintf(buffer, "%s_%d_%d_%s", group, cluster, element, "RCE");
  return buffer;
}


std::string tool::string::id2node(const char* const group,
                                  unsigned          cluster,
                                  unsigned          element)
{
  char buffer[MaxName];
  return std::string(id2node(group, cluster, element, buffer));
}
