// -*-Mode: C++;-*-
/*!
*
* @brief   Print functions
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 25 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef CM_SERVICE_PRINT_HH
#define CM_SERVICE_PRINT_HH

namespace cm {
  namespace service {
    void print_init(const char* src);
    void prints(const char* msg);
    void printv(const char* msgformat, ...);
  }; // debug
}; // service

#endif // CM_SERVICE_PRINT_HH
