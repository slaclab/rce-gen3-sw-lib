// -*-Mode: C++;-*-
/*!
* @file
* @brief   Class implementing a queue (FIFO) using two stacks
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
#ifndef TOOL_CONTAINER_QUEUE_HH
#define TOOL_CONTAINER_QUEUE_HH


#include "container/Stack.hh"

namespace tool {
  namespace container {
    template <class T> class Queue {
    public:
      Queue();

    public:
      const T* empty() const;
      T*       head()  const;

    public:
      void insert(Flink<T>* item);
      T*   remove();
      void reset();
      void out();

    private:
      Stack<T> _in;
      Stack<T> _out;
    };

    template <class T> Queue<T>::Queue() {reset();}

    template <class T> const T* Queue<T>::empty() const
    {
      return _out.empty();
    }

    template <class T> T* Queue<T>::head() const
    {
      return _out.head();
    }

    template <class T> void Queue<T>::insert(Flink<T>* item)
    {
      _in.insert(item);
    }

    template <class T> T* Queue<T>::remove()
    {
      T* head = _out.remove();
      if (head == _out.empty()) {
        out();
        head = _out.remove();
      }
      return head;
    }

    template <class T> void Queue<T>::reset()
    {
      _in.reset();
      _out.reset();
    }

    template <class T> void Queue<T>::out()
    {
      Stack<T> tmp;
      _in.steal(&tmp);
      T* entry;
      const T* empty = _in.empty();
      while ((entry = tmp.remove()), entry != empty) {
        _out.insert(entry);
      }
    }
  }; // container
}; // tool

#endif // TOOL_CONTAINER_QUEUE_HH
