// -*-Mode: C++;-*-
/**
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
*/
#if !defined(SERVICE_DSLUTIL_IDENTIFIER_SPACE_HH)
#define      SERVICE_DSLUTIL_IDENTIFIER_SPACE_HH

#include "dslutil/Base.h"
#include "dslutil/IdBase.hh"
#include "dslutil/AddressStar.hh"
#include "dsl/Space.hh"
#include "dsl/Location.hh"

namespace service {
  namespace atca {
    class Attributes;
  }
  namespace dsl {
    class Tag;
  }
}

namespace service {
  namespace dslutil {
    namespace identifier {
      
      class Space : public service::dsl::Space {
      public:
        static const char* name;
      public:
        Space(const service::atca::Attributes*);
        virtual ~Space() {};
      public:
        const service::dsl::Location* lookup(const service::dsl::Tag*);

      public:
        void identify();

      private:
        uint32_t    _os();
        const char* _id();

      private:
        AddressStar                 _addr;
        service::dsl::Location      _loc;
        uint8_t                     _msg[DSLUTIL_MAX_MSG_SIZE];
        char                        _idstr[DSLUTIL_MAX_ID_STR];
      };
    }
  }
}
#endif
