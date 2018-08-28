// -*-Mode: C;-*-
/**
@file
@brief This is a PUBLIC interface to a set of functions to manage doubly-linked lists. 
Note that these functions are NOT re-entrant.  

@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
LIST

@author
Michael Huffer, SLAC (mehsys@slac.stanford.edu)

@par Date created:
2013/10/28

@par Last commit:
\$Date: 2013-10-07 18:56:03 -0700 (Mon, 07 Oct 2013) $ by \$Author: tether $.

@par Revision number:
\$Revision: 2191 $

@par Credits:
SLAC

*/

#ifndef LIST
#define LIST

/**  @brief

This structure defines an entry on a doubly-linked list. It contains two members: The
first is a pointer to the entry's forward link on its corresponding list while the
second member  

*/

typedef struct List_sLink {struct List_sLink* forward; struct List_sLink* reverse;} ListEntry;

/**  @brief


*/

typedef ListEntry ListHead; // syntactical sugar to differentiate a list from a link...

#if defined(__cplusplus)
extern "C" {
#endif

/**  @brief

This function inserts a specified entry to an arbitrary location within a doubly-linked 
list. The function has two arguments: The first is a pointer to the entry to be inserted. 
The second argument is a pointer to the entry AFTER the specified entry is to be inserted. 
This function returns NO value.

*/

static inline void List_Attach(ListEntry* this, ListEntry* after)
{ 
                                                              
register ListEntry* next = after->forward;  

this->forward  = next;                        
this->reverse  = after;      
next->reverse  = this;     
after->forward = this;     

return;
}

/**  @brief

This function removes a specified entry from a doubly-linked list. The function has 
one argument, which is a pointer to the entry to be removed. The function returns a 
pointer to the removed entry (that is, the input argument). 

*/

static inline ListEntry* List_Detach(ListEntry* this)    
{            
                                                   
register ListEntry* next = this->forward;  
register ListEntry* prev = this->reverse; 

prev->forward = next;
next->reverse = prev;

return this;
}

/**  @brief

This function initializes a specified doubly-linked list to EMPTY. The function takes a 
single argument which is a pointer to the list (actually its list HEAD). This function
returns NO value.

*/

static inline void List_Empty(ListHead* this)
{                                                               
this->forward = (ListEntry*)this;
this->reverse = (ListEntry*)this;
}

/**  @brief

This function returns the entry at the HEAD of a specified doubly-linked list. The 
function takes a single argument which is a a pointer to a list. The function returns 
the entry at the HEAD of that list. Note that for an EMPTY list this function will return
a pointer to the list itself (see "List_Eol").

*/

static inline ListEntry* List_Head(ListHead* this) 
{
return this->forward;
}

/**  @brief

This function returns the entry at the TAIL of a specified doubly-linked list. The 
function takes a single argument which is a a pointer to a list. The function returns 
the entry at the TAIL of that list. Note that for an EMPTY list this function will return
a pointer to the list itself (see "List_Eol").

*/

static inline ListEntry* List_Tail(ListHead* this) 
{
return this->reverse;
}

/**  @brief

This function returns the ADDRESS of a specified doubly-linked list. That is, it returns
a pointer to ITSELF (suitably recast). This value is useful for checking for an empty 
list (see routines above and below). This function takes a single argument which is a
pointer to a list. The function returns that pointer. 

*/

static inline ListEntry* List_Eol(ListHead* this)  
{
return (ListEntry*)this;
}

/**  @brief

This function inserts an entry at the TAIL of a specified doubly-linked list. The 
function takes two arguments: The first is a pointer to the entry to be inserted and the 
second argument is pointer to the list on which the specified entry will be inserted. 
This function returns NO value.   

*/

static inline void List_Insert(ListEntry* this, ListHead* list)
{                                                             
List_Attach(this, list->reverse);
return;  
}

/**  @brief

This function removes an entry at the HEAD of a specified doubly-linked list. The 
function takes a single arguments which is a pointer to the list from which an entry 
will be inserted. The removed entry is returned to the caller.  If the list was EMPTY a
pointer to the list itself is returned (see "CluEol").  

*/

static inline ListEntry* List_Remove(ListHead* list)
{                      
return List_Detach(list->forward);  
}

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
