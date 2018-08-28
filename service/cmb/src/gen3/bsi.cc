/*!@file     bsi.hh
*
* @brief     BootStrap Interface class implementation
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      March 2, 2012 -- Created
*
* $Revision: 3123 $
*
* @verbatim                    Copyright 2012
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#include <stdio.h>
#include <string.h>


#include "cmb/bsi.hh"
#include "cmb/bsi.hh"


namespace service {

  namespace cmb {

    static BSI __bsi;                   // Reserve space; too early to new()
    BSI* BSI::_bsi = 0;

    void
    BSI::initialize()
    {
      BSI::_bsi = &__bsi;               // Point to the reserved space

      *BSI::_bsi = BSI();               // Initialize it by running the ctor
    }


    const uint8_t* const
    BSI::mac(uint8_t value[6]) const
    {
      uint64_t u64 = mac();
      value[0] = (u64 >> 40) & 0xff;
      value[1] = (u64 >> 32) & 0xff;
      value[2] = (u64 >> 24) & 0xff;
      value[3] = (u64 >> 16) & 0xff;
      value[4] = (u64 >>  8) & 0xff;
      value[5] = (u64      ) & 0xff;
      return value;
    }


    const char* const
    BSI::group(char value[BSI_GROUP_NAME_SIZE * sizeof(unsigned)]) const
    {
      unsigned* val = (unsigned*)value;
      unsigned  idx = 0;
      do {
        *val++ = _group(idx);
      }
      while (++idx < BSI_GROUP_NAME_SIZE - 1);
      return value;
    }

    void
    BSI::group(const char* const value) const
    {
      unsigned  len = (strnlen(value, 4 * BSI_GROUP_NAME_SIZE) + 3) / 4;
      unsigned* val = (unsigned*)value;
      unsigned  idx = 0;
      do {
        _group(idx, *val++);
      }
      while (++idx < len);
    }


    const char* const
    BSI::domain(char value[BSI_DOMAIN_NAME_SIZE * sizeof(unsigned)]) const
    {
      unsigned* val = (unsigned*)value;
      unsigned  idx = 0;
      do
      {
        *val++ = domain(idx);
      }
      while (++idx < BSI_DOMAIN_NAME_SIZE - 1);
      return value;
    }


    void
    BSI::dump() const
    {
      char grp_[BSI_GROUP_NAME_SIZE  * sizeof(unsigned)];
      char dmn_[BSI_DOMAIN_NAME_SIZE * sizeof(unsigned)];

      // Break this up into multiple prints or else things get cut off
      printf("\nBSI %s:  Serial no. %04x%08x:\n",
             __func__,
             unsigned(serial_number()>>32),
             unsigned(serial_number()&0xffffffffU));
      printf("  version %08lx, cluster %02x, bay %02x, element %02x\n",
             version(), cluster(), bay(), element());
      printf("  group '%s', network /%u, domain '%s', mac %04x%08x\n",
             group(grp_), network_prefix(), domain(dmn_),
             unsigned(mac()>>32), unsigned(mac()&0xffffffff) );
      printf("  nm %08lx, gw %08lx, logger %08lx, ntp %08lx, dns %08lx, ip %08lx\n",
             nm(), gw(), logger(), ntp(), dns(), ip());
      printf("\n");
    }


    void
    BSI::bram(uint32_t* buffer)
    {
      BsiIO    io;
      unsigned i = 0;
      do
      {
        io.read(i++, buffer++);
      }
      while (i < BSI_CFG_SIZE >> 2);
    }

    void
    BSI::bram(uint32_t* buffer, unsigned length)
    {
      BsiIO    io;
      unsigned i = 0;
      length >>= 2;                     // # of longwords

      while (i < length)
      {
        io.write(i++, *buffer++);
      }
    }
  }

}
