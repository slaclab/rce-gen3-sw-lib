// -*-Mode: C;-*-
/**
This is the PUBLIC interface to the Socket Abstraction Services (SAS) facility.  
This file provides library level access to the SAS facility.


                               Copyright 2014
                                    by
                       The Board of Trustees of the
                    Leland Stanford Junior University.
                           All rights reserved.


Facility:
SAS

Author
S. Maldonado, SLAC (smaldona@slac.stanford.edu)

Date created:
2014/06/02

Credits:
SLAC
*/

#ifndef SAS
#define SAS 5 /* Facility no., see configuration/system/faciliy.doc. */

#include "system/statusCode.h"
#include "sas/Sas_Config.h"

/* typedefs, forward references */

struct         _SAS_Session;
typedef struct _SAS_Session* SAS_Session; // Open session handle.

struct         _SAS_Mbx;
typedef struct _SAS_Mbx*     SAS_Mbx;     // Unnamed mailbox handle.

struct         _SAS_IbMbx;
typedef struct _SAS_IbMbx*   SAS_IbMbx;   // Inbound mailbox handle.   

struct         _SAS_ObMbx;
typedef struct _SAS_ObMbx*   SAS_ObMbx;   // Outbound mailbox handle.   

struct         _SAS_AppMbx;
typedef struct _SAS_AppMbx*  SAS_AppMbx;  // Application mailbox handle.  

typedef uint8_t              SAS_MbxId;   // Mailbox ID.
typedef uint32_t             SAS_Type;    // Application specific frame type.
typedef uint32_t             SAS_Size;    // Length of an outbound header.
typedef uint32_t             SAS_Frame;   // Input handle from inbound mailboxes.
typedef uint32_t             SAS_Message; // Input handle from rundown mailboxes.
typedef void*                SAS_Arg;     // Mailbox bind argument
typedef const char*          SAS_Name;    // Used to identify plugins/named mailboxes.

/** 

These are the possible mailbox delivery options returned by mailbox
handlers.

*/

typedef enum {
  SAS_REENABLE               = 0,  
  /*!< Reenable processing input for this mailbox */
  SAS_DISABLE                = (1<<30),  
  /*!< Disable processing input for this mailbox */
  SAS_ABORT                  = (1<<31),  
  /*!< Abort processing input for all mailboxes */
} SAS_Option;

/* mailbox input handler callback signatures */

typedef uint32_t (*SAS_Handler)   (SAS_Message, void *ctx, SAS_Mbx);  
typedef uint32_t (*SAS_ObHandler) (SAS_Message, void *ctx, SAS_ObMbx);  
typedef uint32_t (*SAS_IbHandler) (SAS_Frame,   void *ctx, SAS_IbMbx);  
typedef uint32_t (*SAS_AppHandler)(uint32_t,    void *ctx, SAS_AppMbx);  

/** 

These are the possible status values for SAS functions which return SAS_Status.

*/

typedef enum {
  SAS_SUCCESS  = STS_K_SUCCESS,        /*!< Zero is always success */                       
  SAS_NO_OCM   = STS_ENCODE(SAS, 1),   /*!< OCM could not be mapped */                      
  SAS_NO_AXI   = STS_ENCODE(SAS, 2),   /*!< AXI could not be mapped */                      
  SAS_NO_GIC   = STS_ENCODE(SAS, 3),   /*!< GIC could not be mapped */                      
  SAS_NO_KVT   = STS_ENCODE(SAS, 4),   /*!< A kvt could not be created */                   
  SAS_NO_SVT   = STS_ENCODE(SAS, 5),   /*!< An svt symbol could not be found */             
  SAS_NO_IRQ   = STS_ENCODE(SAS, 6),   /*!< An irq handler failed registration */           
  SAS_ERR_KVT  = STS_ENCODE(SAS, 7),   /*!< Error in kvt transaction */                     
  SAS_INSF_OCM = STS_ENCODE(SAS, 8),   /*!< Insufficient OCM available */                   
  SAS_BAD_SLOT = STS_ENCODE(SAS, 9),   /*!< Invalid slot specified */                       
  SAS_BAD_ATTR = STS_ENCODE(SAS, 10),  /*!< Invalid attributes specified */                       
 } SAS_Status;

/** 

These are the possible status values which are returned by certain SAS functions
associated with allocations.

*/

#define SAS_OPEN_ERROR     ((SAS_Session)0)        // Returned by SAS_Open.
#define SAS_ALLOC_ERROR    ((SAS_Frame)0)          // Returned by SAS_ObAlloc.
#define SAS_NO_MID         ((SAS_MbxId)0xffffffff) // Returned by SAS_Id and SAS_ObId.


/** 

These are the opcodes for outbound transfer engine actions.

*/

typedef enum {
  SAS_OB_FREE               = 0,  
  /*!< Ignore descriptor contents and return to free-list. */
  SAS_OB_HEADER             = 1,  
  /*!< Transmit header only, free descriptor at completion. */       
  SAS_OB_PAYLOAD            = 2,  
  /*!< Transmit header and payload, free descriptor at completion. */
  SAS_OB_PAYLOAD_RUNDOWN    = 3,  
  /*!< Transmit header and payload, insert rundown context, 
       free descriptor at completion. */
 } SAS_ObOpcode;

/** 

These are the opcodes for inbound transfer engine actions.

*/

typedef enum {
  SAS_IB_FREE               = 0,  
  /*!< Descriptor specifies neither receive buffer nor rundown. */
  SAS_IB_FLUSH              = 1,  
  /*!< Descriptor specifies neither receive buffer nor rundown. 
       Flush payload of oldest pending frame. */
  SAS_IB_PAYLOAD            = 2,  
  /*!< Descriptor specifies only a receive buffer. 
       Move payload of oldest pending frame to buffer. */
  SAS_IB_RUNDOWN            = 4,  
  /*!< Descriptor specifies only a rundown. 
       On completion, insert context onto rundown channel. */
  SAS_IB_PAYLOAD_RUNDOWN    = 6,   
  /*!< Descriptor specifies both receive buffer and rundown. 
       Move payload of oldest pending frame to buffer. 
       On completion, insert context onto rundown channel. */
  SAS_IB_KEEP               = 8,   
  /*!< This is the SAS_IbOpcode modifier that can be or'd in with 
       an inbound opcode. The resulting behavior with this flag 
       is that the inbound engine will not automatically free the 
       SAS_Fd upon completion of an inbound transaction. */
 } SAS_IbOpcode;

/** 

This is the frame descriptor structure.

*/

typedef struct {
  void*     payload;   /*!< Physical address pointing to unspecified payload data */
  uint32_t  size;      /*!< Length of payload (in bytes) */
  SAS_MbxId mid;       /*!< Mailbox identifier for rundown */
  void*     message;   /*!< Unspecified rundown message */
  uint32_t  header[];  /*!< Unspecified header data */
  } SAS_Fd;

/** 

This is the socket errors structure.

*/

typedef struct {
  uint32_t obHdr; /*!< Count of outbound header errors  */
  uint32_t obPay; /*!< Count of outbound payload errors */
  uint32_t ibHdr; /*!< Count of inbound header errors   */
  uint32_t ibPay; /*!< Count of inbound payload errors  */
  } SAS_Errors;

/** 

This is the socket metrics structure.

*/

typedef struct {
  uint32_t ibFull;   /*!< Inbound work fifo full count */
  uint32_t obEmpty;  /*!< Outbound free-list fifo empty count */
  uint32_t obFull;   /*!< Outbound free-list fifo full count  */
  } SAS_Metrics;  
  
/** 

This is the socket faults structure.

*/

typedef struct {
  uint32_t ibRead;   /*!< Inbound read errors   */
  uint32_t ibWrite;  /*!< Inbound write errors  */
  uint32_t ibFrame;  /*!< Inbound frame errors  */
  uint32_t ibOvflow; /*!< Inbound overflows     */
  uint32_t obRead;   /*!< Outbound read errors  */
  uint32_t obWrite;  /*!< Outbound write errors */
  uint32_t obFrame;  /*!< Outbound frame errors */
  uint32_t obOvflow; /*!< Outbound overflows    */
  } SAS_Faults;
  
#if defined(__cplusplus)
extern "C" {
#endif

/**  

This function creates a session to associate with one thread.
  
*/

SAS_Session SAS_Open(void);


/**  

This function deletes a previously created session.
  
*/

void SAS_Close(SAS_Session);

/**  

This function waits on session input.
  
*/

void SAS_Wait(SAS_Session);


/**  

This function converts a virtual address to physical.
  
*/

void* SAS_VirtToPhy(void* addr);

/**  

This function allocates and binds a rundown mailbox to a session.
  
*/

SAS_Mbx SAS_Bind(SAS_Handler, void*, SAS_Session);

/**  

This function binds the inbound socket to a session.
  
*/

SAS_IbMbx SAS_IbBind(SAS_Name, SAS_IbHandler, void*, SAS_Session);

/**  

This function allocates and binds an outbound mailbox to a session.
  
*/

SAS_ObMbx SAS_ObBind(SAS_Name, SAS_ObHandler, void*, SAS_Session);

/**  

This function binds an application mailbox to a session.
  
*/

SAS_AppMbx SAS_AppBind(SAS_Name, SAS_AppHandler, void*, SAS_Session);

/**  

This function enables a mailbox input source.
    
*/

void SAS_Enable(SAS_Mbx);

/**  

This function returns the enable status of a mailbox input source.
    
*/

int SAS_IsEnabled(SAS_Mbx);

/**  

This function enables a mailbox input source.
    
*/

void SAS_IbEnable(SAS_IbMbx);

/**  

This function returns the enable status of a mailbox input source.
    
*/

int SAS_IbIsEnabled(SAS_IbMbx);

/**  

This function enables a mailbox input source.
    
*/

void SAS_ObEnable(SAS_ObMbx);

/**  

This function returns the enable status of a mailbox input source.
    
*/

int SAS_ObIsEnabled(SAS_ObMbx);

/**  

This function enables a mailbox input source.
    
*/

void SAS_AppEnable(SAS_AppMbx);

/**  

This function returns the enable status of a mailbox input source.
    
*/

int SAS_AppIsEnabled(SAS_AppMbx);


/**  

This function tests whether or not an input service function
returned input error.
    
*/

uint32_t SAS_Error(SAS_Message);

/**  

This function returns the ID associated with a mailbox.
  
*/

SAS_MbxId SAS_Id(SAS_Mbx);


/**  

This function returns the ID associated with a mailbox.
  
*/

SAS_MbxId SAS_ObId(SAS_ObMbx);

/**  

This function sets a frame type and size.
  
*/

SAS_Frame SAS_ObSet(SAS_Frame, SAS_Type, SAS_Size);

/**  

This function returns the type associated with a frame
  
*/

uint32_t SAS_TypeOf(const SAS_Frame);

/**  

This function returns the size associated with a frame
  
*/

uint32_t SAS_SizeOf(const SAS_Frame);

/**  

This function returns the frame descriptor implicit in a frame handle.
  
*/

SAS_Fd* SAS_ObFd(const SAS_Frame, SAS_ObMbx);

/**  

This function tests whether or not an input service function
returned input error.
  
*/

uint32_t SAS_ObError(const SAS_Message);

/**  

This function de-allocates an outbound frame descriptor.
  
*/

void SAS_ObFree(SAS_Frame, SAS_ObMbx);

/**  

This function allocates an outbound frame descriptor.
  
*/

SAS_Frame SAS_ObAlloc(SAS_ObMbx);

/**  

This function will encode and schedule an outbound instruction for execution.
  
*/

void SAS_ObPost(SAS_ObOpcode, SAS_Frame, SAS_ObMbx);

/**  

This function returns the frame descriptor implicit in a frame handle.
  
*/

SAS_Fd* SAS_IbFd(const SAS_Frame, SAS_IbMbx);

/**  

This function tests for a frame with an inbound payload.
  
*/

uint32_t SAS_IbPayload(const SAS_Frame);

/**  

This function tests for a frame with an inbound error
  
*/

uint32_t SAS_IbError(const SAS_Frame);

/**  

This function will encode and schedule an inbound instruction for execution.
  
*/

void SAS_IbPost(SAS_IbOpcode, SAS_Frame, SAS_IbMbx);

/**  

This function sets a plug-in state to online.
  
*/

void SAS_On(SAS_Name, SAS_Session);

/**  

This function sets a plug-in state to offline.
  
*/

void SAS_Off(SAS_Name, SAS_Session);

/**  

This function resets a plug-in by disabling it and
then setting it to the offline state.
  
*/

void SAS_Reset(SAS_Name, SAS_Session);

/**  

This function resolves the attributes associated with a socket.
  
*/

const SAS_Attributes* SAS_GetAttributes(SAS_Name, SAS_Session);

/**  

This function resolves the errors associated with a socket.
  
*/

const SAS_Errors* SAS_GetErrors(SAS_Name, SAS_Session);

/**  

This function resolves the metrics associated with a socket.
  
*/

const SAS_Metrics* SAS_GetMetrics(SAS_Name, SAS_Session);

/**  

This function resolves the faults associated with a socket.
  
*/

const SAS_Faults* SAS_GetFaults(SAS_Name, SAS_Session);

/**  

This function returns a pointer to a plug-ins I/O registers.
  
*/

void* SAS_Registers(SAS_Name, SAS_Session);


/**  

This function enables all mailboxes and waits on session input.
  
*/

void SAS_EnableWait(SAS_Session);


#if defined(__cplusplus)
} // extern "C"
#endif

#endif
