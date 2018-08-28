// -*-Mode: C++;-*-
/**
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
*/
#if !defined(SERVICE_DSLUTIL_REBOOTER_SPACE_HH)
#define      SERVICE_DSLUTIL_REBOOTER_SPACE_HH

#include "dslutil/AddressStar.hh"
#include "dsl/Space.hh"

namespace service {
  namespace atca {
    class Attributes;
  }
  namespace dsl {
    class Location;
    class Tag;
  }
}

namespace service {
  namespace dslutil {
    namespace rebooter {
      class Space : public service::dsl::Space {
      public:
        static const char* name;
      public:
        Space(const service::atca::Attributes*);
        virtual ~Space() {};
      public:
        const service::dsl::Location* lookup(const service::dsl::Tag*);

      private:
        void _reboot(uint32_t,uint32_t);
      private:
        AddressStar   _addr;
      };
    }
  }
}
#endif
