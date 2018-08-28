/*!@file     bsi.hh
*
* @brief     BootStrap Interface class definition
*
*            This class provides access to the bootstrap configuration
*            information that is loaded into a 4 KB block RAM at CMB
*            (COB Mezzanine Board) reset time by the IPMI controller over
*            the I2C bus.
*
*            Most of the methods of this class are self explanitory.  For
*            each element in the BSI structure, there is a 'get' method
*            that is characterized by not having any arguments and the
*            return of the structure element's value, and a 'set' method
*            that is characterized by having an argument that contains the
*            value to set the structure element to and returning a void.
*
*            Since the interactions with the I2C are 32 bit word based
*            there are also helper methods that convert these words into
*            a more natural forms.
*
*            Methods are also provided that load/copy the entire
*            structure to/from BRAM.
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
#ifndef SERVICE_CMB_GEN3_BSI_HH
#define SERVICE_CMB_GEN3_BSI_HH


#include "cmb/cmb.hh"
#include "cmb/BSI_cfg.h"
#include "string/ip2dot.hh"
#include "string/mac2colon.hh"
#include "string/id2node.hh"
#include "cmb/gen/bsiIO.hh"


namespace service {

  namespace cmb {

    class BSI : private BsiIO
    {
    public:
      BSI();
      BSI(uint32_t* base) : BsiIO(base) {};
      ~BSI();

    public:
      bool                 versionChk()                                 const;
      uint32_t             version()                                    const;
      void                 version(uint32_t verion)                     const;

      uint8_t              network_phy()                                const;
      void                 network_phy(uint8_t val)                     const;

      uint64_t             mac()                                        const;
      uint64_t             mac(unsigned idx)                            const;
      void                 mac(uint64_t value)                          const;
      void                 mac(unsigned idx, uint64_t value)            const;
      const uint8_t* const mac(uint8_t val[6])                          const;
      const char*    const mac(char buf[tool::string::MaxColon])        const;

      uint32_t             interconnect()                               const;
      void                 interconnect(uint32_t val)                   const;

      uint64_t             serial_number()                              const;
      void                 serial_number(uint64_t val)                  const;

      unsigned             cluster()                                    const;
      unsigned             bay()                                        const;
      unsigned             element()                                    const;
      void                 cluster_addr(unsigned clus,
                                        unsigned bay,
                                        unsigned elem)                  const;

      bool                 moduleIs(ModuleType type)                    const;

      const char*    const group(char val[BSI_GROUP_NAME_SIZE *
                                          sizeof(unsigned)])            const;
      void                 group(const char* const value)               const;
    private:
      unsigned             _group(unsigned idx)                         const;
      void                 _group(unsigned idx, unsigned val)           const;

    public:
      uint32_t             ext_interconnect()                           const;
      void                 ext_interconnect(uint32_t val)               const;

      uint32_t             switch_config(unsigned idx)                  const;
      void                 switch_config(unsigned idx, uint32_t val)    const;

      uint32_t             ce_interconnect(unsigned idx)                const;
      void                 ce_interconnect(unsigned idx, uint32_t val)  const;

    public:
      unsigned             network_prefix()                             const;
      void                 network_prefix(unsigned value)               const;
      uint32_t             nm()                                         const;
      void                 nm(uint32_t value)                           const;
      const char*    const nm(char buffer[tool::string::MaxDotted])     const;

      uint32_t             ip()                                         const;
      uint32_t             ip(unsigned idx)                             const;
      void                 ip(unsigned idx, uint32_t value)             const;
      const char*    const ip(char buffer[tool::string::MaxDotted])     const;

      uint32_t             gw()                                         const;
      void                 gw(uint32_t value)                           const;
      const char*    const gw(char buffer[tool::string::MaxDotted])     const;
      uint32_t             logger()                                     const;
      void                 logger(uint32_t value)                       const;
      const char*    const logger(char buffer[tool::string::MaxDotted]) const;
      uint32_t             ntp()                                        const;
      void                 ntp(uint32_t value)                          const;
      const char*    const ntp(char buffer[tool::string::MaxDotted])    const;
      uint32_t             dns()                                        const;
      void                 dns(uint32_t value)                          const;
      const char*    const dns(char buffer[tool::string::MaxDotted])    const;

      const char*    const domain(char value[BSI_DOMAIN_NAME_SIZE *
                                             sizeof(unsigned)])         const;
      unsigned             domain(unsigned idx)                         const;
      void                 domain(unsigned idx, unsigned value)         const;


    public:
      void                 booted()                                     const;
      void                 booted(unsigned val)                         const;

    public:
      void                 dump()                                       const;

    public:
      static void          bram(uint32_t* buf);
      static void          bram(uint32_t* buf, unsigned len);

    public:
      static
      void                 initialize();
      static
      const BSI* const     instance();

    private:
      static BSI* _bsi;
    };

  }

}

// Include the code to be inlined
#include "cmb/gen/bsi-inl.hh"
#endif
