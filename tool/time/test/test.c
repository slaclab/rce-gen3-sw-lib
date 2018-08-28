#include <sdio.h>

#include <rtems.h>


#include "time/cpu/time.h"


static inline uint64_t get_timer(uint64_t start)
{
  return TOOL_TIME_lticks() - start;
}

static inline uint64_t get_usecs(uint64_t dT)
{
  return TOOL_TIME_t2uS(dT);
}

static inline void udelay(uint32_t us)
{
  /* Range is about 2 million uS, or 2 seconds */
  /* Overhead seems to be about 0x21 ticks */

  uint32_t tBeg = TOOL_TIME_ticks();
  uint32_t tEnd = (us * CPU_CLOCK_FREQ_NUM) / (2 * CPU_CLOCK_FREQ_DEN);

  while (TOOL_TIME_ticks() - tBeg < tEnd);

  //uint32_t tNow = TOOL_TIME_ticks();

  //printf("%s: us = %08lx, tBeg = %08lx, tEnd = %08lx, now = %08lx, dT = %08lx, f = %08lx\n",
  //       __func__, us, tBeg, tEnd, tNow, tNow - tBeg, CPU_CLOCK_FREQUENCY / 2 / 1000000);
}


void test(void)
{
  uint64_t start = get_timer(0);

  printk("%s: Start of 10 second sleep (measure it with a clock!)\n", __func__);
  start = get_timer(0);
  sleep(10);
  start = get_timer(start);
  printf("%s: End of 10 second sleep, time = %llu ticks, %llu uS\n", __func__,
         start, get_usecs(start));

  printk("%s: Start of 1000 uS delay\n", __func__);
  start = get_timer(0);
  udelay(1000);
  start = get_timer(start);
  printf("%s: End of 1000 uS delay, time = %llu ticks, %llu uS\n", __func__,
         start, get_usecs(start));

  printk("%s: Start of 100 uS delay\n", __func__);
  start = get_timer(0);
  udelay(100);
  start = get_timer(start);
  printf("%s: End of 100 uS delay, time = %llu ticks, %llu uS\n", __func__,
         start, get_usecs(start));

  printk("%s: Start of 10 uS delay\n", __func__);
  start = get_timer(0);
  udelay(10);
  start = get_timer(start);
  printf("%s: End of 10 uS delay, time = %llu ticks, %llu uS\n", __func__,
         start, get_usecs(start));
}
