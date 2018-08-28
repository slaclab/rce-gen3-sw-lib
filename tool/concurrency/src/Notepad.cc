// -*-Mode: C++;-*-
//                               Copyright 2014
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

namespace tool {

  namespace concurrency {

    /**
    @class Notepad
    @brief Manages a small global table of thread-local values. Each
    slot in the table may hold a full int or a pointer.
    @tparam SlotType The type of the value to be held by the notepad slot.
    @see NUM_NOTEPADS

    The set of table slots is the same for every thread but the values
    in the slots are specific to each thread. When the context
    switches from one thread to another all the slot values are seen
    to change as well.  The default value for each slot is the null
    pointer.

    Each instantiation of this template becomes the owner of one
    of the slots in the table, unless there are no free slots
    left in which case an exception is thrown. Destruction
    of an instance releases its hold on the slot.

    Each instantiation is associated with a specific data %type;
    its set() and get() member functions will convert data of that
    %type to and from the slot value %type using
    reinterpret_cast<>(). I recommend that you restrict yoursef to
    ints, unsigneds, enums and pointers as these will fit inside
    the storage allocated to a slot value on both 32-bit and
    64-bit platforms.

    If a package needs to use a notepad slot it declares a static
    Notepad instance; assuming that enough slots are available each
    such instance is assigned a slot at the time when C++ static
    constructors are run.

    @fn Notepad::Notepad()
    @brief Allocate a notepad slot from the global pool.
    @exception std::runtime_exception if all slots
    are used up.

    @fn Notepad::~Notepad()
    @brief Release the slot owned by this notepad.

    @fn Notepad::get() const
    @brief Return the slot value.
    @return The value that is valid for the current thread.

    @fn Notepad::set(SlotType slotVal)
    @brief Set the slot value for the current thread.

    @var Notepad::m_slot
    @brief The global notepad slot no. assigned to this instance.

    @enum NotepadConstants
    @brief Constants associated with class template Notepad.

    @var NUM_NOTEPADS
    @brief The maximum number of Notepad instances that can exist at any one time;
    the number of slots in the table.
    */
  }

}
