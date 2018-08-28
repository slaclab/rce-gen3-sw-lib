#ifndef ___GPIO_H___
#define ___GPIO_H___


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xgpiops.h"

/************************** Function Prototypes ******************************/


int GpioInit(void);
int GpioSet(unsigned bank, unsigned pin);
int GpioClear(unsigned bank, unsigned pin);
/************************** Variable Definitions *****************************/
#ifdef __cplusplus
}
#endif


#endif /* ___GPIO_H___ */

