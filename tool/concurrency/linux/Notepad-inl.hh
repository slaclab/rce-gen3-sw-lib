// -*-Mode: C++;-*-
/**
@file
@brief Inline implementation of Notepad<T> for Linux.

@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#include <stdexcept>
#include <cerrno>
#include <cstring>


#include "pth.h"

#include "logger/Logger.hh"
#include "concurrency/SemaphoreGuard.hh"
#include "exception/Exception.hh"

/** @cond development */
#define PTH_CALL(VAR, FUNC, ARGS...)                                     \
  VAR = FUNC(ARGS);                                                      \
  if (!VAR) {                                                            \
    throw Error("Call to %s failed in %s: %s", #FUNC, __PRETTY_FUNCTION__, strerror(errno)); \
  }
/** @endcond */

namespace tool {

  namespace concurrency {

    using std::strerror;
    using tool::exception::Error;
    using service::logger::Logger;

    /** @cond development */
    namespace notepad_private {
        
      extern bool isAllocated[NUM_NOTEPADS];
      
      extern int key[NUM_NOTEPADS];
      
      extern Semaphore* lock;
        
      extern Once initOnce;
        
      extern void init();
    }
    /** @endcond */

    template<typename SlotType>
    Notepad<SlotType>::Notepad() {
      pth_init(); // Will fail gracefully if already called.
      notepad_private::initOnce.run(notepad_private::init);
      SemaphoreGuard sg(*notepad_private::lock);
      int avail = -1;
      for (unsigned i = 0; i < NUM_NOTEPADS; ++i) {
	if (!notepad_private::isAllocated[i]) {
	  avail = i;
	  break;
	}
      }
      if (avail >= 0) {
        int status;
	notepad_private::isAllocated[avail] = true;
	m_slot = avail;
        PTH_CALL(status, pth_key_create, notepad_private::key + avail, 0);
      }
      else {
	throw std::runtime_error("tool::concurrency::Notepad: No more free notepads.");
      }
    }

    template<typename SlotType>
    Notepad<SlotType>::~Notepad() {
      SemaphoreGuard sg(*notepad_private::lock);
      notepad_private::isAllocated[m_slot] = false;
      int status;
      status = pth_key_delete(notepad_private::key[m_slot]);
      if (!status) {
        // Don't throw from a destructor.
        Logger().error("Call to pth_key_delete failed in %s: %s", __PRETTY_FUNCTION__, strerror(errno));
      }
    }

    template<typename SlotType>
    SlotType Notepad<SlotType>::get() const {
      void* v(pth_key_getdata(notepad_private::key[m_slot]));
      return reinterpret_cast<SlotType>(v);
    }

    template<typename SlotType>
    void Notepad<SlotType>::set(SlotType val) {
      int status;
      PTH_CALL(status, pth_key_setdata, notepad_private::key[m_slot], reinterpret_cast<void*>(val));
    }


  } // concurrency

} // tool
#undef PTH__CALL
