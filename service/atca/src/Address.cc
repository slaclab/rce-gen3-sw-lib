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


#include "atca/Address.hh"

#include <string.h>
#include <stdio.h>

using namespace service::atca;

const char* ADDRESS_CONTROL1 = "%d";
const char* ADDRESS_CONTROL2 = "%2d";
const char* ADDRESS_CONTROL3 = "%1d";


/** @brief Address constructor taking individual address components.
    @param shelf The shelf name.
    @param slot The slot number that the COB is inserted to.
    @param cmb The CMB where the RCE is.
    @param element Which RCE in the CMB.
*/
Address::Address(const char* shelf, uint32_t slot, uint32_t cmb, uint32_t element) :
 _element(element),
 _cmb(cmb),
 _slot(slot)
 {
 
 _string[0] = 0;
  
 strncpy(_shelf, shelf, MAX_SHELF_STRING);
 
 _bound_range();
 
 }

/** @brief Constructor taking a presentation string as an argument
    @param presentation The presentation string.
 */
Address::Address(const char* presentation) : 
 _element(UNDEFINED),
 _cmb(UNDEFINED),
 _slot(UNDEFINED)
 {

 _string[0] = 0;
 
 strncpy(_shelf, presentation, MAX_PRESENTATION_STRING);

 char* slot = _terminate(_shelf);
  
  if(!slot) return;
  
  char* cmb = _terminate(slot);
  sscanf(slot, ADDRESS_CONTROL1, &_slot);
  
  if(cmb)
    {
    char* element = _terminate(cmb); 
    sscanf(cmb, ADDRESS_CONTROL1, &_cmb);
    
    if(element)
      {
      _terminate(element); 
      sscanf(element, ADDRESS_CONTROL1, &_element);
      }
    else
      _element = 1;
    }
  else
    {
    _cmb     = 1;
    _element = 1;
    }
  
 _bound_range();
 
 return;
 }

/** @brief return the presentation string for this address
    @return A pointer to the compiled string.
 */
const char* Address::presentation()
  {
  
  if(!isValid()) return _string;
  
  char* next = strcpy(_string, _shelf);
  
  next += strlen(next);
  
  *next++ = '/';
  
  next = _convert(_slot, _slot > 9 ? ADDRESS_CONTROL2 : ADDRESS_CONTROL3, next);
  
  *next++ = '/';
  
  next = _convert(_cmb, ADDRESS_CONTROL3, next);
  
  *next++ = '/';
  
  next = _convert(_element, ADDRESS_CONTROL3, next);
  
  return _string;
  }
  
/** @internal
    @brief do bound and range checking.
    If either element, cmb or slot is out of bounds, the value is set to
    UNDEFINED.
    @endinternal
 */
void Address::_bound_range()
{
  
  if(_element > MAX_ELEMENT) _element = UNDEFINED;   
  if(_cmb     > MAX_CMB)     _cmb     = UNDEFINED;
  if(_slot    > MAX_SLOT)    _slot    = UNDEFINED;
  
  return;
}

/** @internal
    @brief Replace the first slash with a NULL to terminate a string.
    @param string Input buffer
    @return pointer one past the replaced NULL or NULL if a / wasn't found.
    @endinternal
 */
char* Address::_terminate(char* string)
{

  char* term  = string;
  char  next  = *term++;
  
  while(next && (next != '/')) next = *term++;
  
  *(term - 1) = 0;
  
  return next ? term : (char*)0;
}
  
/** @internal
    @brief pretty-print a value into a string using a control format statement
    @param value   Value to pp
    @param control  Format string
    @param string  Buffer to print to
    @return Either a pointer to the end of the new string, or the original string if the printing failed.
    @endinternal
 */
char* Address::_convert(uint32_t value, const char* control, char* string)
{
  
  int length = sprintf(string, control, value);
  
  return (length > 0) ? string + length : string;
}
 

namespace service {
  namespace atca {
/**
   @class Address
   @brief Express an RCE's location in an ATCA shelf.

   This class provides a way to express the location of an RCE on a
   COB in an ATCA shelf.  This location is expressed in the 'RCE Coordinate Space'.

   All addresses in the RCE coordinate space are of the form
   shelf/slot/cmb/element where shelf is the logical name of the shelf,
   slot is the physical slot number of the shelf where the COB is
   located, cmb is the "COB Mezannine Board" bay where the individual
   board is located, and rce is the instance of the processor on the
   particular CMB.  This printed form of the Address is called the
   "presentation".


   @fn Address::isValid() const
   @brief Is the address valid (are the bounds satisfied)?
   @return true if the address is valid

   @fn Address::shelf() const
   @return The shelf name
   shelf is an arbitrary const char* (max length = Address::MAX_SHELF_STRING).
   This must be a legal ATCA shelf name encoded as 8-bit ASCII/Latin-1.

   @fn Address::slot() const
   @return The slot number.
   slot goes from 1 to a max of Address::MAX_SLOT.  This maximum is based on the ATCA
   specification.

   @fn Address::cmb() const
   @return The COB Mezannine board number.
   cmb goes from 0 to Address::MAX_CMB to match the five bays of a COB.

   @fn Address::element() const
   @return The element index inside the CMB (also known as the RCE index).
   element goes from 0 to Address::MAX_ELEMENT.  If there are only two RCEs in a CMB, their
   indexes are 0 and 2.
   
*/
/**

   @var Address::MAX_SHELF_STRING
   @brief The maximum length of a shelf name.  From the ATCA spec.

   @var Address::MAX_PRESENTATION_STRING
   @brief The maximum length of a presentation string.

   @var Address::MAX_SLOT
   @brief The maximum slot number (from the ATCA spec)

   @var Address::MAX_CMB
   @brief The maximum number of Mezannine Boards in a COB

   @var Address::MAX_ELEMENT
   @brief The maximum number of elements in a CMB
   
   @var Address::UNDEFINED
   @brief Used when slot/element/cmb are out of range.
   
   @section examples Examples of presentation strings.

   - tinker/4/3/2
   - cscnrc/1/0/0
   - formica_73-alpha/1/4/0

   
*/


  }
}
