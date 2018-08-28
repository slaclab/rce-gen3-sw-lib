// -*-Mode: C++;-*-
/*!
* @file
* @brief   Class implementing a stack (LIFO) using a singly linked list.
*          Note that insertion and removal are made atomic on the PowerPC
*          through the reservation mechanism.
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_CONTAINER_STACK_HH
#define TOOL_CONTAINER_STACK_HH



#include "container/Flink.hh"

namespace tool {
  namespace container {
    template <class T> class Stack {
    public:
      Stack();

    public:
      const T* empty() const;
      T*       head()  const;

    public:
      T*   insert(Flink<T>* item);
      T*   remove();
      void steal(Stack<T>*);
      void reset();

    private:
      Flink<T> _head;
    };

    template <class T> Stack<T>::Stack() {reset();}

    template <class T> const T* Stack<T>::empty() const
    {
      return static_cast<const T*>(&_head);
    }

    template <class T> T* Stack<T>::head() const
    {
      return static_cast<T*>(_head._flink);
    }

    template <class T> void Stack<T>::reset()
    {
      _head._flink = &_head;
    }

  }; // container
}; // tool

#include "container/family/StackImpl.hh"

#endif // TOOL_CONTAINER_STACK_HH
