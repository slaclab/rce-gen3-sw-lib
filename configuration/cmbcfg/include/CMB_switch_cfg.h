// -*-Mode: C;-*-
/* 
@file CMB_switch_cfg.h
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:


@par Abstract:
This file contains the routines which will assemble and transmit the
bootstrap parameters to the CMB Cluster Elements.

@author
Gregg Thayer <jgt@slac.stanford.edu>

@par Date created:
2012/04/05

@par Last commit:
\$Date: 2014-12-12 09:39:59 -0800 (Fri, 12 Dec 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3849 $

@par Credits:
SLAC
*/

#ifndef CMB_SWITCH_CFG_H
#define CMB_SWITCH_CFG_H

/* ---------------------------------------------------------------------- */
/*
  These are the utility values for extracting fields from the 
  Cluster Switch configuration words.
*/
/* ---------------------------------------------------------------------- */

/* This is the number of switch ports in a cluster */
enum { BSI_N_CLUSTER_PORTS = 24 };

/* These are the configuration types for the ports on the Fulcrum */
typedef enum _CMB_SWITCH_PORT_TYPE
{
   CMB_SWITCH_PORT_TYPE_NC              =  0,
   CMB_SWITCH_PORT_TYPE_DISABLED        =  1,
   CMB_SWITCH_PORT_TYPE_1000BASEX_LANE0 =  2,
   CMB_SWITCH_PORT_TYPE_1000BASEX_LANE1 =  3,
   CMB_SWITCH_PORT_TYPE_1000BASEX_LANE2 =  4,
   CMB_SWITCH_PORT_TYPE_1000BASEX_LANE3 =  5,
   CMB_SWITCH_PORT_TYPE_XAUI            =  6,
   CMB_SWITCH_PORT_TYPE_10GBASEX_LANE0  =  7,
   CMB_SWITCH_PORT_TYPE_10GBASEX_LANE1  =  8,
   CMB_SWITCH_PORT_TYPE_10GBASEX_LANE2  =  9,
   CMB_SWITCH_PORT_TYPE_10GBASEX_LANE3  = 10,
} CMB_SWITCH_PORT_TYPE;

typedef enum _CMB_SWITCH_PORT_GROUP
{
   CMB_SWITCH_PORT_GROUP_NONE = 0
} CMB_SWITCH_PORT_GROUP;

/*  These are the configuration types for the Lanes as reported by the RCE */
typedef enum _RCE_NET_PHY_TYPE
{
   RCE_NET_PHY_TYPE_UNUSED    = 0,
   RCE_NET_PHY_TYPE_1000BASEX = 1,
   RCE_NET_PHY_TYPE_10GBASEX  = 2,
   RCE_NET_PHY_TYPE_XAUI      = 3,
} RCE_NET_PHY_TYPE;

typedef union _RCE_net_phy_type
{
   uint8_t  lane[4];
   uint32_t u32;
} RCE_net_phy_type;

#endif /* CMB_SWITCH_CFG_H */
