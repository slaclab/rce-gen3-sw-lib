// -*-Mode: C++;-*-
/*!
*
* @brief   Macro for formatting messages
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
#ifndef SERVICE_LOGGER_FORMATMESSAGE_HH
#define SERVICE_LOGGER_FORMATMESSAGE_HH

#include <stdarg.h>
#include <stdio.h>

// this macro saves string msgformat to msg using function variable arg list

#define FORMAT_MESSAGE(msgformat, maxlen, msg, len) \
{ \
  va_list ap; \
  va_start(ap, msgformat); \
  len = vsnprintf(msg, maxlen, msgformat, ap); \
  va_end(ap); \
}

#endif // SERVICE_LOGGER_FORMATMESSAGE_HH
