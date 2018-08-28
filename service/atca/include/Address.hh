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
 
#ifndef SERVICE_ATCA_ADDRESS
#define SERVICE_ATCA_ADDRESS

#include <inttypes.h>

namespace service {
namespace atca  {

class Address {
public:
  enum {MAX_SHELF_STRING        = 63};
  enum {MAX_PRESENTATION_STRING = (MAX_SHELF_STRING + 1 + 8)};
public:  
  enum {MAX_SLOT        = 16};
  enum {MAX_CMB         = 5};
  enum {MAX_ELEMENT     = 3};  
public:  
  enum {UNDEFINED = 64};
public:
  Address(const char* shelf, uint32_t slot, uint32_t cmb, uint32_t element);
  Address(const char* presentation);
public:
 ~Address() {}
public:
  const char* presentation(); 
public:
  bool isValid() const {return _slot;}
public:  
  const char* shelf()   const {return _shelf;}
  uint32_t    slot()    const {return _slot;}
  uint32_t    cmb()     const {return _cmb;}
  uint32_t    element() const {return _element;}
private:
  void  _bound_range();
  char* _terminate(char* string);
  char* _convert(uint32_t value, const char* control, char* string);
private:
  uint32_t _element;
  uint32_t _cmb;
  uint32_t _slot;
  char     _shelf[MAX_PRESENTATION_STRING];
  char     _string[MAX_PRESENTATION_STRING];
};

}}

#endif
