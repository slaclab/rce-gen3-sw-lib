
#include "elf/linker.h"

typedef void Func (void);
Func alpha, beta, gamma;

int delta[4]   = {0xfeedbeef, 0, 0, 0};

int epsilon = 0xabcdef;

void alpha(void) {
  alpha();
  beta();
  gamma();
  delta[0] = 0;
  epsilon = 0;
}

void beta(void) {
  delta[0] = 0;
  epsilon = 0;
}

void gamma(void) {
  delta[0] = 0;
  epsilon = 0;
}

/* We just want to define the ,init_array element not export its name. */
static unsigned myInit  __attribute__((section(".init_array"))) = 0xdeadbeef;

/* Mark this object as something to be installed. */
const int lnk_options __attribute__((visibility("default"))) = LNK_INSTALL;
