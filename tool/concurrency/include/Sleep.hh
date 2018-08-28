// -*-Mode: C++;-*-
/*!
* @file
* @brief Declare global functions usleep() and sleep().
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_CONCURRENCY_SLEEP_HH
#define TOOL_CONCURRENCY_SLEEP_HH

namespace tool {
  namespace concurrency {

    void usleep(unsigned us);

    void sleep(unsigned millisecs);

  }; // concurrency
}; // tool

#endif // TOOL_CONCURRENCY_SLEEP_HH
