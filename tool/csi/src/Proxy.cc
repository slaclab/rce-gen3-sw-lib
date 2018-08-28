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



#include "csi/Proxy.hh"

#define INDEX(sequence) (sequence & (MAX_PENDING - 1))

using namespace tool::csi;

/*
** ++
**
**
** --
*/
 
Proxy::Proxy(Link* link, uint32_t timout, uint32_t retries) :
 _inflight(),
 _link(link),
 _next_tid(0),
 _timout(timout),
 _retries(retries),
 _freelist(Transaction::sizeof_request(), MAX_PENDING)
 {

 uint32_t      remaining = MAX_PENDING;
 Transaction** next      = _pending;

 do *next++ = (Transaction*)0; while(--remaining);

 }

/*
** ++
**
**
** --
*/

void Proxy::_post(void* request, uint32_t length) 
 {

 uint32_t tid = _next_tid++;
 
 _tidIs(tid, request);

 Transaction* transaction = new(Transaction::buffer(request)) Transaction(length, _timout, _retries);

 uint32_t index = INDEX(tid);
 
 _pending[index] = transaction;

 _inflight.insert(transaction);
 
 _link->post(request, length);
 
 return;
 }

/*
** ++
**
**
** --
*/

#define FOREVER -1

void* Proxy::_wait() 
 {
 
 Transaction* pending = _inflight.head();
 
 if(pending == _inflight.empty()) return (void*)0;
 
 while(FOREVER) 
   {
   void* response = _link->wait(pending->timout());
   
   if(response) return _rundown(response);
   
   if(!pending->retry()) break;
     
   _link->post(pending->request(), pending->length());
   }
     
 return _rundown(pending);
 }

/*
** ++
**
**
** --
*/

void* Proxy::_rundown(void* response) 
 {
 
 uint32_t     index       = INDEX(_tid(response));
 Transaction* transaction = _pending[index];
  
 if(!transaction) return (void*)0;
  
 _pending[index] = (Transaction*)0;
 
 transaction->remove();
 
 _freelist.deallocate(transaction->request());
   
 return response;
 }

/*
** ++
**
**
** --
*/

void* Proxy::_rundown(Transaction* pending) 
 {
 
 pending->remove();  

 void* request = pending->request();
  
 _pending[INDEX(_tid(request))] = (Transaction*)0;
 
 _freelist.deallocate(request);
 
 return (void*)0;
 }
