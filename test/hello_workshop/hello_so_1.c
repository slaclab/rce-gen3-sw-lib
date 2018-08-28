#include <stdio.h>
#include "debug/print.h"

#define PRINT dbg_printv

int hello(void) {
  PRINT("Hi! I'm a .so!\n");
  return 0;
}
int goodbye(void) {
  PRINT("Goodbye .so!\n");
  return 0;
}
int lnk_prelude(void* arg,
                void* elf) {
  PRINT("Hello prelude!\n");
  hello();
  goodbye();
  PRINT("Goodbye prelude!");
  return 0;
}
