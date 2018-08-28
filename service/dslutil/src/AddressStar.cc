// -*-Mode: C++;-*-
//
//                            Copyright 2014
//                                 by
//                    The Board of Trustees of the
//                 Leland Stanford Junior University.
//                        All rights reserved.


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "dslutil/Base.h"
#include "atca/Address.hh"
#include "kvt/Kvt.h"
#include "dslutil/AddressStar.hh"

#define MAX_ELEMENT   3
#define MAX_CMB       4
#define MAX_SLOT     16

#define CMB_DTM       4


namespace service {
  namespace dslutil {

    /**
       @class AddressStar
       @brief Express an RCE's location in an ATCA shelf, 
       with support for wildcards on the slot/cmb/element portions.

       All addresses in the RCE coordinate space are of the form
       shelf/slot/cmb/rce where shelf is the logical name of the
       shelf, slot is the physical slot number of the shelf where the
       COB is located, cmb is the "COB Mezannine Board" bay where the
       individual board is located, and rce is the instance of the
       processor on the particular CMB.

       This class allows addresses of the form: shelf[/slot[/bay[/rce]]]

       shelf is an arbitrary const char* (max length=63).
       
       slot goes from 1 to a max of 16.  Or it may be the keyword ALL.
       Note, many shelves do not physically have 16 slots.

       cmb goes from 0 to 4 or one of the keywords ALL, CMB, DTM, DPM.

       rce goes from 0 to 3, or the keyword ALL.

       Leaving off the end of the address is the same as using the keyword ALL
       for the omitted arguments.

       Examples:
       
       - tinker/4/3/2   -- shelf tinker, slot 4, bay 3, rce 2
       - tinker/4/3     -- shelf tinker, slot 4, bay 3, rce ALL
       - tinker/4       -- shelf tinker, slot 4, bay ALL, rce ALL
       - egbert/2/ALL/2 -- shelf egbert, slot 2, bay ALL, rce 2
       - egbert/2/DPM   -- all DPMs in egbert/2
    */

    /**
       @enum AddressStar::WildCards
       @brief Internal (and on-the-wire) storage for the various
       wildcard portions of a valid AddressStar.  All values are completely
       arbitrary and chosen for easy spotting in a debugger.

       @var AddressStar::SLOT_ALL_WC
       @brief WildCard for the slot.  Same in meaning as keyword ALL in a text
       based representation.

       @var AddressStar::CMB_DPM_WC
       @brief WildCard for the CMB.  Same in meaning as keyword DPM in a text
       based representation.

       @var AddressStar::CMB_DTM_WC
       @brief WildCard for the CMB.  Same in meaning as keyword DTM in a text
       based representation.

       @var AddressStar::CMB_ALL_WC
       @brief WildCard for the CMB.  Same in meaning as keyword ALL in a text
       based representation.

       @var AddressStar::ELEM_ALL_WC
       @brief WildCard for the element.  Same in meaning as keyword ALL in a text
       based representation.

       @var AddressStar::UNDEFINED
       @brief Special 'WildCard' meaning that this entry is currently undefined.
       This is usually an error.       
    */

    
    /** @brief Construct a wildcarded address when the components are known
        @param[in] shelf The shelf name.
        @param[in] slot  The slot number.
        @param[in] cmb   The CMB (or bay) number.
        @param[in] element The element number.
        Does a range check on the inputs.
     */
    AddressStar::AddressStar(const char* shelf,
                             uint32_t    slot,
                             uint32_t    cmb,
                             uint32_t    element) :
      _element(element),
      _cmb(cmb),
      _slot(slot)
    {
      strncpy(_shelf, shelf, service::atca::Address::MAX_SHELF_STRING);
      _shelf_hash = KvtHash(_shelf);
      _range_check();
    }

    /** @brief Construct a wildcarded address using a shelf name hash rather than the actual shelf name.
        @param[in] shelf The shelf name hash.
        @param[in] slot  The slot number.
        @param[in] cmb   The CMB (or bay) number.
        @param[in] element The element number.
        Does a range check on the inputs.
     */
    AddressStar::AddressStar(uint64_t    shelf_hash,
                             uint32_t    slot,
                             uint32_t    cmb,
                             uint32_t    element) :
      _element(element),
      _cmb(cmb),
      _slot(slot),
      _shelf_hash(shelf_hash)
    {
      _shelf[0] = 0;
      _range_check();
    }

    /** @brief Construct a wildcarded address from a presentation string.
        @param[in] presentation A string of the form shelf[/slot[/bay[/rce]]].
     */
    AddressStar::AddressStar(const char* presentation)
    {
      unsigned i;
      char* name;
      char* slot_s;
      char* cmb_s;
      char* elem_s;
      char *tmp;
      char p[service::atca::Address::MAX_PRESENTATION_STRING];

      // Make a copy of the string since strtok_r modifies the original
      strncpy(p, presentation, service::atca::Address::MAX_PRESENTATION_STRING);

      name    = strtok_r(p, "/", &tmp);
      slot_s  = strtok_r(0, "/", (char**)&tmp);
      cmb_s   = strtok_r(0, "/", (char**)&tmp);
      elem_s  = strtok_r(0, "/", (char**)&tmp);

    if (0 != slot_s)
      {
      if((0 == strcmp(slot_s, "DPM")) || 
         (0 == strcmp(slot_s, "DTM")) ||
         (0 == strcmp(slot_s, "ALL")))
        {
        cmb_s = slot_s;
        slot_s = NULL;
        }
      }
               
      if (0 != slot_s)
        {
        for(i=0; i<strlen(slot_s); ++i)
          slot_s[i] = toupper(slot_s[i]);
        }
      
      if (0 != cmb_s) 
        for(i=0; i<strlen(cmb_s); ++i)
          cmb_s[i] = toupper(cmb_s[i]);

      if (0 != elem_s) 
        for(i=0; i<strlen(elem_s); ++i)
          elem_s[i] = toupper(elem_s[i]);

      strncpy(_shelf, name, service::atca::Address::MAX_SHELF_STRING);

      if (0 == elem_s)
        _element = ELEM_ALL_WC;
      else {
        _element = strtol(elem_s, (char**)&tmp, 0);
        if (elem_s == tmp) {
          if (0 == strcmp(elem_s, "ALL"))
            _element = ELEM_ALL_WC;
        }
      }

      if (0 == cmb_s)
        _cmb = CMB_ALL_WC;
      else {
        _cmb = strtol(cmb_s, (char**)&tmp, 0);
        if (cmb_s == tmp) {
          if (0 == strcmp(cmb_s, "ALL"))
            _cmb = CMB_ALL_WC;
          else if (0 == strcmp(cmb_s, "DPM"))
            _cmb = CMB_DPM_WC;
          else if (0 == strcmp(cmb_s, "DTM"))
            _cmb = CMB_DTM_WC;
          else
            _cmb = UNDEFINED;
        }
      }

      if (0 == slot_s)
        _slot = SLOT_ALL_WC;
      else {
        _slot = strtol(slot_s, (char**)&tmp, 0);
        if (slot_s == tmp) {
          if (0 == strcmp(slot_s, "ALL"))
            _slot = SLOT_ALL_WC;
          else
            _slot = UNDEFINED;
        }
      }

      // If the shelf string is empty, don't try to hash it.
      if (0 == _shelf[0])
        _shelf_hash = UNDEFINED;
      else
        _shelf_hash = KvtHash(_shelf);

      _range_check();
    }


    void AddressStar::_range_check()
    {
      if ((_element > MAX_ELEMENT) && (_element != ELEM_ALL_WC))
        _element = UNDEFINED;
  
      if ((_cmb > MAX_CMB) && (_cmb != CMB_DPM_WC) &&
          (_cmb != CMB_DTM_WC) && (_cmb != CMB_ALL_WC))
        _cmb     = UNDEFINED;

      if ((_slot > MAX_SLOT) && (_slot != SLOT_ALL_WC))
        _slot    = UNDEFINED;
    }

    /** @brief wildcard match of service::atca::Address* to AddressStar
        @param addr  Input address
        @return true if the wildcard match passes, false otherwise
     */
    bool AddressStar::match(service::atca::Address* addr)
    {
      return ( (_match_shelf(addr->shelf()) || _match_shelf_hash(KvtHash(addr->shelf())))
               && _match_slot(addr->slot())
               && _match_cmb(addr->cmb())
               && _match_element(addr->element()) );
    }

    /** @brief wildcard match of AddressStar* to AddressStar
        @param addr  Input address
        @return true if the wildcard match passes, false otherwise
     */
    bool AddressStar::match(AddressStar* addr)
    {
      return ( (_match_shelf(addr->shelf()) || _match_shelf_hash(addr->shelf_hash()))
               && _match_slot(addr->slot())
               && _match_cmb(addr->cmb())
               && _match_element(addr->element()) );
    }

    /** @cond development
     */
    
    /** @brief Match the shelf name
        @return true if the shelf names are identical
     */
    bool AddressStar::_match_shelf(const char* name)
    {
      
      // Can't match if the input is either null or blank.
      if ((0 == name) || (0 == name[0]))
        return false;

      // Also can't match when we don't have an actual shelf name
      if (0 == shelf()[0])
        return false;

      if (strncmp(name, shelf(), service::atca::Address::MAX_SHELF_STRING)) return false;
      return true;
    }

    /** @brief Match the shelf name *hash*
        @return 
     */
    bool AddressStar::_match_shelf_hash(uint64_t value)
    {
      if (value != shelf_hash())      return false;
      return true;
    }

    /** @brief Match the slot
     */
    bool AddressStar::_match_slot(uint32_t value)
    {
      if ((slot() == UNDEFINED) || (value == UNDEFINED))
        return false;
      
      if (slot() == value) return true;

      if ( (slot() == SLOT_ALL_WC) || (value == SLOT_ALL_WC) )
        return true;

      return false;
    }

    /** @brief Match the cmb
     */
    bool AddressStar::_match_cmb(uint32_t value)
    {
      if ((cmb() == UNDEFINED) || (value == UNDEFINED))
        return false;
      
      if (cmb() == value)
        return true;

      if ( (cmb() == CMB_ALL_WC) || (value == CMB_ALL_WC) )
        return true;

      // DPM match
      if ( ((cmb() == CMB_DPM_WC) && (value < CMB_DTM)) ||
           ((value == CMB_DPM_WC) && (cmb() < CMB_DTM)) )
        return true;

      // DTM match
      if ( ((cmb() == CMB_DTM_WC) && (value == CMB_DTM)) ||
           ((value == CMB_DTM_WC) && (cmb() == CMB_DTM)) )
        return true;

      return false;
    }

    /** @brief Match the element
     */
    bool AddressStar::_match_element(uint32_t value)
    {
      if ((element() == UNDEFINED) || (value == UNDEFINED))
        return false;
      
      if (element() == value)
        return true;

      if ((element() == ELEM_ALL_WC) || (value == ELEM_ALL_WC))
        return true;

      return false;
    }

    
    /** @endcond */
    
  }
}
