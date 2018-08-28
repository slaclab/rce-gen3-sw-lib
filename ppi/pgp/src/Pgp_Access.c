/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP Register Access Routines
**
**  Implementation of PGP register encode/decode routines.     
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - November 09, 2015
**
**  Revision History:
**	None.
**
** --
*/

#include "pgp/Pgp.h"

/*
** ++
**
**
** --
*/

inline uint32_t Pgp_ENABLE(uint32_t this, uint32_t axi_register) 
  { 
  uint32_t mask   = ~(~(0xFFFFFFFF << 1) << this);  
  uint32_t result = axi_register & mask; 
  
  return result | (1 << this);
  }

/*
** ++
**
**
** --
*/

inline uint32_t Pgp_DISABLE(uint32_t this, uint32_t axi_register) 
  {
  uint32_t mask = ~(~(0xFFFFFFFF << 1) << this);

  return axi_register & mask;
  }

/*
** ++
**
**
** --
*/

inline uint32_t Pgp_ENCODE(uint32_t this, uint32_t length, uint32_t value, uint32_t axi_register)
  {

  uint32_t mask   = ~(~(0xFFFFFFFF << length) << this);
  uint32_t result = axi_register & mask;

  return result | (value << this);
  }
 
/*
** ++
**
**
** --
*/

inline uint32_t Pgp_IS(uint32_t this, uint32_t axi_register) 
  {
  return (1 << this) & axi_register;
  }
 
/*
** ++
**
**
** --
*/

uint32_t Pgp_DECODE(uint32_t this, uint32_t length, uint32_t axi_register) 
  {
  return (axi_register >> this) & ((1 << length) - 1);
  }
