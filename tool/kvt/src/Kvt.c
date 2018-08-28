// -*-Mode: C;-*-
/**
@cond development
@file Kvt.c
@brief Implement and document the KVT (Key-Value-Table) facility
@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/


#include "list/List.h"
#include "hash/Hash.h"
#include "kvt/Kvt.h"

typedef struct sKvp  {
  ListEntry link;
  KvtKey    key;
  KvtValue  value;
} Kvp;


typedef struct sKvt {
 uint32_t  stride;
 ListHead* lut;
 uint32_t  remaining;
 Kvp*      next;
 ListHead  freelist;
} Kvt;

#define LOOKUP_HEAD(key, kvt) (((uint32_t)key & kvt->stride) + kvt->lut)
#define LNGTHOF_LUT(entries)  (1 << (entries + 1))         
#define SIZEOF_LUT(entries)   (LNGTHOF_LUT(entries) * sizeof(ListHead))
#define SIZEOF_BUF(entries)   ((1 << (entries))    * sizeof(Kvp))

/*
** ++
**
** Forward references for local (private functions)...
**
** --
*/

static Kvp* _after(KvtKey this, ListHead*);
static Kvp* _allocate(Kvt*); 
static Kvp* _match(KvtKey this, ListHead*);

KvtKey KvtHash(const char* vector)
 {
 return HashString(vector, HASH_SEED);
 }


uint32_t KvtSizeof(uint32_t max_entries)
 {
 return sizeof(Kvt) +  SIZEOF_LUT(max_entries)  + SIZEOF_BUF(max_entries);  
 }


KvTable KvtConstruct(uint32_t max_entries, void* buffer)
 {

 Kvt* kvt = (Kvt*)buffer;

 if(!kvt) return (KvTable)0;
 
 List_Empty(&kvt->freelist);

 uint32_t remaining = 1 << max_entries;
 
 kvt->remaining = remaining;
 
 remaining <<= 1; // LUT table is always twice as big as maximum number of entries 

 ListHead* next = (ListHead*)&kvt[1];

 kvt->lut    = next;
 kvt->next   = (Kvp*)&next[remaining];
 kvt->stride = remaining - 1;
  
 do List_Empty(next++); while(--remaining);

 return (KvTable)kvt;
 }
 

KvtValue KvtInsert(KvtKey this, KvtValue value, KvTable table)
 {

 Kvt*      kvt   = (Kvt*)table;
 ListHead* head  = LOOKUP_HEAD(this, kvt);
 Kvp*      after = _after(this, head);
   
 if(!after) return (KvtValue)0;
 
 Kvp* kvp = _allocate(kvt);
  
 if(!kvp) return 0;

 if(!value) return 0;
  
 kvp->value = value;
 kvp->key   = this;
   
 List_Attach((ListEntry*)&kvp->link, (ListEntry*)after);

 return value;
 }
    


KvtValue KvtLookup(KvtKey this, KvTable table)
 {

 Kvt*      kvt  = (Kvt*)table;
 ListHead* head = LOOKUP_HEAD(this, kvt);
 Kvp*      kvp  = _match(this, head);
 
 return kvp ? kvp->value : 0;
 }


KvtValue KvtRename(KvtKey this, KvtValue value, KvTable table)
 {

 Kvt*      kvt  = (Kvt*)table;
 ListHead* head = LOOKUP_HEAD(this, kvt);
 Kvp*      kvp  = _match(this, head);

 KvtValue previous = kvp ? kvp->value : 0;
 
 if(kvp) kvp->value = value;
 
 return kvp ? kvp->value : 0;

 return previous;
 }


KvtValue KvtRemove(KvtKey this, KvTable table)
 {

 Kvt*      kvt  = (Kvt*)table;
 ListHead* head = LOOKUP_HEAD(this, kvt);
 Kvp*      kvp  = _match(this, head);

 if(!kvp) return 0;
 
 List_Detach(&kvp->link);
 
 KvtValue previous = kvp->value;

 List_Insert(&kvp->link, &kvt->freelist);
 
 return previous;
 }

/*----------------------------------------*/
/* Internal functions                     */
/*----------------------------------------*/

static Kvp* _allocate(Kvt* kvt) 
 {
 
 uint32_t remaining = kvt->remaining; 
 Kvp*     kvp       = kvt->next;
 
 if(remaining)
   {
   kvt->remaining = remaining - 1;
   kvt->next      = kvp + 1; 
   }
 else
   {
   kvp = (Kvp*)List_Remove(&kvt->freelist);
   if(kvp == (Kvp*)List_Eol(&kvt->freelist)) kvp = (Kvp*)0;
   }  
 
 return kvp;
 }
   
/*
** ++
**
**
** --
*/

static Kvp* _match(KvtKey this, ListHead* list)
 {

 Kvp* last = (Kvp*)List_Eol(list);
 Kvp* next = (Kvp*)List_Head(list);   
    
 while(next != last)
   {
   if(next->key == this) return next;
   next = (Kvp*)next->link.forward;
   }

 return (Kvp*)0;
 }

/*
** ++
**
**
** --
*/

static Kvp* _after(KvtKey this, ListHead* list)
 {

 Kvp* last = (Kvp*)List_Eol(list);
 Kvp* next = (Kvp*)List_Head(list);      

 while(next != last)
   {
   KvtKey next_key = next->key;
   if(next_key == this) return (Kvp*)0;
   if(next_key  > this) break;
   next = (Kvp*)next->link.forward;
   }

 return (Kvp*)next->link.reverse;
 }


/** @endcond
 */


/** @typedef KvtKey
 *  @brief A unique 64-bit bit value, computed by ::KvtHash or one of
 *         the Hash functions in hash/Hash.h.  This key is used as
 *         input to ::KvtInsert, ::KvtLookup, ::KvtRename and ::KvtRemove.
 */

/** @typedef KvtValue
 *  @brief An opaque 32 bit word used as input to ::KvtInsert and ::KvtRename,
 *         and returned from ::KvtInsert, ::KvtLookup, ::KvtRename and ::KvtRemove.
 */

/** @typedef KvTable
 *  @brief A handle to a Key/Value Table used as input to
 *         ::KvtInsert, ::KvtLookup, ::KvtRename and ::KvtRemove, and returned
 *         from ::KvtConstruct.
 */


/** @fn KvtKey KvtHash(const char* vector)
    @brief Hash a specified string to a key.
 *  @param vector Pointer to the string for which to compute the key.
 *  @return The computed ::KvtKey.
 * 
 *  A key is simply a unique 64-bit value for the specified
 *  string. This key is passed as an input to the functions defined
 *  below. The function takes a single argument which is a pointer to a
 *  string which specifies the string whose key is to be computed. The
 *  function returns the key corresponding to the specified string.
 */

/** @fn uint32_t KvtSizeof(uint32_t max_entries)
 *  @brief Compute the required size of the buffer to allocate when
 *         constructing a ::KvTable object.
 *  @param max_entries  log_2(max number of entries) in the table.
 *  @return Size in bytes of the buffer to allocate.
 */

/** @fn KvTable KvtConstruct(uint32_t max_entries, void* buffer)
 *  @brief Construct a Key/Value table
 *  @param max_entries log_2(max number of entries) in the table.
 *  @param buffer A pre-allocated buffer of the proper size.  (See ::KvtSizeof)
 *  @return A handle to the constructed table.
 * 
 *  This function constructs a Key/Value table which is placed in memory
 *  specified via the buffer argument.  The size of this buffer is 
 *  established by the ::KvtSizeof function.
 * 
 *  @note This implies that the first argument for both ::KvtSizeof and
 *        ::KvtConstruct are consistent.  If the values are not consistent,
 *        behavior is undefined.
 */

/** @fn KvtValue KvtInsert(KvtKey this, KvtValue value, KvTable table)
 *  @brief Insert a key/value pair to a specified ::KvTable.
 *  @param this A key generated by ::KvtHash (see note).
 *  @param value The ::KvtValue to be inserted.  May not be zero (0).
 *  @param table A ::KvTable handle to the Key-Value table where
 *         the pair is to be inserted.
 *  @return The value that was inserted, or zero (0) for failure.
 *          Zero may be returned if:
 *          -# The value of the key/value pair is zero (0).
 *          -# The key is already registered.
 *          -# The table is full (i.e. the maximum entry value
 *             specified in ::KvtConstruct has been exceeded).
 *  
 *  @note Hash functions from hash/Hash.h may be used in place of
 *        ::KvtHash.
 */

/** @fn KvtValue KvtLookup(KvtKey this, KvTable table)
 *  @brief Return the value for a key in a ::KvTable.
 *  @param this A key generated by ::KvtHash (see note).
 *  @param table A ::KvTable handle to the Key-Value table from
 *         which to lookup the key.
 *  @return The ::KvtValue inserted by ::KvtInsert, or zero (0)
 *          if the ::KvtKey is not found.
 *  
 *  @note Hash functions from hash/Hash.h may be used in place of
 *        ::KvtHash.
 */
 
/** @fn KvtValue KvtRename(KvtKey this, KvtValue value, KvTable table)
 *  @brief Replace the value of a specified key/value pair in a ::KvTable.
 *  @param this A key generated by ::KvtHash (see note).
 *  @param value The ::KvtValue to be replaced.  May not be zero (0).
 *  @param table The ::KvTable handle.
 *  @return The \b previous ::KvtValue inserted by ::KvtInsert, or
 *          zero (0) if the ::KvtKey was not found in the ::KvTable.
 */

/** @fn KvtValue KvtRemove(KvtKey this, KvTable table)
 *  @brief Remove a key/value pair from a ::KvTable.
 *  @param this A key generated by ::KvtHash (see note).
 *  @param table A ::KvTable handle to the Key-Value table
 *               to remove the pair from.
 *  @return The ::KvtValue removed, or 
 *          zero (0) if the ::KvtKey was not found in the ::KvTable.
 */
