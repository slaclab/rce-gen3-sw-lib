/*
** ++
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Michael Huffer, SLAC (mehsys@slac.stanford.edu)
**
**  Creation Date:
**	000 - January 09, 2007
**
**  Revision History:
**	None.
**
** --
*/
 
#ifndef SERVICE_ATCA_ATTRIBUTES
#define SERVICE_ATCA_ATTRIBUTES
#include <inttypes.h>

namespace service {
namespace atca  {

class Attributes {
public: 
  Attributes(const char* group, uint32_t cluster, uint32_t bay, uint32_t element);
public:
 ~Attributes() {}
public:
  const char* group()   const {return _group;}
  uint32_t    cluster() const {return _cluster;}
  uint32_t    bay()     const {return _bay;}  
  uint32_t    element() const {return _element;}
public: 
  uint32_t ip()  const {return _ip;}
  uint64_t mac() const {return _mac;}   
private:
  const char* _group;
  uint32_t    _cluster;
  uint32_t    _bay;
  uint32_t    _element;
  uint64_t    _mac;
  uint32_t    _ip;
};

}}

#endif
