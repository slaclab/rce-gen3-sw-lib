// -*-Mode: C;-*-
/* 
@file CMB_endpoint.h
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:


@par Abstract:
This defines structures that are used on both the Cluster Element
and the IPM Controller.

@author
Gregg Thayer <jgt@slac.stanford.edu>

@par Date created:
2012/01/03

@par Last commit:
\$Date: 2014-06-04 17:27:22 -0700 (Wed, 04 Jun 2014) $ by \$Author: panetta $.

@par Revision number:
\$Revision: 3344 $

@par Credits:
SLAC
*/

#ifndef CMB_ENDPOINT_H
#define CMB_ENDPOINT_H

#include <stdint.h>


/*************************************************************************  
    These structures define the postamble which is the same for all boards
    and is a variale length list of endpoint descriptions
*************************************************************************/
enum 
{
   CMB_EXT_LUT = 26, /* Number of external connections on a CMB */
   CMB_INT_LUT = 12  /* Number of internal connections on a CMB */
};

typedef enum _CMB_endpt_types
{
   ENDPT_UNUSED  =  0,
   LVTTL         =  1,
   LVCMOS33      =  2,
   LVCMOS25      =  3,
   LVCMOS18      =  4,
   LVCMOS15      =  5,
   LVCMOS12      =  6,
   PCI33_3       =  7,
   PCI66_3       =  8,
   PCI_X         =  9,
   GTLP          = 10,
   GTL           = 11,
   HSTL_I        = 12,
   HSTL_1_12     = 13,
   HSTL_II       = 14,
   HSTL_III      = 15,
   HSTL_IV       = 16,
   DIFFHSTL_I    = 17,
   DIFFHSTL_2    = 18,
   SSTL2_I       = 19,
   SSTL2_II      = 20,
   DIFFSSTL2_I   = 21,
   DIFFSSTL2_II  = 22,
   SSTL18_I      = 23,
   SSTL18_II     = 24,
   DIFFSSTL18_I  = 25,
   DIFFSSTL18_II = 26,
   HT_25         = 27,
   LVDS_25       = 28,
   LVDSEXT_25    = 29,
   LVPECL_25     = 30
} CMB_endpt_types;


typedef struct _CMB_endpt 
{
  uint8_t type;
  uint8_t attr;
} CMB_endpt;

#endif // CMB_BOOT_H
