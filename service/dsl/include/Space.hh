
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
 
#ifndef SERVICE_DSL_SPACE
#define SERVICE_DSL_SPACE



#include "dsl/Tag.hh"
#include "dsl/Location.hh"

namespace service {
namespace dsl   {

class Space {
public: 
  Space(const char* name) : _name(name) {}
public:
  virtual ~Space() {}
public:
  const char* name() const {return _name;}
public:
  virtual const Location* lookup(const Tag*) = 0;
private:
  const char* _name;
};

}}

#endif
