// -*-Mode: C++;-*-
/**
@file
@brief RTEMS Task main program to call the test driver.

This test program is intended to be run rusing the "run" command from
the RTEMS shell. It therefore uses shell/redirect.h to make sure that
if the shell is part of a telnet session then the output goes to the
telnet socket.

@verbatim
                               Copyright 2012
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim
*/

#include "system/statusCode.h"
#include "shell/redirect.h"
#include "task/Task.h"         

#include "concurrency/Thread.hh"

namespace tool {

  namespace concurrency {

    void testConcurrency();
  }

}

using tool::concurrency::Thread;

/** @brief Provide a main Thread for test suite. */
class Main: public Thread {
public:
  /** @brief Initialize the base class. */
  Main() : Thread(false) {}

  /** @brief Run the test code. */
  virtual void body() {
    tool::concurrency::testConcurrency();
  }

  /** @brief Default destructor. */
  virtual ~Main() {}
};



/** @brief The saved fd of the parent's telnet socket (or console). */
static int outputFd = -1;

/** @brief Set outputFd. Called from the parent thread. */
extern "C" int lnk_prelude(void* prefs, void* elf) {
  outputFd = shell_getTtyFd();
  return STS_K_SUCCESS;
}


/** @brief Redirect Task I/O to the stdout of the parent thread
    and then start the test thread.
*/
extern "C" void Task_Start(int argc, const char** argv)
{
  shell_redirectStdio(outputFd);
  Main tmain;
  tmain.graduate();
}


/** @brief Print a message saying the Task has ended. */
extern "C" void Task_Rundown()
{
  printf("Ending Task testConcurrency.exe.\n");
  return;
}
