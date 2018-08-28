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
* $Revision: 2961 $
*
* Copyright:                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
*/
#ifndef TOOL_INTERRUPT_INTERRUPT_HH
#define TOOL_INTERRUPT_INTERRUPT_HH

namespace tool
{
  namespace interrupt
  {
    namespace handler
    {
      template <class T>
      class InterruptHandler
      {
      public:
        //! The constructor has no arguments and throws no exceptions.
        InterruptHandler() {}
        virtual ~InterruptHandler() {}

        //! Process method
        /*! This method is invoked for each received interrupt.  Note that this is
         *  an interface (call-thru in the CRTP pattern sense) method whose
         *  implementation is expected to be satisfied by a derived class.
         *  @return This function has no arguments and throws no exceptions.
         *          It throws no exceptions.
         */
        inline void process()
        {
          static_cast<T*>(this)->process();
        }
      };
    }; // handler

    class Interrupt
    {
    public:
      enum {CRITICAL = ASM_BOOKE_CRIT_VECTOR,
            EXTERNAL = ASM_EXT_VECTOR};

      template <class T>
      handlers::Handler_t<T>* bind(unsigned                vector,
                                   handlers::Handler_t<T>* handler);
      template <class T>
      handlers::Handler_t<T>* release(unsigned vector);

    };

  }; // interrupt
}; // tool

#endif // TOOL_INTERRUPT_INTERRUPT_HH
