/*!
*
* @brief   Functions for loading a firmware bitstream onto an FPGA
*
* @author  R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date    October 17, 2013 -- Created
*
* $Revision: 3096 $
*
* @verbatim:
*                               Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/
#ifndef TOOL_FPGA_FPGA_H
#define TOOL_FPGA_FPGA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct FPGA FPGA_t;

struct FPGA* TOOL_FPGA_initialize(uint16_t devId);
void     TOOL_FPGA_teardown(struct FPGA* fpga);
unsigned TOOL_FPGA_loadBin(struct FPGA* fpga, uint8_t* src, size_t len);
unsigned TOOL_FPGA_loadBit(struct FPGA* fpga, uint8_t* src, size_t len);
unsigned TOOL_FPGA_enable(struct FPGA* fpga);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
