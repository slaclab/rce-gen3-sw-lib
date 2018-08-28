// -*-Mode: C;-*-

#ifndef POOL_H
#define POOL_H


/* ---------------------------------------------------------------------- *//**


@file  Pool.h
@brief Manage a set of fixed size buffers all with specified memory attributes.

@par
The collection of functions described below allows a set (or pool) of
fixed-size buffers to be created from a specified memory region. The size and
number of buffers is determined by the user. Once the pool is created buffers
can be arbitrary allocated and deallocated from and to the created pool.  

@verbatim
                               Copyright 2013
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
POOL

@author
Michael Huffer, SLAC (mehsys@slac.stanford.edu)

@par Date created:
2013/10/28

@par Credits:
SLAC
                                                                          */
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *\
   
   HISTORY
   -------
  
   DATE       WHO WHAT
   ---------- --- ---------------------------------------------------------
   2014.08.31 jjr Imported from MHuffer.
  
\* ---------------------------------------------------------------------- */

//#define POOL_CACHED

#if defined(__cplusplus)
extern "C" {
#endif


/* ---------------------------------------------------------------------- *//**

@typedef Pool_Handle
@brief   Handle used to identify a created pool.    
                                                                          */
/* ---------------------------------------------------------------------- */
typedef struct {char opaque[8];} Pool_Handle; 
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//**

@brief   Create a set of fixed sized buffers from a specified memory region.
@return  The pool handle used to allocate and deallocate buffers

@param[in]  region      The region of memory to carve the pool from \sa
                        memory/mem.h
@param[in]  max_size    The maximum size of any one buffer
@param[in]  max_entries The maximum number of entries
@param[in]  handle      The opaque pool handle

@par
Create a set of fixed sized buffers from a specified memory region. One can
then allocate and deallocate buffers from that pool using the functions
described below. The function takes four (4) arguments: The first argument 
is an enumeration which identifies the region from which to allocate the
memory for the pool. The set of memory regions is specified in "memory/mem.h".
The second argument is the size (in bytes) of the any one allocated buffer. 
A value of zero (0) is not allowed. The third argument is the number of 
buffers for the pool. A value of zero (0) is not allowed. When the number
of buffers allocated exceeds this number subsequent allocations will return
a NIL value. The fourth argument is a pointer to a user supplied buffer to
hold the internal context for the pool. The size of that buffer is determined
by the "sizeof(Pool_handle)". That buffer's lifetime must match the lifetime
of the buffers allocated from the pool. 

The function returns a handle to the created pool. If the pool cannot be
created a NIL (zero) handle is returned. The pool will not be created if the
input arguments are either malformed, the specified region does not exist, or
the specified region does itself not contain sufficient memory to create the
pool.   
                                                                          */
/* ---------------------------------------------------------------------- */
Pool_Handle* Pool_New(int           region, 
                      unsigned    max_size, 
                      unsigned max_entries, 
                      Pool_Handle *handle);
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//**

@brief  Allocate a fixed-size buffer from a specified pool.
@return Nothing

@param[in]  buffer The buffer to deallocate
@param[in]  pool   The pool to return the buffer to

@par
The specified pool was created with the "Pool_New" function described above.
The size of the allocated buffer is determined by the pool. the function 
takes a single argument which is a handle to the pool from which to allocate
the buffer. That handle was returned from the "Pool_New" function described
above.   

The function returns a pointer to the allocated buffer. If a buffer cannot 
be allocated the function returns a NIL (zero) pointer.
                                                                          */
/* ---------------------------------------------------------------------- */
void* Pool_Allocate(Pool_Handle* pool); 
/* ---------------------------------------------------------------------- */


/* ----------------------------------------------------------------------*//**

@brief  The function probes a specified pool, determining whether or not 
        it is currently empty. The 
@return A non-zero value (1) if the pool is empty or zero (0) if the pool
        is not empty

@param[in] pool  The pool handle to be probed.
                                                                          */
/* ---------------------------------------------------------------------- */
int Pool_Empty(Pool_Handle* pool); 
/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- *//**

@brief  Deallocate a fixed-size buffer allocated from a specified pool.

@par
The specified pool was created with the "Pool_New" function described above.
The function takes two arguments: The first is a pointer to the buffer to
be deallocated. This pointer was returned by a call to the "Pool_Allocate"
function described above. The second argument is a handle to the pool to
return the allocated buffer. That handle was returned from the "Pool_New" 
function described above.   
 
The function returns NO value.   
                                                                          */
/* ---------------------------------------------------------------------- */
void Pool_Deallocate(void* buffer, Pool_Handle* pool);
/* ---------------------------------------------------------------------- */

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
