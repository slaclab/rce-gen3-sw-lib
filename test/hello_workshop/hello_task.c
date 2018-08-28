#include <stdio.h>
#include "debug/print.h"
#include "task/Task.h"
#define PRINT dbg_printv
// Functions from hello.so
extern int hello(void);
extern int goodbye(void);

void Task_Start(int argc,
                const char** argv) {
  PRINT("Hello from Task!\n");
  hello();
  PRINT("Return from Start.\n");
  return;
}

void Task_Rundown() {
  goodbye();
  PRINT("Goodbye from Task!\n");
  return;
}
