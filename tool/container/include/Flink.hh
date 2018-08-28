// -*-Mode: C++;-*-
/*!
* @file
* @brief   Class describing a forward link
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
#ifndef TOOL_CONTAINER_FLINK_HH
#define TOOL_CONTAINER_FLINK_HH

namespace tool {
  namespace container {
    template <class T> class Stack;
    template <class T> class Queue;
    template <class T> class Flink {
    public:
      Flink() {}

    public:
      T* flink() const {return static_cast<T*>(_flink);}

    private:
      friend class Stack<T>;
      friend class Queue<T>;
      Flink<T>* _flink;
    };
  }; // container
}; // tool

#endif // TOOL_CONTAINER_FLINK_HH
