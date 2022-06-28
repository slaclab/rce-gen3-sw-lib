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

#include <string.h>
#include <stdlib.h>


#include "dsl/Server.hh"

#include "dsl/impl/Request.hh"
#include "dsl/impl/Response.hh"

//#include "elf/linker.h"
//#include "memory/mem.h"
#include "kvt/Kvt.h"
//#include "elf/linker.h"          
//#include "ldr/Ldr.h"             

#define FOREVER -1

#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/*
**
** This constant specifies the maximum number of table entries that can be managed.
** Note that this value must be expressed as power of two... 
**
*/

#define MAX_ENTRIES 7 // As a power of 2, i.e., 7 = 128 entries 

/*
** ++
**
**
** --
*/

namespace service {namespace dsl {Server* _singleton = (Server*)0;}}

using namespace service::dsl;

const char* SERVER_MAGIC_COOKIE = "$$MAGIC_COOKIE$$";

//int lnk_options __attribute__((visibility("default"))) =  LNK_INSTALL;

/*
** ++
**
**
** --
*/

void Server::startup()
  {
  
  if(_singleton) return;

  Server* server = new Server();
  
  if(!server) return;
  
  _singleton= server;
    
  }

/*
** ++
**
**
** --
*/

void Server::startup(const char **services)
  {
  const char **svc;
  
  if(_singleton) return;

  Server* server = new Server();
  
  if(!server) return;
  
  _singleton= server;
  
  if(!services) return;
  
  /* install services */
  for(svc = services; *svc; svc++)
    {
    server->_lookup(*svc);
    }
  }

/*
** ++
**
**
** --
*/

void Server::execute()
  {
 
  Server* server = _singleton;
  
  if (server) server->body();
  }

/*
** ++
**
**
** --
*/

void Server::run()
  {
 
  Server* server = _singleton;
  
  if (server) server->start();
  }

/*
** ++
**
**
** --
*/

void Server::bind(Space* space)  
  {
  
  Server* server = _singleton;
  
  if(server) KvtInsert(Location::hash(space->name()), (void *)space, server->_kvtable);
  
  }

/*
** ++
**
**
** --
*/

Server::Server() :
  tool::concurrency::Thread(),
  _magic_cookie(Location::hash(SERVER_MAGIC_COOKIE)),
  _kvtable(KvtConstruct(MAX_ENTRIES,malloc(KvtSizeof(MAX_ENTRIES)))),
  _link(SERVER_PORT)
 {
 }

/*
** ++
**
**
** --
*/
  
void Server::body()
  {
  
  while(FOREVER)
    {
    tool::concurrency::Thread::threadYield();
    Request* request = (Request*)_link.waitUpdate();
    
    if(request->magic_cookie() != _magic_cookie) continue; 

    if(request->version() != VERSION) continue; 

    Space* space = _lookup(request->space());
    if(!space) continue;
    
    const Location* result = space->lookup(request->tag());
  
    if(!result) continue;
  
    Response* response = new(_buffer) Response(_magic_cookie, request->tid(), *result);

    _link.post((void*)response, sizeof(Response));
    }
  }
