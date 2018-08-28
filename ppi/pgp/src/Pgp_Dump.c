/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP PPI Register Dump.
**
**  Implementation of PGP PPI register dump.
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

#include <stdio.h>   // printf...

#include "pgp/Pgp.h"

#include "Pgp_p.h"

/*
** status values
*/

static const char _enabled[]      = "enabled";
static const char _disabled[]     = "disabled";
static const char _asserted[]     = "asserted";
static const char _deasserted[]   = "deasserted";

/*
** AXI-Lite block to manage the PGP interface
*/

static const char _line_0[] =   "  Register 0000: = %08X (Counter reset is %s)\n";
static const char _line_1[] =   "  Register 0004: = %08X (Rx reset is %s)\n";
static const char _line_2[] =   "  Register 0008: = %08X (Flush is %s)\n";
static const char _line_3[] =   "  Register 000C: = %08X (Loopback is %s)\n";
static const char _line_4[] =   "  Register 0010: = %08X (Sideband)\n";
static const char _line_5[] =   "  Register 0014: = %08X (Auto status)\n";
static const char _line_6[] =   "  Register 0018: = %08X (Flow control)\n";
static const char _line_7[] =   "  Register 0020: = %08X (Link status)\n";
static const char _line_8[] =   "  Register 0024: = %08X (Remote link data)\n";
static const char _line_9[] =   "  Register 0028: = %08X (Cell errors)\n";
static const char _line_10[] =  "  Register 002C: = %08X (Link down count)\n";
static const char _line_11[] =  "  Register 0030: = %08X (Link errors)\n";
static const char _line_12[] =  "  Register 0034: = %08X (Remote overflow VC0)\n";
static const char _line_13[] =  "  Register 0038: = %08X (Remote overflow VC1)\n";
static const char _line_14[] =  "  Register 003C: = %08X (Remote overflow VC2)\n";
static const char _line_15[] =  "  Register 0040: = %08X (Remote overflow VC3)\n";
static const char _line_16[] =  "  Register 0044: = %08X (Rx frame errors)\n";
static const char _line_17[] =  "  Register 0048: = %08X (Rx frame count)\n";
static const char _line_18[] =  "  Register 004C: = %08X (Local overflow VC0)\n";
static const char _line_19[] =  "  Register 0050: = %08X (Local overflow VC1)\n";
static const char _line_20[] =  "  Register 0054: = %08X (Local overflow VC2)\n";
static const char _line_21[] =  "  Register 0058: = %08X (Local overflow VC3)\n";
static const char _line_22[] =  "  Register 005C: = %08X (Tx frame errors)\n";
static const char _line_23[] =  "  Register 0060: = %08X (Tx frame count)\n";
static const char _line_24[] =  "  Register 0064: = %08X (Rx clock frequency)\n";
static const char _line_25[] =  "  Register 0068: = %08X (Tx clock frequency)\n";
static const char _line_26[] =  "  Register 0070: = %08X (Tx last opcode)\n";
static const char _line_27[] =  "  Register 0074: = %08X (Rx last opcode)\n";
static const char _line_28[] =  "  Register 0078: = %08X (Tx opcode count)\n";
static const char _line_29[] =  "  Register 007C: = %08X (Rx opcode count)\n";


/*
** decoded link status fields...
*/

static const char _rx_phy[]          = "    Rx PHY ready is %s\n";          
static const char _tx_phy[]          = "    Tx PHY ready is %s\n";        
static const char _local_link[]      = "    Local link ready is %s\n";      
static const char _remote_link[]     = "    Remote link ready is %s\n";        
static const char _tx_ready[]        = "    Tx ready is %s\n";
static const char _rx_polarity[]     = "    Rx link polarity %d\n"; 
static const char _remote_pause[]    = "    Remote pause status %d\n";      
static const char _local_pause[]     = "    Local pause status %d\n";        
static const char _remote_overflow[] = "    Remote overflow status %d\n";
static const char _local_overflow[]  = "    Local overflow status %d\n"; 

/*
** decoded sideband register fields...
*/

static const char _sideband_tx[]     = "    Sideband transmit data %d\n";
static const char _sideband_enable[] = "    Sideband is %s\n";  

/*
** decoded status vector fields...
*/

static const char _rx_cell_error[]     = "  Rx cell errors %d\n";            
static const char _rx_frame_error[]    = "  Rx frame errors %d\n";          
static const char _rx_link_down[]      = "  Rx link down count %d\n";        


void Pgp_Dump(uint32_t link)
  {
  uint8_t sock = PGP_DECODE_SOCK(link);
  uint8_t lane = PGP_DECODE_LANE(link);  
   
  extern Pgp_Registers* registers[PGP_NUMOF_PLUGINS];
  
  Pgp_Registers* regs = registers[sock];

  /*
  ** print all configuration and status registers...
  */

  if(lane >= PGP_NUMOF_LANES)
    {
    printf("%s: invalid link %d\n",__func__,link);
    return;   
    } 

  Pgp_Config* config = &(regs[lane].config);

  printf("%s link %d sock %d lane %d register base 0x%x config 0x%x\n",__func__,link,sock,lane,regs,config);

  uint32_t    next;
  const char* state;

  next  = config->cnt_reset;
  state = Pgp_IS(PGP_COUNTERS_RESET, next) ? _asserted : _deasserted;
  printf(_line_0, next, state);

  next  = config->rx_reset;
  state = Pgp_IS(PGP_PHY_RESET, next)      ? _asserted : _deasserted;
  printf(_line_1, next, state);

  next  = config->flush;
  state = Pgp_IS(PGP_FLUSH, next)          ? _asserted : _deasserted;
  printf(_line_2, next, state);

  next  = config->loopback;
  state = Pgp_IS(PGP_LOOPBACK, next)       ? _enabled : _disabled;
  printf(_line_3, next, state);

  next = config->sideband;
  printf(_line_4, next);   
  printf(_sideband_tx, Pgp_DECODE(PGP_SB_DATA, PGP_SB_DATA_LENGTH, next));
  printf(_sideband_enable, Pgp_IS(PGP_SB_ENABLE, next)      ? _enabled : _disabled);

  next  = config->auto_status;
  state = Pgp_IS(PGP_AUTO, next)           ? _enabled : _disabled;
  printf(_line_5, next, state);

  next  = config->flow_ctl;
  state = Pgp_IS(PGP_FLOW_CTL, next)       ? _enabled : _disabled;
  printf(_line_6, next, state);

  next  = config->link_status; 
  printf(_line_7, next);

  printf(_rx_phy,          Pgp_IS(PGP_RX_READY,       next) ? _asserted  : _deasserted);
  printf(_tx_phy,          Pgp_IS(PGP_TX_READY,       next) ? _asserted  : _deasserted);
  printf(_local_link,      Pgp_IS(PGP_LOCAL_READY,    next) ? _asserted  : _deasserted);
  printf(_remote_link,     Pgp_IS(PGP_REMOTE_READY,   next) ? _asserted  : _deasserted);
  printf(_tx_ready,        Pgp_IS(PGP_TRANSMIT_READY, next) ? _asserted  : _deasserted);

  printf(_rx_polarity,     Pgp_DECODE(PGP_RX_POLARITY,     PGP_RX_POLARITY_LENGTH,    next));
  printf(_remote_pause,    Pgp_DECODE(PGP_REMOTE_PAUSE,    PGP_REMOTE_PAUSE_LENGTH,   next));
  printf(_local_pause,     Pgp_DECODE(PGP_LOCAL_PAUSE,     PGP_LOCAL_PAUSE_LENGTH,    next));
  printf(_remote_overflow, Pgp_DECODE(PGP_REMOTE_OVERFLOW, PGP_REMOTE_PAUSE_LENGTH,   next));
  printf(_local_overflow,  Pgp_DECODE(PGP_LOCAL_OVERFLOW,  PGP_LOCAL_OVERFLOW_LENGTH, next));

  /*
  ** Print all counters...
  */

  Pgp_Counters* counters = &(regs[lane].counters);

  printf(_line_8,  counters->rlink_data);
  printf(_line_9,  counters->cell_errors); 
  printf(_line_10, counters->link_down);
  printf(_line_11, counters->link_errors);
  printf(_line_12, counters->overflow_remote[0]); 
  printf(_line_13, counters->overflow_remote[1]); 
  printf(_line_14, counters->overflow_remote[2]); 
  printf(_line_15, counters->overflow_remote[3]); 

  printf(_line_16, counters->rx_errors);
  printf(_line_17, counters->rx_frames);

  printf(_line_18, counters->overflow_local[0]); 
  printf(_line_19, counters->overflow_local[1]); 
  printf(_line_20, counters->overflow_local[2]); 
  printf(_line_21, counters->overflow_local[3]); 

  printf(_line_22, counters->tx_errors);
  printf(_line_23, counters->tx_frames);

  printf(_line_24, counters->rx_clock);
  printf(_line_25, counters->tx_clock);

  printf(_line_26, counters->last_tx);
  printf(_line_27, counters->last_rx);

  printf(_line_28, counters->tx_op);
  printf(_line_29, counters->rx_op);

  return;
  }
