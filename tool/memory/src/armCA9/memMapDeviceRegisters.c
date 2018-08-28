// -*-Mode: C;-*-
/**
@file
@brief Map and unmap the device registers assuming virtual==real.
@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
DAT

@author
Stephen Tether <tether@slac.stanford.edu>

@par Date created:
2013/10/23

@par Last commit:
\$Date: 2014-06-24 12:59:09 -0700 (Tue, 24 Jun 2014) $ by \$Author: tether $.

@par Revision number:
\$Revision: 3413 $

@par Location in repository:
\$HeadURL: file:///afs/slac.stanford.edu/g/cci/repositories/ctkrepo/core_tags/prod/V2.4.2/tool/memory/src/armCA9/memMapDeviceRegisters.c $

@par Credits:
SLAC
*/

#include "memory/cpu/mem.h"

uint32_t  mem_mapL2Controller(void) {return MEM_L2CC_PHYSICAL_BASE;}

void mem_unmapL2Controller(void) {}

uint32_t mem_mapSlcr(void) {return MEM_SLCR_PHYSICAL_BASE;}

void mem_unmapSlcr(void) {}

