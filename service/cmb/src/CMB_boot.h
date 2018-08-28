// -*-Mode: C;-*-
/*
@file CMB_boot.h
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:


@par Abstract:
Define the structures that will comprise the boot information
which is assembled from the various eeproms (CMB, FTM, RTM) and then
passed to the cluster elements to configure their booting process.


@author
Gregg Thayer <jgt@slac.stanford.edu>

@par Date created:
2012/01/03

@par Credits:
SLAC
*/

#ifndef CMB_BOOT_H
#define CMB_BOOT_H

#include <stdint.h>
#include "CMB_common.h"

/*************************************************************************
*************************************************************************/

/*
  This is the definition of the "bootstrap" word which is passed to
  the cluster element before booting. TBD
*/
typedef struct _CMB_bootstrap_bf {
  uint32_t bootstrap_params:16;
  uint32_t bootstrap:12;
  uint32_t schema:4;
} CMB_bootstrap_bf;

typedef union _CMB_bootstrap {
  uint32_t         u32;
  CMB_bootstrap_bf bf;
} CMB_bootstrap;

/*
  This is the "cluster address" word which is passed to the cluster
  element before booting. For the purpose of a generic CMB, this is
  just a 16 bit word. Any substructure is implementation dependent.
*/
typedef struct _CMB_cluster_addr {
  uint16_t u16;
} CMB_cluster_addr;

typedef struct _CMB_version_claddr_bf {
   CMB_cluster_addr claddr;
   uint16_t         version;
} CMB_version_claddr_bf;

typedef union _CMB_version_claddr {
   uint32_t u32;
   CMB_version_claddr_bf bf;
} CMB_version_claddr;

/*
  These are the "ip options" which are passed to the cluster
  element before booting.
*/
typedef struct _CMB_ip_opts_bf {
  uint32_t subnet_prefix:8;
  uint32_t dns_valid:1;
  uint32_t ntp_valid:1;
  uint32_t gateway_valid:1;
  uint32_t base_valid:1;
  uint32_t reserved1:12;
  uint32_t ipv6:1;
  uint32_t reserved2:7;
} CMB_ip_opts_bf;

typedef union _CMB_ip_opts {
   uint32_t u32;
   CMB_ip_opts_bf bf;
} CMB_ip_opts;

/*
  These are IP address definition, sized for either ipv4 or ipv6
 */
typedef struct _CMB_ipv4_addr {
  uint32_t addr;
  uint32_t mbz[3];
} CMB_ipv4_addr;

typedef struct _CMB_ipv6_addr {
  uint32_t addr[4];
} CMB_ipv6_addr;

typedef union _CMB_ip_address {
  CMB_ipv4_addr ipv4;
  CMB_ipv6_addr ipv6;
} CMB_ip_address;

/*
  This is the "ip information" which is passed to the cluster
  element before booting.
*/
typedef struct _CMB_ip_info {
  CMB_ip_address base;
  CMB_ip_address gateway;
  CMB_ip_address dns;
  CMB_ip_address ntp;
} CMB_ip_info;

/*
  This is the structure that in one form or another will be passed to
  each processing element over its I2C slave interface to allow it to
  boot.
*/
typedef struct _CMB_celement_boot_cfg {
   CMB_bootstrap      bootstrap;
   CMB_version_claddr version_claddr;
   CMB_mac_cfg        mac;
   uint32_t           mbz[4];
   CMB_ip_opts        ip_opts;
   CMB_ip_info        ip_info;
   CMB_endpt_type     int_connections[CMB_INT_LUT];
   CMB_endpt_type     ext_connections[CMB_EXT_LUT];
} CMB_celement_boot_cfg;

void CMB_print_boot_cfg(CMB_celement_boot_cfg* cfg);

#endif // CMB_BOOT_H
