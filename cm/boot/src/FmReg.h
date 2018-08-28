#ifndef FmReg_h
#define FmReg_h

#include <inttypes.h>

#define FM_DEV_BASE 0
#define FM_DEV_SIZE (0x1<<24)
//
//
//

typedef uint32_t FmReg;

#define PLX_READ(reg)      plx_read (_plx,&this->reg)
#define PLX_WRITE(reg,val) plx_write(_plx,&this->reg,val)

#endif
