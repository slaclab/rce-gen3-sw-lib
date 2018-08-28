// -*-Mode: C++;-*-
//                               Copyright 2015
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "conversion/impl/BSWP.ih"
#include "dslutil/UpdateBase.hh"

#define SHELF_IDX    0
#define SLOT_IDX     2
#define CMB_IDX      3
#define ELM_IDX      4
#define ID_IDX       5
#define IP_IDX       6
#define SPARE_IDX    7
#define SRC_IDX      8
#define DST_IDX      SRC_IDX+(MAX_PATH_LEN>>2)

namespace service {
  namespace dslutil {
    namespace updater {
      /**
       */

      Message::Message(const char* shelf_in,
                       uint8_t     slot_in,
                       uint8_t     cmb_in,
                       uint8_t     element_in,
                       uint8_t     os_in,
                       uint32_t    id_in,
                       uint64_t    status_in) :
        slot(slot_in), cmb(cmb_in), element(element_in), os(os_in), id(id_in), status(status_in)
      {
        strncpy(shelf, shelf_in, service::atca::Address::MAX_SHELF_STRING);
      }

      Tag::Tag(AddressStar* addr,
               uint32_t id,
               uint32_t ip,
               const char* src = NULL,
               const char* dst = NULL) :
        service::dsl::Tag()
      {
        value.w64[SHELF_IDX] = BSWP__swap64b(addr->shelf_hash());   
        value.w32[SLOT_IDX]  = BSWP__swap32b(addr->slot());         
        value.w32[CMB_IDX]   = BSWP__swap32b(addr->cmb());          
        value.w32[ELM_IDX]   = BSWP__swap32b(addr->element());      
        value.w32[IP_IDX]    = BSWP__swap32b(ip);                   
        value.w32[ID_IDX]    = id;
        
        bzero((char*)&value.w32[SRC_IDX], MAX_PATH_LEN);
        bzero((char*)&value.w32[DST_IDX], MAX_PATH_LEN);
        
        if(src)
          strncpy((char*)&value.w32[SRC_IDX], src, strlen(src));
        if(dst)          
          strncpy((char*)&value.w32[DST_IDX], dst, strlen(dst));
      }

      uint32_t Tag::id()
      {
        return value.w32[ID_IDX];
      }

      uint32_t Tag::ip()
      {
        return value.w32[IP_IDX];
      }

      const char* Tag::src()
      {
        return (const char*)&value.w32[SRC_IDX];
      }

      const char* Tag::dst()
      {
        return (const char*)&value.w32[DST_IDX];
      }

      AddressStar* Tag::addr(uint8_t* buffer)
      {
        AddressStar* result = new(buffer) AddressStar(BSWP__swap64b(value.w64[SHELF_IDX]),
                                                      BSWP__swap32b(value.w32[SLOT_IDX]),
                                                      BSWP__swap32b(value.w32[CMB_IDX]),
                                                      BSWP__swap32b(value.w32[ELM_IDX]));
        return result;
      }
    }
  }
}
