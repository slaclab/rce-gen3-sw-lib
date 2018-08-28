// -*-Mode: C;-*-
/**
@file
@brief This is a PRIVATE interface to the Socket Abstraction Services (SAS) facility.  
This file provides constants, macros, and memory maps for private structures.

@verbatim
                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.
@endverbatim

@par Facility:
SAS

@author
S. Maldonado, SLAC (smaldona@slac.stanford.edu)

@par Date created:
2014/06/03

@par Last commit:
\$Date: $ by \$Author: $.

@par Revision number:
\$Revision: $

@par Credits:
SLAC
*/

#ifndef SAS_MAP_H
#define SAS_MAP_H

#include "sas/Sas.h"

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- *//*

  The On-Chip Memory, or OCM, is reserved for exclusive use by the
  SAS facility.
  
  The layout of the 256KB OCM is composed of fixed length structures
  and a variable length memory pool.

  This file describes the layout of the OCM memory map
  and provides constants and macros to assist in the access of
  structure data.

  OCM Memory Map - 256KB
  ----------------------------------------
  |  Reserved storage                    |
  |  Metrics table                       |
  |  Faults table                        |
  |  Plugin registers table              |
  |  Socket KVT offset                   |
  |  Application KVT offset              |
  |  Descriptor pool offset              |
  |  Interrupt sync table (8*32 entries) |
  |  Interrupt map table (64 entries)    |
  |  Socket Attributes*4                 |
  |  Session*32                          |
  |  Mailbox*31 (rundown)                |
  |  Mailbox*4  (socket)                 |
  |  Mailbox*8  (application)            |
  ----------------------------------------
  |  Socket key value table (KVT)        |
  |  Application key value table (KVT)   |
  ----------------------------------------  
  |                                      |
  |  Frame Descriptor Pool               |
  |                                      |
  ----------------------------------------

*//* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- *//*

  Sections of the AXI GP0 memory are reserved for exclusive use by the
  SAS facility and firmware.
  
  The layout of the AXI GP0 memory is composed of fixed length
  register blocks as specified in the TBD Firmware documentation.

  This file describes the layout of the AXI GP0 memory map
  and provides constants and macros to assist in the access of
  structure data.

  AXI GP0 Memory Map
  ----------------------------------------------------------
  |  Sections                       Physical Address       |
  ----------------------------------------------------------
  |  Interrupt Remap Registers      0x40000000             |
  |  Interrupt Control Registers    0x40008000             |
  |  Socket0 Mgmt                   0x50000000             |
  |  Socket0 FIFOs                  0x50010000             |
  |  Socket1 Mgmt                   0x50020000             |
  |  Socket1 FIFOs                  0x50030000             |
  |  Socket2 Mgmt                   0x50040000             |
  |  Socket2 FIFOs                  0x50050000             |
  |  Socket3 Mgmt                   0x50060000             |
  |  Socket3 FIFOs                  0x50070000             |
  |  Rundown FIFO Base              0x50080000             |
  |  Utility FIFO Base              0x50080100             |
  ----------------------------------------------------------

*//* ---------------------------------------------------------------------- */

/*
**
** This constant specifies the maximum number of plugin sockets. 
**
*/

#define MAX_SOCKETS                           4

/*
**
** This constant defines the maximum interrupt groups.
**
**
*/

#define MAX_INTR_GROUPS                       8

/*
**
** This constant defines the maximum sources for an interrupt group.
**
**
*/

#define MAX_INTR_SOURCES                      32

/*
**
** This constant defines the maximum interrupts sources in the
** metrics interrupt group.
**
** While there are only 21 metrics sources, 32 have been 
** reserved to account for future expansion.
**
*/

#define MAX_INTR_METRICS                      32

/*
**
** These constants define the fixed offsets of metrics interrupt sources.
**
*/

#define METRICS_MAX_SOCKET                    3

#define METRICS_IBFULL_OFFSET                 0      

#define METRICS_OBEMPTY_OFFSET                1

#define METRICS_OBFULL_OFFSET                 2

#define METRICS_UTILFULL_OFFSET               12

#define METRICS_UTILEMPTY_OFFSET              16

#define METRICS_FAULT_OFFSET                  31

/*
**
** This constant defines the maximum count of transfer fault
** syndrome metrics per engine.
**
*/

#define MAX_FAULT_SYNDROMES                   4

/*
**
** This constant defines the maximum count of sessions.
**
**
*/

#define MAX_SESSIONS                          32

/*
**
** This constant defines the maximum count of inputs a session can manage.
**
**
*/

#define MAX_SESSION_INPUTS                    8

/*
**
** This constant defines the maximum count of rundown channels.
**
**
*/

#define MAX_RUNDWN_CHANNELS                   31

/*
**
** This constant defines the maximum count of application fifos.
**
**
*/

#define MAX_APP_CHANNELS                      8

/*
**
** This constant defines the maximum count of utility fifos
** used by the SAS facility.
**
** chan 0 - session struct offsets
** chan 1 - rundown mailbox struct offsets
**
*/

#define MAX_UTIL_CHANNELS                     2

/* This value defines the enable mask for an interrupt map entry */

#define INTR_MAP_ENAB_MASK                    0x80000000

/*
**
** This constant specifies the byte offset from the axi base where the
** interrupt source map table starts.
**
*/

#define INTR_MAP_AXI_BASE                     0x00000000

/*
**
** This constant specifies the byte offset from the axi base where the
** interrupt group enable/disable I/O map starts.
**
*/

#define INTR_GROUP_AXI_BASE                   0x00008000

/*
**
** This constant specifies the byte offset from the axi base where the socket
** I/O map starts.
**
*/

#define SOCKET_AXI_BASE                       0x10000000

/*
**
** This constant specifies the size in bytes of a socket
** I/O map.
**
*/

#define SOCKET_AXI_SIZE                       0x20000

/*
**
** This constant specifies the byte offset from an axi socket base where its
** FIFO map starts.
**
*/

#define SOCKET_AXI_FIFO_BASE                  0x10000

/*
**
** This constant specifies the byte offset from the axi base for the
** rundown fifo I/O.
**
*/

#define RUNDWN_AXI_BASE                       0x10080000

/*
**
** This constant specifies the byte offset from the axi base for the utility
** free-lists.
**
*/

#define UTIL_AXI_FIFO_BASE                    0x10080100

/*
**
** This constant specifies the byte offset from the axi base for the
** application fifo I/O.
**
** SEMTODO : the app fifo base must be modified to match firmware
**
*/

#define APP_AXI_FIFO_BASE                     0x10080200

/*
**
** These are macros for accessing a private socket structure
** using a session handle.
**
*/

#define SOCKID(sock,axi) (((uint32_t)sock - axi - \
                            SOCKET_AXI_FIFO_BASE - SOCKET_AXI_BASE)/SOCKET_AXI_SIZE)
#define SOCKMASK(s)      (1<<SOCKID(s))

/*
**
** These are macros for accessing a private utility map structure
** using a session handle.
**
*/

#define UTIL(axi) ((UtilMap*)(axi+UTIL_AXI_FIFO_BASE))

/*
**
** These are macros for accessing a private ocm map structure
** using a session handle.
**
*/

#define OCMMAP(ocm) ((OcmMap*)ocm)

/*
** ++
**
** Bit fiddling macros.
**
** --
*/

#define BITSET(field, word) (word |=  (1 << field))
#define BITCLR(field, word) (word &= ~(1 << field))

/*
** ++
**
** Configuration structure for mapping interrupt sources to 
** group source inputs.
**
** --
*/

typedef struct _InterruptMap {
  uint8_t  source;        /*!< The interrupt source ID */
  uint8_t  group;         /*!< The interrupt group ID */
  uint16_t enable;        /*!< The interrupt source enable */
} InterruptMap;

/*
** ++
**
** AXI map of interrupt source remapping registers
**
** --
*/

typedef struct _InterruptRemap  {
  volatile uint32_t src[MAX_INTR_GROUPS][MAX_INTR_SOURCES];
                                /*!< Interrupt source remap table */
} InterruptRemap;

/*
** ++
**
** AXI map of interrupt source control registers
**
** --
*/

typedef struct _InterruptCtl  {
  volatile uint32_t enable;     /*!< Group interrupt enable word */
  volatile uint32_t spare0;     /*!< Reserved location */
  volatile uint32_t disable;    /*!< Group interrupt disable word */
  volatile uint32_t spare1;     /*!< Reserved location */
} InterruptCtl;

/*
** ++
**
** AXI map of socket management I/O
**
** --
*/

typedef struct _SocketMgmt {
  volatile uint32_t state;      /*!< Socket on/off/disable state control */
           uint32_t pad0[2];    /*!< Reserved location */
  volatile uint32_t errReset;   /*!< Socket error reset */
  volatile SAS_Errors errors;   /*!< Socket error storage */
} SocketMgmt;

/*
** ++
**
** AXI map of socket I/O
**
** --
*/

typedef struct _Socket {
  volatile uint32_t obFreeList; /*!< Outbound free-list fifo */
  volatile uint32_t ibPendList; /*!< Inbound pending fifo */
           uint32_t pad2[126];  /*!< Reserved location */
  volatile uint32_t obWorkList; /*!< Outbound work list fifo */
           uint32_t pad3[15];   /*!< Reserved location */
  volatile uint32_t ibWorkList; /*!< Inbound work list fifo */
} Socket;

/*
** ++
**
** AXI map of rundown channel I/O
**
** --
*/

typedef struct _RundwnMap {
  volatile uint32_t channel[MAX_RUNDWN_CHANNELS]; /*!< Rundown fifos */
  volatile uint32_t fault;                        /*!< Rundown fault fifo */
} RundwnMap;

/*
** ++
**
** AXI map of utility free list I/O
**
** --
*/

typedef struct _UtilMap {
  volatile uint32_t session;     /*!< The free-list for session offsets */
  volatile uint32_t rundwn;      /*!< The free-list for rundown mailbox offsets */
  volatile uint32_t reserved[2]; /*!< Reserved utility fifos */
} UtilMap;

/*
** ++
**
** OCM map of socket transfer faults
**
** index 0: read error
** index 1: write error
** index 2: frame error
** index 3: overflow error
**
** --
*/

typedef struct _FaultTable  {
  uint32_t ib[MAX_FAULT_SYNDROMES];  /*!< Inbound engine fault counter */
  uint32_t ob[MAX_FAULT_SYNDROMES];  /*!< Outbound engine fault counter */
} FaultTable;

/*
** ++
**
** OCM map of synchronization object to interrupt source table.
**
** --
*/

typedef struct _InterruptTable  {
  uint32_t sync[MAX_INTR_GROUPS][MAX_INTR_SOURCES]; 
                                   /*!< Table of interrupt sync objects */
} InterruptTable;

/*
** ++
**
** Forward reference for the private common mailbox structure.
**
** --
*/

struct _Mbx;
typedef struct _Mbx *Mbx;

/*
** ++
**
** Signature for a common mailbox handler
**
** --
*/

typedef uint32_t (*Handler) (uint32_t, void *ctx, Mbx mbx);

/*
** ++
**
** OCM map of the common mailbox structure.
** This structure should be cache aligned.
**
** --
*/

struct _Mbx  {
  SAS_Session s;           /*!< Session handle */
  uint32_t    ocm;         /*!< Ocm handle */
  uint8_t     idx;         /*!< Mailbox session index */
  uint8_t     mid;         /*!< Mailbox ID */
  uint16_t    enab;        /*!< Interrupt control register offset */
  uint16_t    mask;        /*!< Interrupt enable offset (cannot exceed 64k) */
  uint16_t    intrBase;    /*!< The interrupt source base */
  uint32_t    pend;        /*!< Axi mapped fifo register address */
  Handler     handler;     /*!< Mailbox handler routine */
  Socket     *socket;      /*!< Axi mapped socket handle */
  void       *context;     /*!< Mailbox handler context argument */
};

/*
** ++
**
** Unnamed mailbox structure.
**
** --
*/

struct _SAS_Mbx    {struct _Mbx m;};

/*
** ++
**
** Inbound mailbox structure.
**
** --
*/

struct _SAS_IbMbx  {struct _Mbx m;};

/*
** ++
**
** Outbound mailbox structure.
**
** --
*/

struct _SAS_ObMbx  {struct _Mbx m;};

/*
** ++
**
** Application mailbox structure.
**
** --
*/

struct _SAS_AppMbx {struct _Mbx m;};

/*
** ++
**
** OCM map of session control
** This structure should be cache aligned.
**
** --
*/

struct _SAS_Session  {
  uint8_t     enabled;                  /*!< Bit mask of enabled mailboxes */
  uint8_t     profile;                  /*!< SAS_Wait profile control */
  uint16_t    srcs;                     /*!< Count of mailboxes */
  uint32_t    sync;                     /*!< Interrupt synchronization */
  uint32_t    axi;                      /*!< Mapped axi base */
  uint32_t    ocm;                      /*!< Mapped ocm base */
  uint32_t    valid;                    /*!< SAS_Wait valid profile counter */
  uint32_t    invalid;                  /*!< SAS_Wait invalid profile counter */
  uint32_t    wait;                     /*!< SAS_Wait wait profile counter */
  uint32_t    disable;                  /*!< SAS_Wait disable profile counter */
  uint32_t    pend[MAX_SESSION_INPUTS]; /*!< Axi mapped fifo register addresses */
  struct _Mbx mbx[MAX_SESSION_INPUTS];  /*!< Mailbox storage */
};

/*
** ++
**
** OCM map of fixed data structures.
**
** SEMTODO: remove workaround for missing fw app fifo interface
** --
*/

typedef struct _OcmMap  {
  uint32_t            reserved[8];                    /*!< OCM reserved locations */
  uint32_t            metricsTable[MAX_INTR_METRICS]; /*!< Metrics table */
  FaultTable          faultTable[MAX_SOCKETS];        /*!< Transfer faults table */
  InterruptTable      intrTable;                      /*!< Interrupt sync table */
  InterruptMap        intrMap[SAS_INTR_COUNT];        /*!< Interrupt mapping table */
  struct _SAS_Session sessions[MAX_SESSIONS];         /*!< Session structures */
  SAS_Preferences     plugins[MAX_SOCKETS];           /*!< Plugin configuration */
  uint32_t            appTable[MAX_APP_CHANNELS];     /*!< Application fifos */
  uint32_t            socketKvt;                      /*!< Offset of socket kvt */
  uint32_t            appKvt;                         /*!< Offset of app mailbox kvt */
  uint32_t            descriptorBase;                 /*!< Descriptor pool base */
  uint32_t            descriptorFree;                 /*!< Descriptor pool free */
} OcmMap;

/*
** ++
**
** Externs for storing common memory maps for the OCM and AXI.
**
** --
*/

extern uint32_t       SasAxi;
extern uint32_t       SasOcm;
extern OcmMap        *SasOcmMap;

/**  

This function initializes the SAS facility.
  
*/

SAS_Status SAS_Construct(void);


/**  

This function configures a socket assigns its plugin.
  
*/

SAS_Status SAS_Plugin(const SAS_Preferences*);

/**  

This function registers a plug-in name with an application slot.
  
*/

SAS_Status SAS_AppRegister(const SAS_AppPreferences*);

#endif /* SAS_MAP_H */
