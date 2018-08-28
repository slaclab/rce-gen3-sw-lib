// -*-Mode: C++;-*-
/*!
* @file
* @brief   Class implementing a doubly linked list
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
#ifndef TOOL_CONTAINER_LIST_HH
#define TOOL_CONTAINER_LIST_HH


#include "container/Link.hh"

namespace tool {
  namespace container {
    template <class T>
    class List {
    public:
      List();
      List(bool skipinit);

    public:
      const T* empty() const;
      T* head() const;
      T* tail() const;

    public:
      void insert(Link<T>* item);
      T*   remove();
      void reset();

    private:
      Link<T> _listhead;
    };

    template <class T> List<T>::List() {reset();}

    template <class T> List<T>::List(bool skipinit) {}

    template <class T> const T* List<T>::empty() const
    {
      return static_cast<const T*>(&_listhead);
    }

    template <class T> T* List<T>::head() const
    {
      return static_cast<T*>(_listhead._flink);
    }

    template <class T> T* List<T>::tail() const
    {
      return static_cast<T*>(_listhead._blink);
    }

    template <class T> void List<T>::insert(Link<T>* item)
    {
      _listhead._blink->insert(item);
    }

    template <class T> T* List<T>::remove()
    {
      Link<T>* head = _listhead._flink;
      head->remove();
      return static_cast<T*>(head);
    }

    template <class T> void List<T>::reset()
    {
      _listhead._flink = &_listhead;
      _listhead._blink = &_listhead;
    }

  }; // container
}; // tool

#endif // TOOL_CONTAINER_LIST_HH
