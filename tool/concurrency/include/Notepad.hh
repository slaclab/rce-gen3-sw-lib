// -*-Mode: C++;-*-
/**
   @file
   @brief Declares the Notepad class template.
   @verbatim
                               Copyright 2011
                                     by
                        The Board of Trustees of the
                      Leland Stanford Junior University.
                             All rights reserved.
   @endverbatim
*/
#if !defined(TOOL_CONCURRENCY_NOTEPAD_HH)
#define      TOOL_CONCURRENCY_NOTEPAD_HH


#include "concurrency/Once.hh"

namespace tool {

  namespace concurrency {


    enum NotepadConstants {
      NUM_NOTEPADS = 16
    };

    class Semaphore;

    template<typename SlotType>
    class Notepad {
    public:
      Notepad();
      ~Notepad();

      SlotType get()                  const;
      void     set(SlotType slotVal);

    private:
      unsigned m_slot;
    };

  } // concurrency

} // tool

#include "concurrency/os/Notepad-inl.hh"

#endif // TOOL_CONCURRENCY_NOTEPAD_HH
