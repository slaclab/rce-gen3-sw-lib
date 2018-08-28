/*!@class  Interrupt
*
* @brief   Class InterruptHandler is an abstract class providing the interface
*          definition for various call-back methods.  This interface must
*          be satisfied by application code in order to process data and
*          timeouts.
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    February 6, 2009 -- Created
*
* $Revision: 40 $
*
* Copyright:                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
*/
#include <rtems.h>
#include <bsp/vectors.h>

using namespace tool::interrupt;

static int _interruptHandler(BSP_Exception_frame *f, unsigned int vector)
{
  _handler[vector]->process();
  return 0;
}


template <class T>
Handlers::Handler<T>* Interrupt::bind(unsigned              vector,
                                      Handlers::Handler<T>* handler)
{
  disable();

  Handlers::Handler_t<T>* prvHandler = _handler[vector];
  _handler[vector] = handler;

  ppc_exc_set_handler(vector, _interruptHandler);

  enable();

  return prvHandler;
}


template <class T>
Handlers::Handler<T>* Interrupt::release(unsigned vector)
{
  disable();

  Handlers::Handler_t<T>* prvHandler = _handler[vector];
  _handler[vector] = NULL;

  ppc_exc_set_handler(vector, NULL);

  enable();

  return prvHandler;
}
