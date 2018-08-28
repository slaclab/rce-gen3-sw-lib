// -*-Mode: C++;-*-
/*!
* @file
* @brief   Class implementing a stack
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 662 $
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

namespace tool {
  namespace container {

    template <class T> T* Stack<T>::insert(Flink<T>* item)
    {
      Flink<T>* tmp = _head._flink;
      item->_flink = tmp;
      _head._flink = item;
      return tmp;
    }

    template <class T> T* Stack<T>::remove()
    {
      Flink<T>* head = _head._flink;
      _head._flink = head->_flink;
      return static_cast<T*>(head);
    }

    template <class T> void Stack<T>::steal(Stack<T>* s)
    {
      s->_head._flink = _head._flink;
      _head._flink = &_head;
    }

  }; // container
}; // tool

#endif // TOOL_CONTAINER_STACK_HH
