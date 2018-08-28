/*
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	    000 - July 28, 2013
**
**  Revision History:
**	    None.
**
** --
*/

#ifndef MAPGIC_H
#define MAPGIC_H

/*  
 *  This is the offset of the GIC from the start of the
 *  HIGHREG region at 0xF8000000 
 */
#define GIC_CPU_MAP_OFFSET   0x00F01000

#define GIC_MAP_SIZE         0x00001000 /* 4k */

#endif /* MAPGIC_H */
