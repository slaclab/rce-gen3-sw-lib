#ifndef cm_plx_h
#define cm_plx_h

#include <inttypes.h>

typedef struct
{
  uint32_t v[4];
} uint128_t;


typedef struct {
  char       _buffer[48];
  uint128_t* _tx_buffer;
  uint128_t* _rx_buffer;

  unsigned _rx_last;

  volatile unsigned* _reg;
  unsigned _tag;
} plx;

plx*     plx_init (uint32_t addr);
unsigned plx_read (plx*, const uint32_t*);
void     plx_write(plx*, uint32_t*, uint32_t);

#endif
