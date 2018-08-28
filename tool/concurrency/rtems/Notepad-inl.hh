// -*-Mode: C++;-*-
/**
@file
@brief Inline implementation of Notepad<T> for RTEMS.
@verbatim
                               Copyright 2011
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/
#include <stdexcept>

#include <rtems.h>


#include "concurrency/SemaphoreGuard.hh"

namespace tool {

  namespace concurrency {

    /** @cond development */
    namespace notepad_private {

      extern bool isAllocated[NUM_NOTEPADS];

      extern Semaphore* lock;

      extern Once initOnce;

      extern void init();
    }
    /** @endcond */

    template<typename SlotType>
    Notepad<SlotType>::Notepad() {
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
	notepad_private::isAllocated[avail] = true;
	m_slot = avail;
      }
      else {
	throw std::runtime_error("tool::concurrency::Notepad: No more free notepads.");
      }
    }

    template<typename SlotType>
    Notepad<SlotType>::~Notepad() {
      SemaphoreGuard sg(*notepad_private::lock);
      notepad_private::isAllocated[m_slot] = false;
    }

    template<typename SlotType>
    SlotType Notepad<SlotType>::get() const {
      uint32_t v;
      rtems_task_get_note(RTEMS_SELF, m_slot, &v);
      return reinterpret_cast<SlotType>(v);
    }

    template<typename SlotType>
    void Notepad<SlotType>::set(SlotType val) {
      rtems_task_set_note(RTEMS_SELF, m_slot, reinterpret_cast<uint32_t>(val));
    }

  }; // concurrency

}; // tool
