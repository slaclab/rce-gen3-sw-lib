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
#ifndef SERVICE_CMB_BSI_INL_HH
#define SERVICE_CMB_BSI_INL_HH


#include "cmb/bsi.hh"
#include "cmb/bsi.hh"
#include "conversion/Endianness.h"
#include "conversion/Swap.hh"
#include "exception/Exception.hh"

// These are macros so that other macro names can be constructed
#define BSI_EXTRACT(b,s,v) { read(b##_OFFSET, &v); v = (v & (b##_M_##s)) >> (b##_V_##s); }
#define BSI_INSERT(b,s,v)  ((v << (b##_V_##s)) & (b##_M_##s))


namespace service {

  namespace cmb {

    inline
    BSI::BSI() :
      BsiIO()
    {
    }

    inline
    BSI::~BSI()
    {
    }

    inline const BSI* const
    BSI::instance()
    {
      return BSI::_bsi;
    }


    inline bool
    BSI::versionChk() const
    {
      return version() == configuration::cmb::BSI_K_VERSION;
    }

    inline uint32_t
    BSI::version() const
    {
      uint32_t value = 0;
      read(BSI_CLUSTER_CFG_VERSION_OFFSET, &value);
      return value;
    }

    inline void
    BSI::version(uint32_t value) const
    {
      write(BSI_CLUSTER_CFG_VERSION_OFFSET, value);
    }


    inline uint8_t
    BSI::network_phy() const
    {
      uint32_t value = 0;
      read(BSI_NETWORK_PHY_TYPE_OFFSET, &value);
      return (uint8_t)(value >> (8 * element()) & 0xff);
    }

    inline void
    BSI::network_phy(uint8_t value) const
    {
      uint32_t s = 8 * element();
      uint32_t v;
      read (BSI_NETWORK_PHY_TYPE_OFFSET, &v);
      write(BSI_NETWORK_PHY_TYPE_OFFSET, (v & ~(0xff << s)) | (value << s));
    }


    inline uint64_t
    BSI::mac() const
    {
      union
      {
        uint32_t u32[2];
        uint64_t u64;
      } value;

      read(BSI_MAC_ADDR_OFFSET,     &value.u32[0]);
      read(BSI_MAC_ADDR_OFFSET + 1, &value.u32[1]);

      return value.u64;
    }

    inline void
    BSI::mac(uint64_t value) const
    {
      union
      {
        uint64_t u64;
        uint32_t u32[2];
      } val = { value };

      write(BSI_MAC_ADDR_OFFSET,     val.u32[0]);
      write(BSI_MAC_ADDR_OFFSET + 1, val.u32[1]);
    }

    inline const char* const
    BSI::mac(char buffer[tool::string::MaxColon]) const
    {
      return tool::string::mac2colon(mac(), buffer);
    }


    inline uint32_t
    BSI::interconnect() const
    {
      uint32_t value = 0;
      read(BSI_INTERCONNECT_OFFSET, &value);
      return value;
    }

    inline void
    BSI::interconnect(uint32_t value) const
    {
      write(BSI_INTERCONNECT_OFFSET, value);
    }


    inline unsigned
    BSI::cluster() const
    {
      uint32_t value = 0;
      BSI_EXTRACT(BSI_CLUSTER_ADDR, CLUSTER, value);
      return value;
    }

    inline unsigned
    BSI::bay() const
    {
      uint32_t value = 0;
      BSI_EXTRACT(BSI_CLUSTER_ADDR, BAY, value);
      return value;
    }

    inline unsigned
    BSI::element() const
    {
      uint32_t value = 0;
      BSI_EXTRACT(BSI_CLUSTER_ADDR, ELEMENT, value);
      return value;
    }

    inline void
    BSI::cluster_addr(unsigned clus, unsigned bay, unsigned elem) const
    {
      uint32_t value = (BSI_INSERT(BSI_CLUSTER_ADDR, CLUSTER, clus) |
                        BSI_INSERT(BSI_CLUSTER_ADDR, BAY,     bay)  |
                        BSI_INSERT(BSI_CLUSTER_ADDR, ELEMENT, elem));
      write(BSI_CLUSTER_ADDR_OFFSET, value);
    }

    inline bool
    BSI::moduleIs(ModuleType type) const
    {
      unsigned   b = bay();
      ModuleType t;

      t =  ( b == CMB_K_BAY_0                       ) ? CMB_K_DTM :
          (((b >= CMB_K_BAY_1) && (b <= CMB_K_BAY_4)) ? CMB_K_DPM :
                                                        CMB_K_UNRECOGNIZED);
      return type == t;
    }


    inline uint64_t
    BSI::serial_number() const
    {
      union
      {
        uint32_t u32[2];
        uint64_t u64;
      } val;

      read(BSI_CMB_SERIAL_NUMBER_OFFSET,     &val.u32[0]);
      read(BSI_CMB_SERIAL_NUMBER_OFFSET + 1, &val.u32[1]);

      return val.u64;
    }

    inline void
    BSI::serial_number(uint64_t value) const
    {
      union
      {
        uint64_t u64;
        uint32_t u32[2];
      } val = { value };

      write(BSI_CMB_SERIAL_NUMBER_OFFSET,     val.u32[0]);
      write(BSI_CMB_SERIAL_NUMBER_OFFSET + 1, val.u32[1]);
    }


    inline unsigned
    BSI::_group(unsigned idx) const
    {
      uint32_t value = 0;
      read(BSI_CLUSTER_GROUP_NAME_OFFSET + idx, &value);
      return value;
    }

    inline void
    BSI::_group(unsigned idx, unsigned value) const
    {
      write(BSI_CLUSTER_GROUP_NAME_OFFSET + idx, value);
    }


    inline uint32_t
    BSI::ext_interconnect() const
    {
      uint32_t value = 0;
      read(BSI_EXTERNAL_INTERCONNECT_OFFSET, &value);
      return value;
    }

    inline void
    BSI::ext_interconnect(uint32_t value) const
    {
      write(BSI_EXTERNAL_INTERCONNECT_OFFSET, value);
    }


    inline uint32_t
    BSI::switch_config(unsigned idx) const
    {
      if (idx >= BSI_CLUSTER_SWITCH_CONFIGURATION_SIZE)
        throw tool::exception::Error("Out-of-bounds array index (%u >= %u)",
                                     idx, BSI_CLUSTER_SWITCH_CONFIGURATION_SIZE);

      uint32_t value = 0;
      read(BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + idx, &value);
      return value;
    }

    inline void
    BSI::switch_config(unsigned idx, uint32_t value) const
    {
      if (idx >= BSI_CLUSTER_SWITCH_CONFIGURATION_SIZE)
        throw tool::exception::Error("Out-of-bounds array index (%u >= %u)",
                                     idx, BSI_CLUSTER_SWITCH_CONFIGURATION_SIZE);

      write(BSI_CLUSTER_SWITCH_CONFIGURATION_OFFSET + idx, value);
    }


    inline uint32_t
    BSI::ce_interconnect(unsigned idx) const
    {
      if (idx >= BSI_CLUSTER_ELEMENT_INTERCONNECT_SIZE)
        throw tool::exception::Error("Out-of-bounds array index (%u >= %u)",
                                     idx, BSI_CLUSTER_ELEMENT_INTERCONNECT_SIZE);

      uint32_t value = 0;
      read(BSI_CLUSTER_ELEMENT_INTERCONNECT_OFFSET + idx, &value);
      return value;
    }

    inline void
    BSI::ce_interconnect(unsigned idx, uint32_t value) const
    {
      if (idx >= BSI_CLUSTER_ELEMENT_INTERCONNECT_SIZE)
        throw tool::exception::Error("Out-of-bounds array index (%u >= %u)",
                                     idx, BSI_CLUSTER_ELEMENT_INTERCONNECT_SIZE);

      write(BSI_CLUSTER_ELEMENT_INTERCONNECT_OFFSET + idx, value);
    }


    inline unsigned
    BSI::network_prefix() const
    {
      uint32_t value = 0;
      //read(BSI_NETWORK_PREFIX_OFFSET, &value);
      return value;
    }

    inline void
    BSI::network_prefix(unsigned value) const
    {
      //write(BSI_NETWORK_PREFIX_OFFSET, value);
    }

    inline uint32_t
    BSI::nm() const
    {
      return -1 << (32 - network_prefix());
    }

    inline void
    BSI::nm(uint32_t value) const
    {
      int np = 32;
      do
      {
        if (value & 1)  break;
        value >>= 1;
      }
      while (--np);
      network_prefix(np);
    }

    inline const char* const
    BSI::nm(char buffer[tool::string::MaxDotted]) const
    {
      return tool::string::ip2dot(nm(), buffer);
    }


    inline uint32_t
    BSI::ip() const
    {
      return ip(element());
    }

    inline uint32_t
    BSI::ip(unsigned idx) const
    {
      uint32_t value = 0;
      //read(BSI_CLUSTER_IP_ADDR_ARRAY_OFFSET(size()) + idx, &value);
      return value;
    }

    inline void
    BSI::ip(unsigned idx, uint32_t value) const
    {
      //write(BSI_CLUSTER_IP_ADDR_ARRAY_OFFSET(size()) + idx, value);
    }

    inline const char* const
    BSI::ip(char buffer[tool::string::MaxDotted]) const
    {
      return tool::string::ip2dot(ip(), buffer);
    }


    inline uint32_t
    BSI::gw() const
    {
      uint32_t value = 0;
      //read(BSI_GATEWAY_IP_OFFSET, &value);
      return value;
    }

    inline void
    BSI::gw(uint32_t value) const
    {
      //write(BSI_GATEWAY_IP_OFFSET, value);
    }

    inline const char* const
    BSI::gw(char buffer[tool::string::MaxDotted]) const
    {
      return tool::string::ip2dot(gw(), buffer);
    }


    inline uint32_t
    BSI::logger() const
    {
      uint32_t value = 0;
      //read(BSI_LOGGER_IP_OFFSET, &value);
      return value;
    }

    inline void
    BSI::logger(uint32_t value) const
    {
      //write(BSI_LOGGER_IP_OFFSET, value);
    }

    inline const char* const
    BSI::logger(char buffer[tool::string::MaxDotted]) const
    {
      return tool::string::ip2dot(logger(), buffer);
    }


    inline uint32_t
    BSI::ntp() const
    {
      uint32_t value = 0;
      //read(BSI_NTP_SERVER_IP_OFFSET, &value);
      return value;
    }

    inline void
    BSI::ntp(uint32_t value) const
    {
      //write(BSI_NTP_SERVER_IP_OFFSET, value);
    }

    inline const char* const
    BSI::ntp(char buffer[tool::string::MaxDotted]) const
    {
      return tool::string::ip2dot(ntp(), buffer);
    }


    inline uint32_t
    BSI::dns() const
    {
      uint32_t value = 0;
      //read(BSI_DNS_SERVER_IP_OFFSET, &value);
      return value;
    }

    inline void
    BSI::dns(uint32_t value) const
    {
      //write(BSI_DNS_SERVER_IP_OFFSET, value);
    }

    inline const char* const
    BSI::dns(char buffer[tool::string::MaxDotted]) const
    {
      return tool::string::ip2dot(dns(), buffer);
    }


    inline unsigned
    BSI::domain(unsigned idx) const
    {
      uint32_t value = 0;
      //read(BSI_DOMAIN_NAME_OFFSET + idx, &value);
      return value;
    }

    inline void
    BSI::domain(unsigned idx, unsigned value) const
    {
      //write(BSI_DOMAIN_NAME_OFFSET + idx, value);
    }


    inline void
    BSI::booted(unsigned value) const
    {
      write(BSI_BOOT_RESPONSE_OFFSET, value);
    }

    inline void
    BSI::booted() const
    {
      booted(BSI_BOOT_RESPONSE_SUCCESS);
    }

  }

}
#endif
