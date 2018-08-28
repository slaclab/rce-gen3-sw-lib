
/*
** ++
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Michael Huffer, SLAC <mehsys@slac.stanford.edu>
**
**  Creation Date:
**	000 - , 2007
**
**  Revision History:
**	None.
**
** --
*/
 
#ifndef SERVICE_ATCA_SPACE
#define SERVICE_ATCA_SPACE



#include "dsl/Space.hh"
#include "atca/Attributes.hh"

namespace service {
namespace atca   {

class Space : public dsl::Space {
public:
  Space(const atca::Attributes*);
public:
  virtual ~Space() {}
public: 
  const service::dsl::Location* lookup(const service::dsl::Tag*);
private:
  const atca::Attributes* _attributes;
  uint32_t                _slot;
  uint64_t                _shelf;
  uint8_t                 _result[sizeof(dsl::Location)];
};

}}

#endif
