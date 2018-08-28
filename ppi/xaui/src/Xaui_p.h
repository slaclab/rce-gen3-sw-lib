// -*-Mode: C;-*-
/**
@file
@brief This is the PRIVATE interface to the XAUI facility.

@verbatim
                               Copyright 2015
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
XAUI

@author
Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2014/10/30

@par Credits:
SLAC
*/

#include "task/Task.h"
#include "sas/Sas.h"

/* @brief

Defines the size of a resource allocated from mem_rsAlloc
*/

#define RESOURCE_SIZE   2048

/* @brief

Defines the size in log2() of a resource allocated from mem_rsAlloc
*/

#define RESOURCE_STRIDE  11 /* 2048 byte resource */


/* @brief

Defines the mask for rundown messages
*/


#define RUNDOWN_MSG_MASK 0xFFFFFFFC

/* @brief

Used to Reset per packet counters (see "Xaui_Counters")

*/

enum {
XAUI_COUNTERS_RESET   = 0, 
XAUI_COUNTERS_MBZ     = XAUI_COUNTERS_RESET + 1, XAUI_COUNTERS_MBZ_LENGTH = 31};

/* @brief

Used to Reset the PHY

*/

enum {
XAUI_PHY_RESET   = 0, 
XAUI_PHY_MBZ     = XAUI_PHY_RESET + 1, XAUI_PHY_MBZ_LENGTH = 31};
     
/* @brief

This is the bit field structure of the XAUI core configuration vector as defined by the 
"Configuration and Status Vectors" section of the Xilinx XAUI documentation. 
(see page 95 of XUAI v12.1 Xilinx PG053) 

*/

enum {
XAUI_CORE_LOOPBACK     = 0, 
XAUI_CORE_POWER_DOWN   = XAUI_CORE_LOOPBACK + 1,
XAUI_CORE_RESET_FAULTS = XAUI_CORE_POWER_DOWN + 1,
XAUI_CORE_RESET_LINK   = XAUI_CORE_RESET_FAULTS + 1,
XAUI_CORE_TEST         = XAUI_CORE_RESET_LINK + 1,
XAUI_CORE_TEST_PATTERN = XAUI_CORE_TEST+ 1,                                      XAUI_CORE_TEST_PATTERN_LENGTH = 2,
XAUI_CORE_MBZ          = XAUI_CORE_TEST_PATTERN + XAUI_CORE_TEST_PATTERN_LENGTH, XAUI_CORE_MBZ_LENGTH = 25};

/* @brief

Amount of pause time for a sent pause frame

*/

enum {
XAUI_PAUSE_GAP     = 0,                                      XAUI_PAUSE_GAP_LENGTH = 16,
XAUI_PAUSE_GAP_MBZ = XAUI_PAUSE_GAP + XAUI_PAUSE_GAP_LENGTH, XAUI_PAUSE_GAP_MBZ_LENGTH = 16};
 
/* @brief

This is the bit field structure of the XAUI core status vector as defined by the 
"Configuration and Status Vectors" section of the Xilinx XUAI documentation. 
(see page 96 of XUAI v12.1 Xilinx PG053) 

*/

enum {
XAUI_STATUS_TX_FAULT = 0,                                                        // transmit fault (reset by "Xaui_Core_State.reset_fault") 
XAUI_STATUS_RX_FAULT = XAUI_STATUS_TX_FAULT + 1,                                 // receive fault  (reset by "Xaui_Core_State.reset_fault")
XAUI_STATUS_SYNCHED  = XAUI_STATUS_RX_FAULT + 1, XAUI_STATUS_SYNCHED_LENGTH = 4, // receive synchronization with one bit per lane(see table 5-47)
XAUI_STATUS_ALIGNED  = XAUI_STATUS_SYNCHED + XAUI_STATUS_SYNCHED_LENGTH,         // All 4 lanes aligned (table 5-32)
XAUI_STATUS_RX_LINK  = XAUI_STATUS_ALIGNED + 1,                                  // Receive link/up/down (reset by "Xaui_Core_State.reset_rx_link")
XAUI_STATUS_MBZ      = XAUI_STATUS_RX_LINK + 1,  XAUI_STATUS_MBZ_LENGTH = 24};   // 32-bit padding (Must Be Zero)

/* @brief

This is the bit field structure of the XAUI core debug vector.

*/

enum {
XAUI_DEBUG_PHASED  = 0,                                                     // TX phase alignment completed
XAUI_DEBUG_SYNCHED = XAUI_DEBUG_PHASED + 1,  XAUI_DEBUG_SYNCHED_LENGTH = 4, // receive synchronization with one bit per lane(see table 5-47)
XAUI_DEBUG_ALIGNED = XAUI_DEBUG_SYNCHED + XAUI_DEBUG_SYNCHED_LENGTH,        // All 4 lanes aligned (table 5-32)
XAUI_DEBUG_MBZ     = XAUI_DEBUG_ALIGNED + 1, XAUI_DEBUG_MBZ_LENGTH = 26};   // 32-bit padding (Must Be Zero)

/* @brief

This is the bit field structure of the XAUI core control vector.

*/

enum {
XAUI_CTL_RX_SHIFT    = 0, XAUI_CTL_RX_SHIFT_LENGTH = 4, // RX header shift byte count
XAUI_CTL_TX_SHIFT    = XAUI_CTL_RX_SHIFT + XAUI_CTL_RX_SHIFT_LENGTH,  XAUI_CTL_TX_SHIFT_LENGTH = 12, // TX header shift byte count
XAUI_CTL_MAC_FILTER  = XAUI_CTL_TX_SHIFT + XAUI_CTL_TX_SHIFT_LENGTH, XAUI_CTL_MAC_FILTER_LENGTH = 1,     // MAC filter enable
XAUI_CTL_IP_CSUM   = XAUI_CTL_MAC_FILTER + XAUI_CTL_MAC_FILTER_LENGTH, XAUI_CTL_IP_CSUM_LENGTH = 1,     // IP checksum
XAUI_CTL_TCP_CSUM  = XAUI_CTL_IP_CSUM + XAUI_CTL_IP_CSUM_LENGTH, XAUI_CTL_TCP_CSUM_LENGTH = 1,     // TCP checksum
XAUI_CTL_UDP_CSUM  = XAUI_CTL_TCP_CSUM + XAUI_CTL_TCP_CSUM_LENGTH, XAUI_CTL_UDP_CSUM_LENGTH = 1};     // UDP checksum

/* @brief

This is the bit field structure of the XAUI core configuration vector.

*/

typedef struct {
  uint32_t counters;    // Reset RX and TX related counters (see
  uint32_t phy;         // Reset phy interface
  uint32_t core;        // Core configuration;
  uint32_t spare0;      // Spare bytes
  uint32_t pause;       // Pause time in one (1) Nanosecond tics  
  uint32_t mac[2];      // Own MAC address (high order two bytes are MBZ)
  uint32_t rssiIp;      // IP address for RSSI
  uint32_t status;      // Xilinx Core status values
  uint32_t debug;       // Xilinx Core debug configuration
  uint32_t spare[4];    // Don't care... (for alignment)    
  uint32_t ctl;         // Control bits enable/disable hdr shift, csum offload, MAC filter
  uint32_t hdr_size;    // 1+size, default 15 (16 64-bit values)
  uint8_t  hole[192];   // Don't care... (for alignment)    
} Xaui_Config;
 
/* @brief

These are the counters the plug-in maintains on a per-packet basis

*/

typedef struct {
  uint32_t rx;                // # of received frames
  uint32_t tx;                // # of transmitted frames
  uint32_t rxp;               // # of received pause frames
  uint32_t txp;               // # of transmitted pause frames
  uint32_t rx_overflow;       // # receive buffering errors
  uint32_t rx_crc;            // # of RX CRC errors
  uint32_t tx_underflow;      // # of transmit buffering errors;
  uint32_t tx_link_not_ready; // # of link not ready errors;
  uint32_t tx_local_fault;
  uint32_t rx_local_fault;
  uint32_t sync_status[4];
  uint32_t alignment;
  uint32_t rx_link_status;
  uint32_t rx_drops;          // # of RX FIFO drops
} Xaui_Counters;
 
/* @brief

TBD.

*/

typedef struct {
  Xaui_Config   config;    // configuration parameters
  Xaui_Counters counters;  //  counters...
} Xaui_Registers;

/* @brief

TBD

*/

inline uint32_t Xaui_IS(uint32_t this, uint32_t axi_register);
 
/* @brief

TBD

*/

uint32_t Xaui_DECODE(uint32_t this, uint32_t length, uint32_t axi_register);

/* @brief

TBD

*/

inline uint32_t Xaui_ENABLE(uint32_t this, uint32_t axi_register);

/* @brief

TBD

*/

inline uint32_t Xaui_DISABLE(uint32_t this, uint32_t axi_register);

/* @brief

TBD

*/

inline uint32_t Xaui_ENCODE(uint32_t this, uint32_t length, uint32_t value, uint32_t axi_register);

uint32_t Xaui_TxInit(const Task_Attributes *attrs);

void Xaui_RxStats(int reset);

void Xaui_TxStats(int reset);
