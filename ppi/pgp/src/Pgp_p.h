// -*-Mode: C;-*-
/**
@file
@brief This is the PRIVATE interface to the PGP facility.

@verbatim
                               Copyright 2015
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
PGP

@author
Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2015/11/09

@par Credits:
SLAC
*/


/* @brief

Used to Reset per packet counters (see "Pgp_Counters")

*/

enum {
PGP_COUNTERS_RESET   = 0, 
PGP_COUNTERS_MBZ     = PGP_COUNTERS_RESET + 1, PGP_COUNTERS_MBZ_LENGTH = 31};

/* @brief

Used to Reset the Rx PHY

*/

enum {
PGP_PHY_RESET   = 0, 
PGP_PHY_MBZ     = PGP_PHY_RESET + 1, PGP_PHY_MBZ_LENGTH = 31};

/* @brief

Used to flush PGP

*/

enum {
PGP_FLUSH       = 0, 
PGP_FLUSH_MBZ   = PGP_FLUSH + 1, PGP_FLUSH_MBZ_LENGTH = 31};

/* @brief

Used to enable PGP loopback

*/

enum {
PGP_LOOPBACK      = 0, 
PGP_LOOPBACK_MBZ  = PGP_LOOPBACK + 2, PGP_LOOPBACK_MBZ_LENGTH = 30};

/* @brief

Used to set the sideband data

*/
 
enum {
PGP_SB_DATA     = 0, PGP_SB_DATA_LENGTH = 8,
PGP_SB_ENABLE   = PGP_SB_DATA + PGP_SB_DATA_LENGTH, PGP_SB_ENABLE_LENGTH = 1,
PGP_SB_MBZ      = PGP_SB_ENABLE + PGP_SB_ENABLE_LENGTH, PGP_SB_MBZ_LENGTH = 23};

/* @brief

Used to enable the PPI auto status send 

*/

enum {
PGP_AUTO      = 0, 
PGP_AUTO_MBZ  = PGP_AUTO + 1, PGP_AUTO_MBZ_LENGTH = 31};


/* @brief

Used to disable flow control

*/

enum {
PGP_FLOW_CTL      = 0, 
PGP_FLOW_CTL_MBZ  = PGP_FLOW_CTL + 1, PGP_FLOW_CTL_MBZ_LENGTH = 31};
     
/* @brief

This is the bit field structure of the PGP core configuration vector

*/

enum {
PGP_RX_READY        = 0, 
PGP_TX_READY        = PGP_RX_READY        + 1,
PGP_LOCAL_READY     = PGP_TX_READY        + 1,
PGP_REMOTE_READY    = PGP_LOCAL_READY     + 1,
PGP_TRANSMIT_READY  = PGP_REMOTE_READY    + 1,
PGP_MBZ_0           = PGP_TRANSMIT_READY  + 1,                          PGP_MBZ_0_LENGTH           = 3,
PGP_RX_POLARITY     = PGP_MBZ_0           + PGP_MBZ_0_LENGTH,           PGP_RX_POLARITY_LENGTH     = 2,
PGP_MBZ_1           = PGP_RX_POLARITY     + PGP_RX_POLARITY_LENGTH,     PGP_MBZ_1_LENGTH           = 2,
PGP_REMOTE_PAUSE    = PGP_MBZ_1           + PGP_MBZ_1_LENGTH,           PGP_REMOTE_PAUSE_LENGTH    = 4,
PGP_LOCAL_PAUSE     = PGP_REMOTE_PAUSE    + PGP_REMOTE_PAUSE_LENGTH,    PGP_LOCAL_PAUSE_LENGTH     = 4,
PGP_REMOTE_OVERFLOW = PGP_LOCAL_PAUSE     + PGP_LOCAL_PAUSE_LENGTH,     PGP_REMOTE_OVERFLOW_LENGTH = 4,
PGP_LOCAL_OVERFLOW  = PGP_REMOTE_OVERFLOW + PGP_REMOTE_OVERFLOW_LENGTH, PGP_LOCAL_OVERFLOW_LENGTH  = 4};
    
/* @brief

These are the configuration registers the plug-in maintains on a per-link basis

*/

typedef struct {
  uint32_t cnt_reset;          // Count Reset
  uint32_t rx_reset;           // Reset Rx
  uint32_t flush;              // Flush
  uint32_t loopback;           // Loop Back
  uint32_t sideband;           // Sideband data to transmit, Sideband data enable
  uint32_t auto_status;        // Auto Status Send Enable (PPI)
  uint32_t flow_ctl;           // Disable Flow Control
  uint32_t hole;               // Ignore
  uint32_t link_status;        // Link status
 } Pgp_Config;

/* @brief

These are the counters the plug-in maintains on a per-link basis

*/
 
typedef struct {   
  uint32_t rlink_data;         // Remote Link Data
  uint32_t cell_errors;        // Cell Error Count
  uint32_t link_down;          // Link Down Count
  uint32_t link_errors;        // Link Error Count
  uint32_t overflow_remote[4]; // Remote Overflow VC Count
  uint32_t rx_errors;          // Receive Frame Error Count
  uint32_t rx_frames;          // Receive Frame Count
  uint32_t overflow_local[4];  // Local Overflow VC Count
  uint32_t tx_errors;          // Transmit Frame Error Count
  uint32_t tx_frames;          // Transmit Frame Count
  uint32_t rx_clock;           // Receive Clock Frequency
  uint32_t tx_clock;           // Transmit Clock Frequency
  uint32_t last_tx;            // Last OpCode Transmitted
  uint32_t last_rx;            // Last OpCode Received
  uint32_t tx_op;              // OpCode Transmit count
  uint32_t rx_op;              // OpCode Received count
} Pgp_Counters;

/* @brief

TBD.

*/

typedef struct {
  Pgp_Config   config;    // configuration parameters
  Pgp_Counters counters;  // counters...
  uint32_t     hole[993];
} Pgp_Registers;

void Pgp_Dump(uint32_t link);

