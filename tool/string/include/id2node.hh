// -*-Mode: C++;-*-
/*!
*
* @brief   Function for converting a binary identifier to a node name
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    March 21, 2012 -- Created
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
#ifndef TOOL_STRING_ID2NODE_HH
#define TOOL_STRING_ID2NODE_HH

#include <string>

namespace tool {
  namespace string {

  /*! Convert a binary identifier to a node name
   *
   *  Note that the caller is expected to provide a buffer large enough to
   *  fit the entire string.  The name's format is:
   *    <shelf_name>_<cluster ID>_<element ID>_RCE
   *
   *  @param[in] group The group name
   *  @param[in] cluster The cluster address
   *  @param[in] element The element address
   *  @param[in] buffer The string buffer to fill with the result
   *  @return @buffer filled with the node name
   */
  const char* const id2node(const char* const group,
                            unsigned          cluster,
                            unsigned          element,
                            char* const       buffer);

  /*! Convert a binary identifier to a node name as a std::string
   *
   *  The name's format is:  <shelf_name>_<cluster ID>_<element ID>_RCE
   *
   *  @param[in] group The group name
   *  @param[in] cluster The cluster address
   *  @param[in] element The element address
   *  @return a std::string filled with the node name
   */
  std::string id2node(const char* const group,
                      unsigned          cluster,
                      unsigned          element);

  /*! Maximum node name size */
  enum {MaxName = 32};

  }

}
#endif
