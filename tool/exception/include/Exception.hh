// -*-Mode: C++;-*-
/*!
* @file
* @brief   Class Exception definition
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 3096 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_EXCEPTION_EXCEPTION_HH
#define TOOL_EXCEPTION_EXCEPTION_HH

namespace tool {
  namespace exception {
    class Exception {
    public:
      Exception();
      virtual ~Exception();

    public:
      const char* what() const;
      unsigned    len()  const;

    public:

    public:

    private:

    protected:
      static const unsigned MaxLength = 256;
      unsigned _len;
      char     _msg[MaxLength];
    };

    class Error : public Exception {
    public:
      Error();
      Error(const char* msgformat, ...);
      virtual ~Error();
    };

    class Event : public Exception {
    public:
      Event();
      Event(const char* msgformat, ...);
      virtual ~Event();
    };
  } // exception
} // tool

#endif // TOOL_EXCEPTION_EXCEPTION_HH
