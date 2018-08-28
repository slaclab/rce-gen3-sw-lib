// -*-Mode: C++;-*-
//                               Copyright 2015
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#include <stdio.h>

#include "net/MacAddress.hh"
#include "dsl/Location.hh"
#include "dslutil/AddressStar.hh"
#include "dslutil/IdBase.hh"
#include "dslutil/IdListener.hh"
#include "concurrency/SemaphoreGuard.hh"
#include "debug/print.h"

namespace service {
  namespace dslutil {
    namespace identifier {
      using tool::concurrency::SemaphoreGuard;

      enum {DATA_LOCK  = 0,
            SHELF_LOCK = 1};
    
      Listener::Listener() :
        tool::concurrency::Thread(),
        _interface(),
        _udp(_interface.bcAddr(), (uint16_t)IDENTIFIER_PORT, (uint16_t)IDENTIFIER_PORT),
        _terminate(false)
      {
        name("FOOO");
        _shelf[0] = 0;
        _idstr[0] = 0;        
      }


      Listener::Listener(const char* ifname) :
        tool::concurrency::Thread(),
        _interface(ifname),
        _udp(_interface.bcAddr(), (uint16_t)IDENTIFIER_PORT, (uint16_t)IDENTIFIER_PORT),
        _terminate(false)
      {
        name("FOOO");
        _shelf[0] = 0;
        _idstr[0] = 0;
      }

      Listener::~Listener()
      {
      }

      void Listener::body()
      {
        clear();
        
        while (! _terminate) {
          Message* msg;
          msg = (Message*)_udp.waitUpdate(500);
          if (msg) {

            { // Check the received shelf name against what's expected.
              char tmp[MAX_SHELF_STR];
              {
                SemaphoreGuard guard(_lock[SHELF_LOCK]);
                strncpy(tmp, _shelf, MAX_SHELF_STR);
              }
              if (strncmp(msg->shelf, tmp, MAX_SHELF_STR) != 0) {
                printf("Listener::body: Got a response from unexpected shelf (%s) have %s\n",
                     msg->shelf, tmp);
                continue;
              }
            }
            
            { 
              SemaphoreGuard guard(_lock[DATA_LOCK]);
              if (_status.nslots < msg->slot) _status.nslots = msg->slot;

              _status.slot_response |= (1<<(msg->slot-1));
              DSL_slot_rce_state* st = &_status.state[(msg->slot-1)];
                
              st->responded |= (1 << st->rces);
              st->os[st->rces]    = msg->os;
              st->bay[st->rces]   = msg->cmb;
              st->rce[st->rces]   = msg->element;
              st->ip[st->rces]    = msg->ip;
              strncpy(&st->id[st->rces*DSLUTIL_MAX_ID_STR], msg->id, DSLUTIL_MAX_ID_STR);
              st->rces++; 
            }
          }
          yield();
        }
      }

      DSL_slot_rce_status*
      Listener::status(void* buffer)
      {
        {
          SemaphoreGuard guard(_lock[DATA_LOCK]);
          memcpy(buffer, (void*)&_status, sizeof(DSL_slot_rce_status));
        }
        return (DSL_slot_rce_status*)buffer;
      }

      void
      Listener::clear()
      {
        SemaphoreGuard guard(_lock[DATA_LOCK]);
        memset((void*)&_status, 0, sizeof(DSL_slot_rce_status));
      }

      void
      Listener::setShelf(const char* shelf_s)
      {
        SemaphoreGuard guard(_lock[SHELF_LOCK]);
        strncpy(_shelf, shelf_s, MAX_SHELF_STR);
      }

      void
      Listener::report(bool showMissing)
      {
        int slot;
        int rce;
        int i,j;

        DSL_slot_rce_status s;
        status(&s);

        for (slot=0; slot < s.nslots; ++slot) {
          
          if (((s.slot_response & (1<<slot)) == 0)) {
            if (showMissing) {
              printf("Slot %d : No Responses\n", slot+1);
            }
          } else {
            DSL_slot_rce_state state = s.state[slot];
            printf("Slot %d : Had Responses\n", slot+1);
            for(i=0;i<DSLUTIL_MAX_BAY_PER_COB;i++) {
              for(j=0;j<DSLUTIL_MAX_RCE_PER_BAY;j++) {
                for (rce=0; rce < DSLUTIL_MAX_RCE_PER_COB; ++rce) {
                  if ( (state.responded & (1<<rce)) == 0) {
                    if (showMissing) 
                      printf("   RCE %d Did not respond\n", rce);
                  } else {
                    if(state.bay[rce] == i && state.rce[rce] == j) {
                      const char* os;
                      switch (state.os[rce]) {
                      case OS_NOTSET:
                        os = "NOT SET";   break;
                      case OS_RTEMS:
                        os = "RTEMS";     break;
                      case OS_LINUX:
                        os = "LINUX";     break;
                      case OS_RESCUE:
                        os = "RESCUE";    break;
                      case OS_UNKNOWN:
                        os = "UNKNOWN";   break;
                      default:
                        os = "ERROR: os invalid"; break;
                      }

                      dsl::Location loc(state.ip[rce]);
                      char buf[64];

                      printf("   RCE %d/%d : OS: %s  IP: %-15s  ID: %s",
                      state.bay[rce], state.rce[rce], os, loc.layer3.addr_p(buf), &state.id[rce*DSLUTIL_MAX_ID_STR]);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
