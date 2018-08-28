/*
 * Copyright (c) 2013 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <info@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#include <bsp.h>
#include <bsp/arm-cp15-start.h>
#include <bsp/arm-a9mpcore-start.h>
#include <bsp/arm-a9mpcore-clock.h>

#include "debug/platform/init.h"

/* Frequency in mhz*10 of the ps ctl input clock.
 * This is provided by the oscillator crystal which
 * is the same for all zynq boards.
 */
#define PS_CTL_FREQ                 333333333

/* SLCR clock registers */
#define CLK_CTL_REG                 0xF8000120
#define CLK_CTL_DIV_SHIFT           0x8
#define CLK_CTL_DIV_MASK            0x3F

#define PLL_CTL_REG                 0xF8000100
#define PLL_CTL_DIV_SHIFT           0xC
#define PLL_CTL_DIV_MASK            0x7F

#define PERIPH_CLK_DIV              0x2

#include "startup/init.h"

static uint32_t ZynqCpuClkRate;
static uint32_t ZynqPeriphClkRate = BSP_ARM_A9MPCORE_PERIPHCLK;

/*
 * Return the RTEMS peripheral clock rate. For systems such as the zynq this
 * is controlled by the PL logic generation and can vary.
 * This rate is used by RTEMS as the source for the 64-bit global timer,
 * which in turn drives the tick counter.
 * This routine overrides the BSP default.
 */
uint32_t a9mpcore_clock_periphclk(void)
{
  return ZynqPeriphClkRate;
}

/*
 * Calculate the cpu and RTEMS peripheral clock rates.
 */
void calc_zynq_clks(void)
{
  uint32_t clkctl  = *(volatile uint32_t *)CLK_CTL_REG;
  uint32_t pllctl  = *(volatile uint32_t *)PLL_CTL_REG;
  
  /* get frequency divisor for the cpu clock source */
  uint32_t clkdiv = (clkctl >> CLK_CTL_DIV_SHIFT) & CLK_CTL_DIV_MASK;
  
  /* get pll feedback divisor */
  unsigned long long plldiv = (pllctl >> PLL_CTL_DIV_SHIFT) & PLL_CTL_DIV_MASK;

  if((!clkdiv) || (!plldiv)) return;

  /* calculate the cpu 6x4x clock freq */
  ZynqCpuClkRate = (PS_CTL_FREQ*plldiv)/(clkdiv*10); // ps_ctl is Mhz*10
  
  /* the RTEMS peripheral clock rate is 1/2 of the CPU freq */
  ZynqPeriphClkRate = ZynqCpuClkRate/PERIPH_CLK_DIV;
}

BSP_START_TEXT_SECTION void bsp_start_hook_0(void)
{
  bsp_start_clear_bss(); /* Need this done early for printk to work reliably. */
  calc_zynq_clks();
  dbg_earlyInit();       /* Init. the syslog and the console UART. */
  arm_a9mpcore_start_hook_0();
  BOOT_STATE(BOOT_BSPSTARTHOOK0);
}

void mem_setupMmuTable(void);

BSP_START_TEXT_SECTION void bsp_start_hook_1(void)
{
  arm_a9mpcore_start_hook_1();
  mem_setupMmuTable();
  BOOT_STATE(BOOT_BSPSTARTHOOK1);
}
