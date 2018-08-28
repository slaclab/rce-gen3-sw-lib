// -*-Mode: C++;-*-
/*!@file   dcr.h
*
* @brief   Definition of DCR space for first generation RCE boards
*
* @author  Michael Huffer -- REG/DRD - (mehsys@slac.stanford.edu)
*
* @date    March 27, 2007 -- Created
*
* $Revision: 842 $
*
* @verbatim                    Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef _CONFIGURATION_DCR_RCEG1_DCR_H_
#define _CONFIGURATION_DCR_RCEG1_DCR_H_

// These are #defines to allow them to be used by assembler code

#define FWVERS  0x2f5     // Firmware version number register
#define MEMERST 0x2F6     // RLDRAM read error status register
#define LEDREG  0x2F7     // LED register
#define FLSHOPT 0x2F8     // Flash memory boot options
#define FLSHCMD 0x2F9     // Flash memory command
#define FLSHOFF 0x2FA     // Flash memory data offset
#define FLSHDAT 0x2FB     // Flash memory data port
#define INTNC0  0x2FC     // Non-critical interrupt word 0
#define INTNC1  0x2FD     // Non-critical interrupt word 1
#define INTCR0  0x2FE     // Critical interrupt word 0
#define INTCR1  0x2FF     // Critical interrupt word 1
#define PEB0CSR 0x300     // Pending export block 0 control/status register
#define PEB0DAT 0x301     // Pending export block 0 data FIFO register
#define PEB0FLT 0x302     // Pending export block 0 fault register
#define PEB1CSR 0x304     // Pending export block 1 control/status register
#define PEB1DAT 0x305     // Pending export block 1 data FIFO register
#define PEB1FLT 0x306     // Pending export block 1 fault register
#define PEB2CSR 0x308     // Pending export block 2 control/status register
#define PEB2DAT 0x309     // Pending export block 2 data FIFO register
#define PEB2FLT 0x30A     // Pending export block 2 fault register
#define PEB3CSR 0x30C     // Pending export block 3 control/status register
#define PEB3DAT 0x30D     // Pending export block 3 data FIFO register
#define PEB3FLT 0x30E     // Pending export block 3 fault register
#define PEB4CSR 0x310     // Pending export block 4 control/status register
#define PEB4DAT 0x311     // Pending export block 4 data FIFO register
#define PEB4FLT 0x312     // Pending export block 4 fault register
#define PEB5CSR 0x314     // Pending export block 5 control/status register
#define PEB5DAT 0x315     // Pending export block 5 data FIFO register
#define PEB5FLT 0x316     // Pending export block 5 fault register
#define ECB0CSR 0x340     // Export completion block 0 control/status register
#define ECB0DAT 0x341     // Export completion block 0 data FIFO register
#define ECB0FLT 0x342     // Export completion block 0 fault register
#define ECB1CSR 0x344     // Export completion block 1 control/status register
#define ECB1DAT 0x345     // Export completion block 1 data FIFO register
#define ECB1FLT 0x346     // Export completion block 1 fault register
#define ECB2CSR 0x348     // Export completion block 2 control/status register
#define ECB2DAT 0x349     // Export completion block 2 data FIFO register
#define ECB2FLT 0x34A     // Export completion block 2 fault register
#define FLB0CSR 0x380     // Free list block 0 control/status register
#define FLB0DAT 0x381     // Free list block 0 data FIFO register
#define FLB0FLT 0x382     // Free list block 0 fault register
#define FLB1CSR 0x384     // Free list block 1 control/status register
#define FLB1DAT 0x385     // Free list block 1 data FIFO register
#define FLB1FLT 0x386     // Free list block 1 fault register
#define FLB2CSR 0x388     // Free list block 2 control/status register
#define FLB2DAT 0x389     // Free list block 2 data FIFO register
#define FLB2FLT 0x38A     // Free list block 2 fault register
#define PIB0CSR 0x3C0     // Pending import block 0 control/status register
#define PIB0DAT 0x3C1     // Pending import block 0 data FIFO register
#define PIB0FLT 0x3C2     // Pending import block 0 fault register
#define PIB1CSR 0x3C4     // Pending import block 1 control/status register
#define PIB1DAT 0x3C5     // Pending import block 1 data FIFO register
#define PIB1FLT 0x3C6     // Pending import block 1 fault register
#define PIB2CSR 0x3C8     // Pending import block 2 control/status register
#define PIB2DAT 0x3C9     // Pending import block 2 data FIFO register
#define PIB2FLT 0x3CA     // Pending import block 2 fault register
#define PIB3CSR 0x3CC     // Pending import block 3 control/status register
#define PIB3DAT 0x3CD     // Pending import block 3 data FIFO register
#define PIB3FLT 0x3CE     // Pending import block 3 fault register
#define PIB4CSR 0x3D0     // Pending import block 4 control/status register
#define PIB4DAT 0x3D1     // Pending import block 4 data FIFO register
#define PIB4FLT 0x3D2     // Pending import block 4 fault register
#define PIB5CSR 0x3D4     // Pending import block 5 control/status register
#define PIB5DAT 0x3D5     // Pending import block 5 data FIFO register
#define PIB5FLT 0x3D6     // Pending import block 5 fault register

#endif
