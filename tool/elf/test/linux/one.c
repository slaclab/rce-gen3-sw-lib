
#include "elf/linker.h"

typedef void Func (void);
Func alpha, beta, gamma;

extern int delta[4];

/* Force multiple relocations to refer to delta. */
int *foo[] = {delta, delta+1, delta+2, delta+3};

void alpha(void) {
  alpha();
  beta();
  gamma();
  delta[0] = 0;
}

/* We just want to define the ,init_array element not export its name. */
static unsigned myInit  __attribute__((section(".init_array"))) = 0xdeadbeef;

/* The following should be ignored by lnk_load(). */
unsigned lnk_options = LNK_INSTALL;
