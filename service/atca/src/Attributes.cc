/*
** ++
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Michael Huffer, SLAC (mehsys@slac.stanford.edu)
**
**  Creation Date:
**	000 - January 09, 2007
**
**  Revision History:
**	None.
**
** --
*/



#include "net/Interface.hh"
#include "atca/Attributes.hh"

using namespace service::atca;

/** @brief Attributes constructor
    @param group
    @param cluster
    @param bay
    @param element
 */
Attributes::Attributes(const char* group, uint32_t cluster, uint32_t bay, uint32_t element) :
  _group(group),   
  _cluster(cluster),
  _bay(bay),
  _element(element)
  {
  net::Interface interface;
  _ip  = interface.ipAddr();
  _mac = interface.hwAddr();
  }


namespace service {
  namespace atca {

/**
   @class Attributes
   @brief Encapsulate the ATCA attributes of an RCE.

   @fn Attributes::group() const
   @brief  The group name for this RCE.
   @return A pointer to the group name.
   @note Group name is typically the ATCA shelf name.

   @fn Attributes::cluster() const
   @brief The cluster ID for this RCE.
   @return The cluster ID.
   @note The cluster ID is typically the ATCA slot number.

   @fn Attributes::bay() const
   @brief The bay number for this RCE
   @return The bay number.

   @fn Attributes::element() const
   @brief The element number for this RCE.
   @return The element number.

   @fn Attributes::ip() const
   @brief The IP address of the first interface in network byte order as a 32 bit number.
   @return The IP address in network byte order.

   @fn Attributes::mac() const
   @brief The MAC address of the first interface in network byte order
   @return The MAC address in network byte order.

 */


  }
}
