
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
 
#ifndef TOOL_CSI_LINK
#define TOOL_CSI_LINK

#include <inttypes.h>

namespace tool {
namespace csi   {

class Link {
public: 
  Link() {}
public:
  virtual ~Link() {}
public:
  uint32_t usecs(uint32_t tics) const {return tics << 10;}
public:  
  virtual void  post(void* packet, uint32_t length) = 0;
  virtual void* wait(uint32_t tmo)                  = 0; 
};

}}

#endif
