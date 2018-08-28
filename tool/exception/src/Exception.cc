// -*-Mode: C++;-*-
/*!
*
* @brief   Class Exception implementation
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 3125 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include "exception/Exception.hh"
#include "logger/FormatMessage.hh"

tool::exception::Exception::Exception() :
  _len(0)
{
  _msg[0] = 0;
}

tool::exception::Exception::~Exception() {}

const char* tool::exception::Exception::what() const {return _msg;}
unsigned    tool::exception::Exception::len()  const {return _len;}

tool::exception::Error::Error() : tool::exception::Exception() {}

tool::exception::Error::Error(const char* msgformat, ...) : tool::exception::Exception()
{
  FORMAT_MESSAGE(msgformat, MaxLength, _msg, _len);
}

tool::exception::Error::~Error() {}

tool::exception::Event::Event() : tool::exception::Exception() {}

tool::exception::Event::Event(const char* msgformat, ...) : tool::exception::Exception()
{
  FORMAT_MESSAGE(msgformat, MaxLength, _msg, _len);
}

tool::exception::Event::~Event() {}
