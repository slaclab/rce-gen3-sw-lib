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
**
**  Revision History:
**	None.
**
** --
*/

#include <stdio.h>   // printf...
#include <strings.h>
#include <stdlib.h>

#include "pool/Pool.h" 
#include "list/List.h"

/*
** ++
**
**
** --
*/



#ifdef __arm__

#include "memory/mem.h"

#else

#include <stdlib.h>
static inline void *mem_Region_alloc(int region, int amount)
{
    return malloc (amount);
}

#endif


#define ERROR (Pool_Handle*)0;

Pool_Handle* Pool_New(int           region, 
                      unsigned    max_size, 
                      unsigned max_entries, 
                      Pool_Handle*    pool)
 {

 unsigned size = (((max_size + sizeof(ListEntry)) + 31) >> 5) << 5;
 
 if(!size) return ERROR;
 
 unsigned remaining = max_entries;
 
 if(!remaining) return ERROR;

 char* next = mem_Region_alloc(region, (size * remaining)+8);
 //printf("%s: allocated pool at 0x%x %d non-cached bytes: %d %d\n",__func__,next,(size*remaining),size,remaining);
 
 if(!next) return ERROR;
 
 bzero(next,(size * remaining)+8);
 
 next += 8;
  
 ListHead* freelist = (ListHead*)pool;
 
 List_Empty(freelist);

 do {List_Insert((ListEntry*)next, freelist); next += size;} while(--remaining);

 return pool;
 }
 
/*
** ++
**
**
** --
*/

void* Pool_Allocate(Pool_Handle* pool) 
 {
 
 ListHead*  freelist = (ListHead*)pool;
 ListEntry* entry    = List_Remove(freelist);
   
 return entry != List_Eol(freelist) ? (void*)(entry + 1) : (void*)0; 
 }
  


/*
** ++
**
**
** --
*/
int Pool_Empty(Pool_Handle* pool) 
 {
 ListHead* freelist = (ListHead*)pool;

 return freelist->forward == List_Eol(freelist); 
 }


/*
** ++
**
**
** --
*/

void Pool_Deallocate(void* buffer, Pool_Handle* pool)
 {

 ListHead* freelist = (ListHead*)pool;
 ListEntry* entry   = (ListEntry*)buffer;
 
 List_Insert(entry - 1, freelist);
 
 return;
 }


