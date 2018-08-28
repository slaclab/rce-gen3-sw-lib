// -*-Mode: C++;-*-
/**
@cond development
@file rtems/Interface.hh
@brief Encapsulate socket semantics for reading and writing.
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
 
#ifndef SERVICE_NET_INTERFACE_RTEMS
#define SERVICE_NET_INTERFACE_RTEMS

#define __BSD_VISIBLE 1
#include <sys/socket.h> // For AF_LINK
#undef  __BSD_VISIBLE

#include <net/if_dl.h>

#define LOOPBACK_NAME "lo0"

#endif

/** @endcond */
