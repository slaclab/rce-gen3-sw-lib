#include <stdio.h>
#include "svt/Svt.h"
#include "debug/print.h"

#define PRINT dbg_printv
#define NUM 15
#define TABLE (1<<NUM)

int hello(void) {
  PRINT("Hi! I'm a .so!\n");
  const char* hm = Svt_Translate
    ("HELLO_MESSAGE",TABLE);
  if(hm)
    PRINT("%s\n",hm);

  return 0;
}
int goodbye(void) {
  const char* gm = Svt_Translate
    ("GOODBYE_MESSAGE",TABLE);
  if(gm)
    PRINT("%s\n",gm);
  PRINT("Goodbye .so!\n");
  return 0;
}
int lnk_prelude(void* arg,
                void* elf) {
  PRINT("Hello prelude!\n");
  hello();
/* install the hello SVT */
  Svt_Install(NUM,
              "examples:hello.svt");
  
  goodbye();
  PRINT("Goodbye prelude!");
  return 0;
}
