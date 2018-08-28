
#include "elf/linker.h"

typedef void Func (void);
Func beta;

extern int delta[4];

void beta(void) {
  beta();
  delta[0] = 0;
}

/* We just want to define the ,init_array element not export its name. */
static unsigned myInit  __attribute__((section(".init_array"))) = 0xdeadbeef;

/* Mark this object as something to be installed. */
const int lnk_options __attribute__((visibility("default"))) = LNK_INSTALL;
