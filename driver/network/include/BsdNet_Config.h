// -*-Mode: C;-*-
/**
@file
@brief 

This file contains the definition of the "BsdNet_Attributes" structure. Instances of this
structure are used to specify the attributes the network driver.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
NETWORK

@author
Michael Huffer, SLAC (mehsys@slac.stanford.edu)

@par Date created:
2014/01/30

@par Last commit:
\$Date: 2013-10-07 18:56:03 -0700 (Mon, 07 Oct 2013) $ by \$Author: tether $.

@par Revision number:
\$Revision: 2191 $

@par Credits:
SLAC

*/

#ifndef BSDNET_CONFIG_H
#define BSDNET_CONFIG_H

#include "rtems.h"

typedef struct {
  const char*         hostname;
  const char*         domainname;
  const char*         gateway;
  const char*         log_host;
  const char*         name_server1;
  const char*         name_server2;
  const char*         name_server3;
  const char*         ntp_server1;
  const char*         ntp_server2;
  const char*         ntp_server3;
  bool                use_dhcp;
  rtems_task_priority priority;
  uint32_t            sb_efficiency;
  uint32_t            udp_tx_buf_size;
  uint32_t            udp_rx_buf_size;
  uint32_t            tcp_tx_buf_size;
  uint32_t            tcp_rx_buf_size;
  uint32_t            mbuf_bytecount;
  uint32_t            mbuf_cluster_bytecount;
  } BsdNet_Attributes;    

#if defined (__cplusplus)
extern "C" {
uint32_t BsdNet_Configure(BsdNet_Attributes *prefs, void* attachFn);
}

#endif
#endif
