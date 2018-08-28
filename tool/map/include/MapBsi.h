/*
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Sergio Maldonado SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	    000 - July 19 2013
**
**  Revision History:
**	    None.
**
** --
*/

#ifndef MAPBSI_H
#define MAPBSI_H

/*  
 *  This is the offset of the BSI from the start of the
 *  AXI1 region at 0x80000000.
 */
#define BSI_AXI_MAP_OFFSET  0x04000000

/* This is the size of the BSI block RAM in bytes*/
#define BSI_MAP_SIZE        0x800 /* 2k */

#endif /* MAPBSI_H */
