/*
** ++
**  Package: XAUI
**	
**
**  Abstract: XAUI Register Dump Routines
**
**  Implementation of Xaui register dump routines.
**
**  Author:
**      Michael Huffer, SLAC (mehsys@slac.stanford.edu)
**
**  Creation Date:
**	000 - August 19, 2014
**
**  Revision History:
**	None.
**
** --
*/

#include <stdio.h>   // printf...

#include "xaui/Xaui.h"
#include "Xaui_p.h"

/*
** TBD
*/

static const char _enabled[]      = "enabled";
static const char _disabled[]     = "disabled";
static const char _asserted[]     = "asserted";
static const char _deasserted[]   = "deasserted";
static const char _up[]           = "up";
static const char _down[]         = "down";
static const char _not_synched[]  = "not synchronized";
static const char _are_synched[]  = "are synchronized";
static const char _complete[]     = "complete";
static const char _not_complete[] = "not complete";
static const char _aligned[]      = "aligned";
static const char _not_aligned[]  = "not aligned";

/*
** TBD...
*/

static const char _line_0[] =  "  Register 0000: = %08X (Reset counter is %s)\n";
static const char _line_1[] =  "  Register 0004: = %08X (Reset PHY is %s)\n";
static const char _line_2[] =  "  Register 0008: = %08X (Core parameters)\n";
static const char _line_4[] =  "  Register 0010: = %08X (Pause frame time is %d)\n";
static const char _line_5[] =  "  Register 0014: = %08X%08X (MAC address)\n";
static const char _line_6[] =  "  Register 001C: = %08X (RSSI IP)\n";
static const char _line_7[] =  "  Register 0020: = %08X (Core status)\n";
static const char _line_8[] =  "  Register 0024: = %08X (Debug status)\n";
static const char _line_9[] =  "  Register 0038: = %08X (Control bits)\n";
static const char _line_10[] = "  Register 003C: = %08X (Header size)\n";

/*
** decoded core fields...
*/

static const char _loopback[]     = "    Loopback is %s\n";            
static const char _power_down[]   = "    Power-down is %s\n";          
static const char _reset_faults[] = "    Reset faults is %s\n";        
static const char _reset_link[]   = "    Reset link is %s\n";          
static const char _test[]         = "    Test pattern is %s\n";        
static const char _test_pattern[] = "    Test pattern is set to %d\n"; 

/*
** decoded status register fields...
*/

static const char _tx_fault[] = "    Transmit (TX) fault is %s\n"; 
static const char _rx_fault[] = "    Receive (RX) fault is %s\n";  
static const char _link[]     = "    Receive (RX) Link is %s\n";   
static const char _synched[]  = "    Receive (RX) lanes are %s\n"; 

/*
** decoded debug register fields...
*/

static const char _phased[]        = "    Transmit (TX) phase alignment is %s\n";                                 
static const char _lanes_synched[] = "    Bit-list of synchronized lanes is %01X (synchronized if offset SET)\n"; 
static const char _lanes_aligned[] = "    Receive (RX) lanes are %s\n";                                           

/*
** decoded ctl fields...
*/

static const char _rxShiftCnt[]   = "    Receive (RX) header shift bytes %d\n";            
static const char _txShiftCnt[]   = "    Transmit (TX) header shift bytes %d\n";               
static const char _macFilter[]    = "    MAC filter is %s\n";        
static const char _ipCsum[]       = "    IP checksum offload is %s\n"; 
static const char _tcpCsum[]      = "    TCP checksum offload is %s\n"; 
static const char _udpCsum[]      = "    UDP checksum offload is %s\n"; 

/*
** XAUI MAC dump strings
*/

static const char _counter_0[] = "  Register 0100: = %u (# of received (RX) frames)\n";
static const char _counter_1[] = "  Register 0104: = %u (# of transmitted (TX) frames)\n";
static const char _counter_2[] = "  Register 0108: = %u (# of receive (RX) pause frames)\n";
static const char _counter_3[] = "  Register 010C: = %u (# of transmit (TX) pause frames)\n";
static const char _counter_4[] = "  Register 0110: = %u (# of receive (RX) overflows)\n";
static const char _counter_5[] = "  Register 0114: = %u (# of received (RX) CRC errors)\n";
static const char _counter_6[] = "  Register 0118: = %u (# of transmit (TX) under runs)\n";
static const char _counter_7[] = "  Register 011C: = %u (# of transmit (TX) link not ready)\n";
static const char _counter_8[] = "  Register 0120: = %u (# of receive (RX) frame drops)\n";

void Xaui_Dump()
 {
  
  extern Xaui_Registers* _Xaui_Registers;
  
 /*
 ** print all configuration and status registers...
 */
  
 Xaui_Config* config = &(_Xaui_Registers->config);

 uint32_t    next;
 const char* state;
 
 next  = config->counters;
 state = Xaui_IS(XAUI_COUNTERS_RESET, next) ? _asserted : _deasserted;

 printf(_line_0, next, state);
 
 next  = config->phy;
 state = Xaui_IS(XAUI_PHY_RESET, next) ? _asserted : _deasserted;

 printf(_line_1, next, state);

 next  = config->core;
 
 printf(_line_2, next);
  
 printf(_loopback,     Xaui_IS(XAUI_CORE_LOOPBACK,     next) ? _enabled  : _disabled);
 printf(_power_down,   Xaui_IS(XAUI_CORE_POWER_DOWN,   next) ? _enabled  : _disabled);
 printf(_reset_faults, Xaui_IS(XAUI_CORE_RESET_FAULTS, next) ? _asserted : _deasserted);
 printf(_reset_link,   Xaui_IS(XAUI_CORE_RESET_LINK,   next) ? _asserted : _deasserted);
 printf(_test,         Xaui_IS(XAUI_CORE_TEST,         next) ? _enabled  : _disabled);
 printf(_test_pattern, Xaui_DECODE(XAUI_CORE_TEST_PATTERN, XAUI_CORE_TEST_PATTERN_LENGTH, next));

 next  = config->pause;
 
 printf(_line_4, next, Xaui_DECODE(XAUI_PAUSE_GAP, XAUI_PAUSE_GAP_LENGTH, next));

 printf(_line_5, config->mac[1] & 0xFFFF, config->mac[0]);

 next  = config->rssiIp;
 
 printf(_line_6, next);

 next  = config->status;
 
 printf(_line_7, next);

 printf(_tx_fault, Xaui_IS(XAUI_STATUS_TX_FAULT, next) ? _asserted    : _deasserted);
 printf(_rx_fault, Xaui_IS(XAUI_STATUS_RX_FAULT, next) ? _asserted    : _deasserted);
 printf(_synched,  Xaui_IS(XAUI_STATUS_SYNCHED,  next) ? _are_synched : _not_synched);
 printf(_link,     Xaui_IS(XAUI_STATUS_RX_LINK,  next) ? _up          : _down);

 next  = config->debug;

 printf(_line_8, next);

 printf(_phased,        Xaui_IS(XAUI_DEBUG_PHASED, next) ? _complete    : _not_complete);
 printf(_lanes_synched, Xaui_DECODE(XAUI_DEBUG_SYNCHED, XAUI_DEBUG_SYNCHED_LENGTH, next));
 printf(_lanes_aligned, Xaui_IS(XAUI_DEBUG_ALIGNED,  next) ? _aligned : _not_aligned);
 
 next = config->ctl;
 
 printf(_line_9, next);
 
 printf(_rxShiftCnt, Xaui_DECODE(XAUI_CTL_RX_SHIFT, XAUI_CTL_RX_SHIFT_LENGTH, next));
 printf(_txShiftCnt, Xaui_DECODE(XAUI_CTL_TX_SHIFT, XAUI_CTL_TX_SHIFT_LENGTH, next));
 printf(_macFilter, Xaui_IS(XAUI_CTL_MAC_FILTER, next) ? _enabled : _disabled);
 printf(_ipCsum, Xaui_IS(XAUI_CTL_IP_CSUM, next) ? _enabled : _disabled);
 printf(_tcpCsum, Xaui_IS(XAUI_CTL_TCP_CSUM, next) ? _enabled : _disabled);
 printf(_udpCsum, Xaui_IS(XAUI_CTL_UDP_CSUM, next) ? _enabled : _disabled);

 next = config->hdr_size;
 
 printf(_line_10, next & 0xF);
 
 /*
 ** Print all counters...
 */
 
 Xaui_Counters* counters = &(_Xaui_Registers->counters);
 
 printf(_counter_0, counters->rx); 
 printf(_counter_1, counters->tx); 
 printf(_counter_2, counters->rxp);
 printf(_counter_3, counters->txp);
 printf(_counter_4, counters->rx_overflow); 
 printf(_counter_5, counters->rx_crc); 
 printf(_counter_6, counters->tx_underflow); 
 printf(_counter_7, counters->tx_link_not_ready); 
 printf(_counter_8, counters->rx_drops);

 return;
 }
