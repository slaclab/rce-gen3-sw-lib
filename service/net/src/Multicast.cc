// -*-Mode: C++;-*-
/**
@cond development
@file Multicast.cc
@brief Brief description of file contents and purpose here.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include "net/Multicast.hh"

/** @endcond */
// =========== Documentation ================

namespace service {

  namespace net {
/**
   @class Multicast
   @brief Encapsulate the semantics for an IP multicast.


   @fn Multicast::Multicast(uint32_t mcst, uint16_t port,  uint32_t interface)
   @brief Set up an IP multicast on a particular interface.
   @param[in] mcst The multicast address (network order).
   @param[in] port The multicast port (host order).
   @param[in] interface The IP address of the interface to bind to (network order).

   
   @fn Multicast::Multicast(uint32_t mcst, uint16_t port)                     
   @brief Set up an IP multicast.
   @param[in] mcst The multicast address (network order).
   @param[in] port The multicast port (host order).

   
   @fn Multicast::Multicast(const char* mcst, uint16_t port,  uint32_t interface)
   @brief Set up an IP multicast on a particular interface.
   @param[in] mcst A pointer to the multicast address string in dot notation.
   @param[in] port The multicast port (host order).
   @param[in] interface The IP address of the interface to bind to (network order).

   
   @fn Multicast::Multicast(const char* mcst, uint16_t port)                     
   @brief Set up an IP multicast.
   @param[in] mcst A pointer to the multicast address string in dot notation.
   @param[in] port The multicast port (host order).

   
   @fn uint32_t Multicast::interface() const
   @brief Return the IP address of the interface (network order).
   @return The IP address of the interface (network order).

*/
  }

}
