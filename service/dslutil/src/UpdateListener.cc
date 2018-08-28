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
#include "dslutil/UpdateBase.hh"
#include "dslutil/UpdateListener.hh"
#include "concurrency/SemaphoreGuard.hh"
#include "debug/print.h"
#include "cob_ipmc/COB_ipmc.h"

namespace service {
  namespace dslutil {
    namespace updater {
      using tool::concurrency::SemaphoreGuard;

      enum {DATA_LOCK  = 0,
            SHELF_LOCK = 1};
    
      Listener::Listener() :
        tool::concurrency::Thread(),
        _interface(),
        _udp(_interface.bcAddr(), (uint16_t)UPDATE_PORT, (uint16_t)UPDATE_PORT),
        _terminate(false)
      {
        name("FOOO");
        _shelf[0] = 0;
      }


      Listener::Listener(const char* ifname) :
        tool::concurrency::Thread(),
        _interface(ifname),
        _udp(_interface.bcAddr(), (uint16_t)UPDATE_PORT, (uint16_t)UPDATE_PORT),
        _terminate(false)
      {
        name("FOOO");
        _shelf[0] = 0;
      }

      Listener::~Listener()
      {
      }

      void Listener::body()
      {
        clear();
        
        while (! _terminate) {
          Message* msg;
          msg = (Message*)_udp.waitUpdate(200);
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
              DSL_slot_update_state* st = &_status.state[(msg->slot-1)];
                
              st->responded  |= (1 << st->rces);
              st->os[st->rces]     = msg->os;
              st->bay[st->rces]    = msg->cmb;
              st->rce[st->rces]    = msg->element;
              st->id[st->rces]     = msg->id;
              st->status[st->rces] = msg->status;
              st->rces++;
            }
          }
          yield();
        }
      }

      DSL_slot_update_status*
      Listener::status(void* buffer)
      {
        {
          SemaphoreGuard guard(_lock[DATA_LOCK]);
          memcpy(buffer, (void*)&_status, sizeof(DSL_slot_update_status));
        }
        return (DSL_slot_update_status*)buffer;
      }

      void
      Listener::clear()
      {
        SemaphoreGuard guard(_lock[DATA_LOCK]);
        memset((void*)&_status, 0, sizeof(DSL_slot_update_status));
      }

      void
      Listener::setShelf(const char* shelf_s)
      {
        SemaphoreGuard guard(_lock[SHELF_LOCK]);
        strncpy(_shelf, shelf_s, MAX_SHELF_STR);
      }

      void
      Listener::report(COB_slot_rce_status *exp, bool showMissing)
      {
        int slot;
        int rce;
        int i,j;

        DSL_slot_update_status s;
        status(&s);

        if(exp)
          {
          exp->nslots = 0;
          exp->slot_valid_mask = 0;
          }
          
        for (slot=0; slot < s.nslots; ++slot) {
          
          if (((s.slot_response & (1<<slot)) == 0)) {
            if (showMissing) {
              printf("Slot %d : No Responses\n", slot+1);
            }
          } else {
            if(exp)
              {
              exp->slot_valid_mask |= (1<<slot);
              exp->slot[slot].running = 0;
              exp->slot[slot].present = 0;
              exp->slot[slot].enabled = 0;
              }
            DSL_slot_update_state state = s.state[slot];
            printf("Slot %d : Had Responses\n", slot+1);
            if(exp)
              exp->nslots++;
            for(i=0;i<DSLUTIL_MAX_BAY_PER_COB;i++) {
              for(j=0;j<DSLUTIL_MAX_RCE_PER_BAY;j++) {
                for (rce=0; rce < DSLUTIL_MAX_RCE_PER_COB; ++rce) {
                  if ( (state.responded & (1<<rce)) == 0) {
                    if (showMissing) 
                      printf("   RCE %d Did not respond\n", rce);
                  } else {
                    if(state.bay[rce] == i && state.rce[rce] == j) {
                      if(exp)
                        exp->slot[slot].running |= ((!state.rce[rce] ? 1: 4) << (4*i));

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
                                
                      printf("   RCE %d/%d : OS: %s  (ID:0x%08x  STATUS:0x%x)\n",
                      state.bay[rce], state.rce[rce], os, state.id[rce], state.status[rce]);
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
