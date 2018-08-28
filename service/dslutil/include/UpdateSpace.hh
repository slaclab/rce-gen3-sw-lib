// -*-Mode: C++;-*-
/**
                               Copyright 2015
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
*/
#if !defined(SERVICE_DSLUTIL_UPDATER_SPACE_HH)
#define      SERVICE_DSLUTIL_UPDATER_SPACE_HH

#include "dslutil/Base.h"
#include "dslutil/UpdateBase.hh"
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
    namespace updater {
      class Space : public service::dsl::Space {
      public:
        static const char* name;
      public:
        Space(const service::atca::Attributes*);
        virtual ~Space() {};
      public:
        const service::dsl::Location* lookup(const service::dsl::Tag*);

      private:
        void     _update(uint32_t id, uint32_t ip, const char *src, const char *dst);
        void     _post(uint32_t id, uint32_t status);
        void     _mount_boot(void);
        void     _mount_os(uint32_t mode);
        uint32_t _os();
        
      private:
        AddressStar   _addr;
        uint8_t       _msg[DSLUTIL_MAX_MSG_SIZE];
      };
    }
  }
}
#endif
