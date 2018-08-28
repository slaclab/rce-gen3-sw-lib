// -*-Mode: C++;-*-
/*!
* @file
* @brief   Implementation for the Stack class
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
#ifndef TOOL_CONTAINER_STACK_IMPL_HH
#define TOOL_CONTAINER_STACK_IMPL_HH


namespace tool {
  namespace container {

    template <class T> T* Stack<T>::insert(Flink<T>* item)
    {
      // avoid the dreaded inline-assembly r0 problem by using stwx.
      Flink<T>* tmp;
      asm volatile ("1:;"
                    "LDREX  %0,0,%2"
                    "STWX   %0,0,(%1)"
                    "STREX  %1,0,%2"
                    "CMP    %1,#0"
                    "BNE    1b"
                    : "=&r" (tmp) : "r" (item), "r" (&_head) : "cc", "memory");
      return static_cast<T*>(tmp);
    }

    template <class T> T* Stack<T>::remove()
    {
      // avoid the dreaded inline-assembly r0 problem by using lwzx.
      Flink<T>* head;
      Flink<T>* tmp;
      asm volatile ("1:;"
                    "LDREX  %0,0,%2"
                    "STWX   %1,0,(%0)"
                    "STREX  %1,0,%2"
                    "CMP    %1,#0"
                    "BNE    1b"
                    : "=&r" (head), "=&r" (tmp) : "r" (&_head) : "cc", "memory");
      return static_cast<T*>(head);
    }

    template <class T> void Stack<T>::steal(Stack<T>* s)
    {
      // avoid the dreaded inline-assembly r0 problem by using stwx.
      Flink<T>* tmp;
      asm volatile ("1:;"
                    "LDREX  %0,0,%2"
                    "STWX   %0,0,(%1)"
                    "STREX  %2,0,%2"
                    "CMP    %2,#0"
                    "BNE    1b"
                    : "=&r" (tmp) : "&r" (s), "r" (&_head) : "cc", "memory");
    }
  }; // container
}; // tool

#endif // TOOL_CONTAINER_STACK_IMPL_HH
