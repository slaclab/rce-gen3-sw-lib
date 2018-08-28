#include "fsbl.h"
#include "xil_cache.h"
#include "xil_exception.h"
#include "platform.h"
#include "xtime_l.h"

/* Exception handlers */
static void RegisterHandlers(void);
static void Undef_Handler (void);
static void SVC_Handler (void);
static void PreFetch_Abort_Handler (void);
static void Data_Abort_Handler (void);
static void IRQ_Handler (void);
static void FIQ_Handler (void);

extern int hello_mem (void);

int main()
{
  u32 BootModeRegister = 0;
  u32 RebootStatusRegister = 0;
  XTime ts[6];

  XTime_GetTime(&ts[0]);

  /*
   * PCW initialization for MIO,PLL,CLK and DDR
   */
  ps7_init();

  XTime_GetTime(&ts[1]);

  /*
   * Unlock SLCR for SLCR register write
   */
  SlcrUnlock();

  XTime_GetTime(&ts[2]);

  /*
   * Flush the Caches
   */
  Xil_DCacheFlush();

  XTime_GetTime(&ts[3]);

  /*
   * Disable Data Cache
   */
  Xil_DCacheDisable();

  XTime_GetTime(&ts[4]);

  /*
   * Register the Exception handlers
   */
  RegisterHandlers();

  XTime_GetTime(&ts[5]);

  /*
   * Print a Banner
   */
  xil_printf("\n\r\n\rXilinx OCM Memory Test \n\r");
  xil_printf("Release %d.%d/%d.%d	%s-%s\r\n",
             SDK_RELEASE_VER, SDK_SUB_VER,
             SDK_RELEASE_YEAR, SDK_RELEASE_QUARTER,
             __DATE__,__TIME__);

  xil_printf("\n\r");

#ifdef FSBL_PERF
  XTime tEnd = 0;
  xil_printf("%-16s: ", "First");
  FsblMeasurePerfTime(ts[0], tEnd);
  xil_printf("%-16s: ", "ps7_init");
  FsblMeasurePerfTime(ts[1], ts[0]);
  xil_printf("%-16s: ", "SlcrUnlock");
  FsblMeasurePerfTime(ts[2], ts[1]);
  xil_printf("%-16s: ", "DCacheFlush");
  FsblMeasurePerfTime(ts[3], ts[2]);
  xil_printf("%-16s: ", "DCacheDisable");
  FsblMeasurePerfTime(ts[4], ts[3]);
  xil_printf("%-16s: ", "RegisterHandlers");
  FsblMeasurePerfTime(ts[5], ts[4]);
#endif

  /*
   * Read bootmode register
   */
  BootModeRegister = Xil_In32(BOOT_MODE_REG);
  BootModeRegister &= BOOT_MODES_MASK;
  fsbl_printf(DEBUG_INFO,
              "Boot mode register: 0x%08x\r\n",BootModeRegister);

  RebootStatusRegister = Xil_In32(REBOOT_STATUS_REG);
  fsbl_printf(DEBUG_INFO,
              "Reboot status register: 0x%08x\r\n",RebootStatusRegister);

  init_platform();

  hello_mem();

  cleanup_platform();

  xil_printf("\r\nDone.\r\n");
  for(;;);

  return 0;
}

/******************************************************************************/
/**
*
* This function Registers the Exception Handlers
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static void RegisterHandlers(void)
{
	Xil_ExceptionInit();

	 /*
	 * Initialize the vector table. Register the stub Handler for each
	 * exception.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNDEFINED_INT,
					(Xil_ExceptionHandler)Undef_Handler,
					(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SWI_INT,
					(Xil_ExceptionHandler)SVC_Handler,
					(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT,
				(Xil_ExceptionHandler)PreFetch_Abort_Handler,
				(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
				(Xil_ExceptionHandler)Data_Abort_Handler,
				(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler)IRQ_Handler,(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT,
			(Xil_ExceptionHandler)FIQ_Handler,(void *) 0);

	Xil_ExceptionEnable();

}

static void Undef_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"UNDEFINED_HANDLER\r\n");
	for (;;);
}

static void SVC_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"SVC_HANDLER \r\n");
	for (;;);
}

static void PreFetch_Abort_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"PREFETCH_ABORT_HANDLER \r\n");
	for (;;);
}

static void Data_Abort_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"DATA_ABORT_HANDLER \r\n");
	for (;;);
}

static void IRQ_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"IRQ_HANDLER \r\n");
	for (;;);
}

static void FIQ_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"FIQ_HANDLER \r\n");
	for (;;);
}

/******************************************************************************
*
* This function Gets the ticks from the Global Timer
*
* @param	Current time
*
* @return
*			None
*
* @note		None
*
*******************************************************************************/
#ifdef FSBL_PERF
void FsblGetGlobalTime (XTime tCur)
{
	XTime_GetTime(&tCur);
}


/******************************************************************************
*
* This function Measures the execution time
*
* @param	Current time , End time
*
* @return
*			None
*
* @note		None
*
*******************************************************************************/
void FsblMeasurePerfTime (XTime tCur, XTime tEnd)
{
	double tDiff = 0.0;
	double tPerfSeconds;
	XTime_GetTime(&tEnd);
	tDiff  = (double)tEnd - (double)tCur;

	/*
	 * Convert tPerf into Seconds
	 */
	tPerfSeconds = tDiff/COUNTS_PER_SECOND;

	/*
	 * Convert tPerf into Seconds
	 */
	tPerfSeconds = tDiff/COUNTS_PER_SECOND;

#if defined(FSBL_DEBUG) || defined(FSBL_DEBUG_INFO)
	xil_printf("%d.%03d seconds \r\n",(int)tPerfSeconds,
                   (int)(1000*tPerfSeconds) - 1000*((int)tPerfSeconds));
#endif

}
#endif
