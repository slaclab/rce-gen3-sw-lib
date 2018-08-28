/*
** ++
**  Package:
**  
**
**  Abstract:
**
**  Author:
**      Michael Huffer, SLAC, (415) 926-4269
**
**  Creation Date:
**	000 - June 20 1,1997
**
**  Revision History:
**	None.
**
** --
*/

#ifndef TOOL_CONTAINER_LINKEDLIST
#define TOOL_CONTAINER_LINKEDLIST

namespace tool {
namespace container {
namespace linkedList {

/*
** ++
**
**
** --
*/

class EntryBase {
public:
  EntryBase();
  EntryBase(EntryBase* empty);  
public:
  ~EntryBase();
public:
  EntryBase* flink();
  EntryBase* blink();
public:
  void linkAfter(EntryBase*);
  void remove();
private:
  EntryBase* _flink;
  EntryBase* _blink;
};

/*
** ++
**
**
** --
*/

inline EntryBase::EntryBase()
{                                                               
}

/*
** ++
**
**
** --
*/

inline EntryBase::EntryBase(EntryBase* empty)
{                                                               
_flink = (EntryBase*)&_flink;
_blink = (EntryBase*)&_flink;
}

/*
** ++
**
**
** --
*/

inline EntryBase::~EntryBase()
{                                                               
}

/*
** ++
**
**
**
** --
*/

inline EntryBase* EntryBase::flink()
{                                                               
return _flink;
}

/*
** ++
**
**
**
** --
*/

inline EntryBase* EntryBase::blink()
{                                                               
return _blink;
}

/*
** ++
**
** Insert ourself on a doubly-linked list. The input argument is a pointer to 
** the entry AFTER the entry is to be inserted.
**
** --
*/

inline void EntryBase::linkAfter(EntryBase* entry)
{                                                               
register EntryBase* next = entry->_flink;  

this->_flink  = next;                        
this->_blink  = entry;      
next->_blink  = this;     
entry->_flink = this;     

return;
}

/*
** ++
**
** Remove ourself from the list we are linked to.
**
** --
*/

inline void EntryBase::remove()    
{                                                               
register EntryBase* next = _flink;  
register EntryBase* prev = _blink; 

prev->_flink = next;
next->_blink = prev;

return;
}

/*
** ++
**
**
** --
*/

template<class T> class Entry : EntryBase {
public:
  Entry()         : EntryBase()      {}
  Entry(T* empty) : EntryBase(empty) {}  
public:
  ~Entry()                           {}
public:
  T* flink()          {return (T*)EntryBase::flink();}
  T* blink()          {return (T*)EntryBase::blink();}
public:
  void linkAfter(T* entry) {EntryBase::linkAfter(entry);}
  void remove()            {EntryBase::remove();}
};

/*
** ++
**
**
** --
*/

class ListBase {
public:
  ListBase() : _listhead((EntryBase*)this) {} 
public:
 ~ListBase()               {}
public: 
  EntryBase* head();   
  EntryBase* tail();   
  EntryBase* empty() const;   
  void       insert(EntryBase*);
  EntryBase* remove();
private:
  EntryBase _listhead;
};

/*
** ++
**
** Return the entry at the head of the queue
**
** --
*/

inline EntryBase* ListBase::head() 
{
return _listhead.flink();
}

/*
** ++
**
** Return the entry at the tail of the queue
**
** --
*/

inline EntryBase* ListBase::tail() 
{
return _listhead.blink();
}

/*
** ++
**
** Return the queue's label (usefull for checking for an empty queue)
**
** --
*/

inline EntryBase* ListBase::empty() const 
{
return (EntryBase*)&_listhead;
}

/*
** ++
**
** This function assumes the object represents the listhead of a doubly linked
** list and inserts the entry specified by the input argument at the TAIL of 
** that list.
**
** --
*/

inline void ListBase::insert(EntryBase* entry)
{                                                               
entry->linkAfter(_listhead.blink());
return;  
}

/*
** ++
**
** This function assumes the object represents the listhead of a doubly 
** linked list and removes the entry at the HEAD of the list. The removed 
** entry is returned to the caller.
**
** --
*/

inline EntryBase* ListBase::remove()
{  
register EntryBase* head = _listhead.flink();                    
head->remove();
return head;  
}

/*
** ++
**
**
** --
*/

template<class T> class List : ListBase {
public:
  List() : ListBase() {} 
public:
 ~List()              {}
public: 
  T*   head()           {return (T*)ListBase::head();}   
  T*   tail()           {return (T*)ListBase::tail();}   
  T*   empty() const    {return (T*)ListBase::empty();}
  void insert(T* entry) {ListBase::insert((EntryBase*)entry); return;}
  T*   remove()         {return (T*)ListBase::remove();}
};

}}}

#endif
