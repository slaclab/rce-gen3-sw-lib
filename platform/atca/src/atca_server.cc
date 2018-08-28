/*
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	    000 - August 18, 2013
**
**  Revision History:
**	    None.
**
** --
*/
 


#include <stdio.h>

#include "concurrency/Thread.hh"
#include "dsl/init.hh"
#include "atca/init.hh"

using namespace tool::concurrency;

// We need to provide a main thread to run the tests under Linux.
class Main: public Thread {
public:
  Main(int argc, char **argv) : Thread(false) {}

  virtual void body() {    
    service::dsl::initialize();

    service::atca::initialize();
  }

  virtual ~Main() {}
  
private:
};

int main(int argc, char **argv) {
  Main tmain(argc,argv);
  tmain.graduate();
  while(1){
  Thread::threadYield();
  };
  
  return 0;
}

