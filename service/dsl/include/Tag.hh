
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
 
#ifndef SERVICE_DSL_TAG
#define SERVICE_DSL_TAG

#include <inttypes.h>
#include <stddef.h>

namespace service {
namespace dsl   {

#define MAX_TAG_WORDS   24
#define MAX_TAG_BYTES  (24*sizeof(uint32_t))

class Tag {
public:
  Tag() {}
  Tag(const Tag& clone) 
    {
    value.w64[0]  = clone.value.w64[0];
    value.w64[1]  = clone.value.w64[1];
    value.w64[2]  = clone.value.w64[2];
    value.w64[3]  = clone.value.w64[3];
    value.w64[4]  = clone.value.w64[4];
    value.w64[5]  = clone.value.w64[5];
    value.w64[6]  = clone.value.w64[6];
    value.w64[7]  = clone.value.w64[7];
    value.w64[8]  = clone.value.w64[8];
    value.w64[9]  = clone.value.w64[9];
    value.w64[10] = clone.value.w64[10];
    value.w64[11] = clone.value.w64[11];
    }
    
public:
 ~Tag() {}
public:
  union {uint32_t w32[MAX_TAG_WORDS]; uint64_t w64[MAX_TAG_WORDS>>1];} value;
};

}}

#endif
