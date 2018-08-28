/*!@file   i2c.h
*
* @brief   Definition of I2C space for Gen 2 RCE boards
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    November 10, 2011 -- Created
*
* $Revision: 1358 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef _I2C_H_
#define _I2C_H_

#include "datCode.hh"
#include DAT_GENERATION(bootstrap, common, i2cIO.h)

#define I2C_FIRST     0x000

#define I2C_BOOTSTRAP 0x000           // Fmt vers | bootLoader opts | boot opts

#define I2C_LAST      0x3ff
#endif
