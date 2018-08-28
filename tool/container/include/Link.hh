// -*-Mode: C++;-*-
/*!
* @file
* @brief   Class describing a double link
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
#ifndef TOOL_CONTAINER_LINK_HH
#define TOOL_CONTAINER_LINK_HH

namespace tool {
  namespace container {
    template <class T> class List;
    template <class T> class Link {
    public:
      Link() {}

      T* flink() const {return static_cast<T*>(_flink);}
      T* blink() const {return static_cast<T*>(_blink);}

      void insert(Link<T>* item);
      void remove();

    private:
      friend class List<T>;
      Link<T>* _flink;
      Link<T>* _blink;
    };

    template <class T> void Link<T>::insert(Link<T>* item)
    {
      Link<T>* next  = _flink;
      Link<T>* after = this;
      item->_flink = next;
      item->_blink = after;
      next->_blink = item;
      after->_flink = item;
    }

    template <class T> void Link<T>::remove()
    {
      register Link<T>* next = _flink;
      register Link<T>* prev = _blink;
      prev->_flink = next;
      next->_blink = prev;
    }
  }; // container
}; // tool

#endif // TOOL_CONTAINER_LINK_HH
