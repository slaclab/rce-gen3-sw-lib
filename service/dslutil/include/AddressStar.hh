// -*-Mode: C++;-*-
/**
@file AddressStar.hh
@brief 
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#if !defined(SERVICE_DSLUTIL_ADDRESSSTAR_HH)
#define      SERVICE_DSLUTIL_ADDRESSSTAR_HH

#include <stdint.h>
#include <stddef.h>

#include "atca/Address.hh"

namespace service {
  namespace dslutil {
    
    class AddressStar {
    public:
      
    public:
      AddressStar(const char*, uint32_t, uint32_t, uint32_t);
      AddressStar(uint64_t, uint32_t, uint32_t, uint32_t);
      AddressStar(const char*);
      ~AddressStar() {}

    public:
      const char* shelf()      const {return _shelf;}
      uint32_t    slot()       const {return _slot;}
      uint32_t    cmb()        const {return _cmb;}
      uint32_t    element()    const {return _element;}
      uint64_t    shelf_hash() const {return _shelf_hash;}

    public:
      bool match(service::atca::Address*);
      bool match(AddressStar*);

    public:
      void* operator new(size_t size, void* buffer) {return buffer;}


    private:
      void _range_check();
      bool _match_shelf(const char*);
      bool _match_shelf_hash(uint64_t);
      bool _match_slot(uint32_t);
      bool _match_cmb(uint32_t);
      bool _match_element(uint32_t);
      
    private:
      uint32_t      _element;
      uint32_t      _cmb;
      uint32_t      _slot;
      uint64_t      _shelf_hash;
      char          _shelf[service::atca::Address::MAX_SHELF_STRING];
    };    
  }
}
#endif
