/*
**  Package: SAS
**	
**
**  Abstract: SAS Package Test Driver
**      
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	    000 - June 03, 2014
**
**  Revision History:
**	    None.
**
** --
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "sas/Sas.h"
#include "../src/Map.h"
#include "memory/mem.h"
#include "map/Lookup.h"

//#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#define debug_printf printf
#else
#define debug_printf(...)
#endif

//#define TIMING

#ifdef TIMING

#define INIT_CNTR() \
      __asm__ volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(0x5));

#define READ_CNTR(_val) \
      __asm__ volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(_val));
      
#define MARK_START(_val) \
      __asm__ volatile ("isb sy"); \
      __asm__ volatile ("dsb sy"); \
      READ_CNTR(_val);

#define MARK_END READ_CNTR

static uint32_t _st;
static uint32_t _et;

#endif

#define PAYLOAD_SIZE  32*1024*1024

#define VALID           0
#define INVALID         1
#define WAIT            2

#define OBRDWN          0
#define IBFRAME         1
#define IBRDWN          2
#define APP0            3
#define APP1            4

#define MIN_HDR_SIZE    24
#define MAX_HDR_SIZE    256

int OBHDR_SIZE  = 32;
int OBPAY_SIZE  = 1024;
int OBFD_COUNT  = 128;
int IBPAY_SIZE  = 1024;
int IBFD_COUNT  = 128;
int ITERATIONS  = 1024*1024;

Ocm        _ocm;
SAS_MbxId  _ibMid;
uint32_t  *_paybase; 
uint32_t  *_ibpay;   
uint32_t  *_obpay;   
uint32_t  *_profile;
uint32_t  _metrics[8];

#ifndef __linux__
//#define APP_MBX
#define APP_MBX_ENTRIES   32
#define APP_FIFO_BASE     0x50080108
#define APP0_SLOT         0
#define APP0_OFFSET       0x10080108
#define APP1_SLOT         1
#define APP1_OFFSET       0x1008010C
const char APP0_NAME[] = "app0";
const char APP1_NAME[] = "app1";
#endif

int testPpi(SAS_Session ses, const char *name, const char *fw);
void submit(SAS_ObMbx);
void printMetrics(const char* name, SAS_Session);

const char USAGE[] =
  "\n  Socket Abstraction Services Test:\n\n\
  Usage: SasTest.exe <option> <ppi_name> [args]\n\
    option 0: start protocol plugin loopback test\n\
    option 1: print protocol plugin loopback metrics\n";
          
const char USAGE_OP0[] =
 "  Socket Abstraction Services Test:\n\
    Start protocol plugin loopback test\n\n\
  Usage: SasTest.exe 0 <ppi_name> <fw_bitfile> [loops] [obsize] [ibsize] [hdrsize]>\n\
    ppi_name: ppi name string\n\
  fw_bitfile: ppi firmware filename\n\
       loops: loopback iterations to execute\n\
      obsize: outbound payload size in bytes\n\
      ibsize: inbound payload size in bytes\n\
     hdrsize: header size in bytes (max 256)\n\n";

const char USAGE_OP1[] =
 "  Socket Abstraction Services Test:\n\
    Print protocol plugin loopback metrics\n\n\
  Usage: SasTest.exe 1 <ppi_name>\n\
    ppi_name: ppi plugin name string\n";

/*
** ++
**
**
** --
*/

void usage(int opt)
  {
    if(opt == 0)
      printf(USAGE_OP0);
    else if(opt == 1)
      printf(USAGE_OP1);
    else
      printf(USAGE);
  }

/*
** ++
**
**
** --
*/

int validateOpts(int opt, int argc, char **argv)
  {
  if(!(argc > 3))
    {
    printf("Specify a ppi fw bitfile\n");
    usage(opt);
    return -1;
    }
  if(argc > 4)
    ITERATIONS = atoi(argv[4]);
  if(argc > 5)
    OBPAY_SIZE = atoi(argv[5]);
  if(argc > 6)
    IBPAY_SIZE = atoi(argv[6]);
  if((IBPAY_SIZE+OBPAY_SIZE) > PAYLOAD_SIZE)
    {
    printf("Payload sizes ob:%d ib:%d exceed buffer capacity of %d\n",
           IBPAY_SIZE,OBPAY_SIZE,PAYLOAD_SIZE);
    return -1;
    }
  if(argc > 7)
    {
    OBHDR_SIZE = atoi(argv[7]);
    if(OBHDR_SIZE > MAX_HDR_SIZE)
      {
      printf("Invalid header size %d, using %d\n",OBHDR_SIZE,MAX_HDR_SIZE);
      OBHDR_SIZE = MAX_HDR_SIZE;
      }
    if( (OBPAY_SIZE && (OBHDR_SIZE < MIN_HDR_SIZE)) || \
        (!OBPAY_SIZE && (OBHDR_SIZE < MIN_HDR_SIZE)) )
      {
      printf("Invalid header size %d, using %d\n",OBHDR_SIZE,MIN_HDR_SIZE);
      OBHDR_SIZE = MIN_HDR_SIZE;
      }
    }
  return 0;
  }
    
/*
** ++
**
**
** --
*/

void printMetrics(const char* name, SAS_Session s)
  {
  /* check socket errors */
  const SAS_Errors  *errors;
  const SAS_Metrics *metrics;
  const SAS_Faults  *faults;
    
  errors = SAS_GetErrors(name,s);
  if(errors)
    {
    printf("\%s loopback metrics:\n",name);
    printf("  ob rdwn  %d\n",(int)_metrics[OBRDWN]);
    printf("  ib data  %d\n",(int)_metrics[IBFRAME]);
    printf("  ib rdwn  %d\n",(int)_metrics[IBRDWN]);
    printf("  app0     %d\n",(int)_metrics[APP0]);
    printf("  app1     %d\n",(int)_metrics[APP1]);
    printf("\%s input wait profiling:\n",name);
    printf("  valid    %d\n",(int)_profile[VALID]);
    printf("  invalid  %d\n",(int)_profile[INVALID]);
    printf("  waits    %d\n",(int)_profile[WAIT]);
    printf("%s socket errors:\n",name);
    printf("  obHdr    %d\n",(int)errors->obHdr);
    printf("  obPay    %d\n",(int)errors->obPay);
    printf("  ibFrame  %d\n",(int)errors->ibHdr);
    printf("  ibPay    %d\n",(int)errors->ibPay);
    }
  else
    printf("Failure getting %s socket errors\n",name);

  metrics = SAS_GetMetrics(name,s);
  if(metrics)
    {
    printf("%s socket metrics:\n",name);
    printf("  ibFull   %d\n",(int)metrics->ibFull);   
    printf("  obEmpty  %d\n",(int)metrics->obEmpty);  
    printf("  obFull   %d\n",(int)metrics->obFull);   
    }
  else
    printf("Failure getting %s socket metrics\n",name);
    
  faults = SAS_GetFaults(name,s);
  if(metrics)
    {
    printf("%s socket faults:\n",name);
    printf("  obRead   %d\n",(int)faults->obRead);
    printf("  obWrite  %d\n",(int)faults->obWrite);
    printf("  obFrame  %d\n",(int)faults->obFrame);
    printf("  obOvflow %d\n",(int)faults->obOvflow);
    printf("  ibRead   %d\n",(int)faults->ibRead);
    printf("  ibWrite  %d\n",(int)faults->ibWrite);
    printf("  ibFrame  %d\n",(int)faults->ibFrame);
    printf("  ibOvflow %d\n",(int)faults->ibOvflow);
    }
  else
    printf("Failure getting %s socket faults\n",name);
  }
  
/*
** ++
**
**
** --
*/

int main(int argc, char **argv)
  {
  SAS_Session ses;  
  int opt = -1;

  if(argc == 1)
    {
    usage(opt);
    return -1;
    }
     
  if(argc > 1)
    opt = atoi(argv[1]);
  
  if(argc == 2)
    {
    printf("Specify a ppi name string\n");
    usage(opt);
    return -1;
    }

  /* map the ocm space */
  Ocm _ocm = LookupOcm();
  if(!_ocm)
    {
    printf("Error mapping OCM\n");
    return -1;
    }

  /* open a channel for the PPI */
  ses = SAS_Open();
  if(!ses)
    {
    printf("Error opening session\n");
    return -1;    
    }

  /* metrics counters */
  ses->profile = 1;
  _profile = (uint32_t*)&ses->valid;

  if(opt == 0) 
    {
    opt = validateOpts(opt,argc,argv);
    if(opt != 0) goto rundown;
            
    printf("\nexecute %d %s loopbacks\n",(int)ITERATIONS,argv[2]);
    printf("  obpay size  %d\n",(int)OBPAY_SIZE);
    printf("  ibpay size  %d\n",(int)IBPAY_SIZE);
    printf("  header size %d\n",(int)OBHDR_SIZE);
    testPpi(ses,argv[2],argv[3]);
    }
  
  rundown:
      
  printMetrics(argv[2],ses);
  
  printf("closing %s session\n",argv[2]);
  SAS_Close(ses);
        
  TeardownOcm(_ocm);
    
  return 0;
  }

#ifdef APP_MBX

/*
** ++
**
**
** --
*/

uint32_t app0MbxCb(uint32_t input, void* c, SAS_ObMbx m)
  {
  debug_printf("Got application 0 message 0x%x\n",(int)input);
  if(++_metrics[APP0] >= APP_MBX_ENTRIES)
    return SAS_DISABLE;
  else
    return SAS_REENABLE;
  }

/*
** ++
**
**
** --
*/

uint32_t app1MbxCb(uint32_t input, void* c, SAS_AppMbx m)
  {
  debug_printf("Got application 1 message 0x%x\n",(int)input);
  if(++_metrics[APP1] >= APP_MBX_ENTRIES)
    return SAS_DISABLE;
  else
    return SAS_REENABLE;
  }

#endif

/*
** ++
**
**
** --
*/

uint32_t obRundwnMbxCb(SAS_Message input, void* c, SAS_ObMbx m)
  {
  uint32_t ret = SAS_REENABLE;
  
  debug_printf("Got outbound rundown %d message\n",(int)SAS_ObId(m));
  debug_printf("  ob rundown error: 0x%x\n",SAS_ObError(input));
  debug_printf("  ob rundown input: 0x%x\n",(int)input);
  debug_printf("  ob rundown value: 0x%x\n",(int)input>>2);
  _metrics[OBRDWN]++;
  if(SAS_ObError(input))
    {
    printf("Error in obRundwnMbxCb %d input 0x%x\n",
           (int)SAS_ObId(m),(int)input);
    printf("  iter %d ibpay 0x%x obpay 0x%x\n",
           (int)_metrics[OBRDWN],(int)_ibpay,(int)_obpay);
    ret = SAS_ABORT;
    }
  if(_metrics[OBRDWN] >= ITERATIONS)
    {
    debug_printf("obRundwnMbxCb %d DISABLE at %d fd 0x%x\n",
           (int)SAS_ObId(m),(int)_metrics[OBRDWN],(int)input);
    ret = SAS_DISABLE;
    }
  else
    submit(m);
  
  return ret;
  }

/*
** ++
**
**
** --
*/

uint32_t ibRundwnMbxCb(SAS_Message input, void* c, SAS_Mbx m)
  {
  uint32_t ret = SAS_REENABLE;
  
  debug_printf("Got inbound rundown message\n");
  debug_printf("  ib rundown error: 0x%x\n",SAS_Error(input));
  debug_printf("  ib rundown input: 0x%x\n",(int)input);

  _metrics[IBRDWN]++;    

  if(SAS_Error(input))
    {
    printf("Error in ibRundwnMbxCb %d input 0x%x at %d\n",
           (int)SAS_Id(m),(int)input,(int)_metrics[IBRDWN]);
    ret = SAS_ABORT;
    }
  else if((input>>2) != _metrics[IBRDWN])
    {
    printf("ibRundwnMbxCb %d invalid input %d at %d\n",
           (int)SAS_Id(m),(int)(input>>2),(int)_metrics[IBRDWN]);
    ret = SAS_ABORT;
    }
  else if(_metrics[IBRDWN] >= ITERATIONS)
    {
    debug_printf("ibRundwnMbxCb %d DISABLE at %d input %d\n",
           (int)SAS_Id(m),(int)_metrics[IBRDWN],(int)(input>>2));
    ret = SAS_DISABLE;
    }
  return ret;
  }

/*
** ++
**
**
** --
*/

uint32_t ibFrameMbxCb(SAS_Frame frame, void* c, SAS_IbMbx m)
  {
  uint32_t *fdp;

  _metrics[IBFRAME]++;    
  
  SAS_Fd *ibfd = SAS_IbFd(frame,m);
  debug_printf("Got inbound header message 0x%x\n",(int)frame);
  
  debug_printf("  IbFd       0x%x\n",(int)ibfd);
  debug_printf("  IbError    0x%x\n",(int)SAS_IbError(frame));
  debug_printf("  IbPayload  0x%x\n",(int)SAS_IbPayload(frame));
  debug_printf("  IbType     0x%x\n",(int)SAS_TypeOf(frame));
  fdp = (uint32_t*)&ibfd->header[0];
  debug_printf("ibhdr 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
               fdp[0],fdp[1],fdp[2],fdp[3],fdp[4],fdp[5]);

  if(SAS_IbError(frame))
    {
    printf("Inbound header error in frame 0x%x\n",(int)frame);
    return SAS_ABORT;
    }
    
  if(!SAS_IbPayload(frame))
    {
    printf("No inbound payload specified\n");
    int ret = SAS_REENABLE;
    if(_metrics[IBFRAME] >= ITERATIONS)
      {
      debug_printf("ibFrameMbxCb !pay DISABLE at %d fd 0x%x\n",
             (int)_metrics[IBFRAME],(int)ibfd);
      ret = SAS_DISABLE;      
      }
    else
      {
      debug_printf("Submit obMbx 0x%x\n",(int)fdp[3]);
      submit((SAS_ObMbx)fdp[3]);
      }
    SAS_IbPost(SAS_IB_FREE, frame, m);
    return ret;
    }
  
  ibfd->payload = (void*)fdp[0];
  ibfd->size = fdp[1];
  ibfd->mid = fdp[2];
  ibfd->message = (void*)(_metrics[IBFRAME]<<2);

  if(ibfd->mid != _ibMid)
    {
    printf("Invalid ibMid in ibFrameMbxCb %d 0x%x\n",
           (int)ibfd->mid,(int)frame);
    SAS_IbPost(SAS_IB_FREE, frame, m);
    return SAS_ABORT;
    }

  if(!ibfd->size)
    {
    printf("Invalid ibfd size in ibFrameMbxCb 0x%x\n",(int)ibfd->size);
    SAS_IbPost(SAS_IB_FREE, frame, m);
    return SAS_ABORT;
    }
    
  debug_printf("Schedule ib fd 0x%x size %d rundown to %d\n",
               (int)ibfd,(int)fdp[1],(int)ibfd->mid);

  SAS_IbPost(SAS_IB_PAYLOAD_RUNDOWN, frame, m);

  if(_metrics[IBFRAME] >= ITERATIONS)
    {
    debug_printf("ibFrameMbxCb DISABLE at %d fd 0x%x\n",
           (int)_metrics[IBFRAME],(int)ibfd);
    return SAS_DISABLE;      
    }
    
  return SAS_REENABLE;
  }

/*
** ++
**
**
** --
*/
  
int testPpi(SAS_Session ses, const char *name, const char *fw)
  {
  SAS_Attributes attrs;
  SAS_Preferences prefs;
  SAS_Mbx ibMbx;       // inbound completion
  SAS_IbMbx frameMbx;  // inbound header/frame
  SAS_ObMbx obMbx;     // outbound completion
#ifdef APP_MBX
  SAS_AppPreferences app0Attrs; // application 0 attributes
  SAS_AppPreferences app1Attrs; // application 1 attributes
  SAS_AppMbx app0Mbx;  // application messages
  SAS_AppMbx app1Mbx;  // application messages
#endif

#if defined TIMING
  INIT_CNTR();
#endif  
  
#ifdef __linux__
  _paybase = malloc(PAYLOAD_SIZE);
#else  
  uint32_t psize;
  _paybase = (uint32_t*)mem_Region_alloc(MEM_REGION_UNCACHED, PAYLOAD_SIZE);
#endif  
  if(!_paybase)
    {
    printf("Failure to allocate obpay from uncached memory region\n");
    return -1;
    }
  printf("payload base 0x%x allocated %d bytes\n",(int)_paybase,PAYLOAD_SIZE);
 
  memset((void*)_paybase,0,PAYLOAD_SIZE);
 
  _paybase[(IBPAY_SIZE>>2)-1] = 0xdeaddead;
  _paybase[(IBPAY_SIZE>>2)-2] = 0xbeefbeef;
  _paybase[(IBPAY_SIZE>>2)-3] = 0xbeefdead;
  _paybase[(IBPAY_SIZE>>2)-4] = 0xdeadbeef;
  _paybase[(IBPAY_SIZE>>2)-5] = 0xcafecafe;
  _paybase[(IBPAY_SIZE>>2)-6] = 0xabadabad;
  _paybase[(IBPAY_SIZE>>2)-7] = 0xcafebad;
  _paybase[(IBPAY_SIZE>>2)-8] = 0xabadcafe;

  /* metrics counters */
  memset((void*)_metrics,0,sizeof(uint32_t)*8);
  
  attrs.moh     = OBHDR_SIZE;
  attrs.obFd    = OBFD_COUNT;
  attrs.mib     = OBHDR_SIZE;
  attrs.ibFd    = IBFD_COUNT;
  
  prefs.attrs   = &attrs;
  prefs.name    = name;
  prefs.bitfile = fw;
  prefs.region  = MEM_REGION_SOCKET;
  prefs.offset  = SOCKET_AXI_BASE+(atoi(prefs.bitfile)*SOCKET_AXI_SIZE);
  
  /* connect the plugin */
  int ret = SAS_Plugin(&prefs);
  if(!ret) printf("%s plugin success!\n",name);
  else printf("%s plugin error: 0x%x\n",name,(int)ret);

#ifdef APP_MBX
  app0Attrs.slot   = APP0_SLOT;
  app0Attrs.offset = APP0_OFFSET;
  app0Attrs.name   = APP0_NAME;
  /* register user mailbox */
  ret = SAS_AppRegister(&app0Attrs);
  if(!ret) printf("%s register 0x%x success!\n",APP0_NAME,APP0_SLOT);
  else printf("%s register error: 0x%x\n",APP0_NAME,(int)ret);

  app1Attrs.slot   = APP1_SLOT;
  app1Attrs.offset = APP1_OFFSET;
  app1Attrs.name   = APP1_NAME;
  /* register app mailbox */
  ret = SAS_AppRegister(&app1Attrs);
  if(!ret) printf("%s register 0x%x success!\n",APP1_NAME,APP1_SLOT);
  else printf("%s register error: 0x%x\n",APP1_NAME,(int)ret);
  
  int i;
  for(i=0;i<APP_MBX_ENTRIES;i++)
    {
    debug_printf("Write app0 0x%x value 0x%x\n",(int)APP_FIFO_BASE,(i+1)<<2);
    *(volatile uint32_t*)(APP_FIFO_BASE) = (i+1)<<2;
    }

  for(i=0;i<APP_MBX_ENTRIES;i++)
    {
    debug_printf("Write app1 0x%x value 0x%x\n",(int)APP_FIFO_BASE+4,(i+1)<<4);
    *(volatile uint32_t*)(APP_FIFO_BASE+4) = (i+1)<<4;
    }
#endif
      
  /* bring the socket disabled->offline */
  debug_printf("Set %s socket offline\n",name);
  SAS_Off(name,ses);

  printf("%s registers 0x%x\n",name,(int)SAS_Registers(name,ses));
  /* allocate mailboxes */
  obMbx = SAS_ObBind(name,(SAS_ObHandler)obRundwnMbxCb,0,ses);
  if(!obMbx)
    {
    printf("Error allocating outbound mailbox\n");
    SAS_Close(ses);
    return -1;
    }
  SAS_ObEnable(obMbx);
  frameMbx = SAS_IbBind(name,(SAS_IbHandler)ibFrameMbxCb,0,ses);
  if(!frameMbx)
    {
    printf("Error allocating header mailbox\n");
    SAS_Close(ses);
    return -1;
    }
  SAS_IbEnable(frameMbx);
  ibMbx = SAS_Bind((SAS_Handler)ibRundwnMbxCb,0,ses);
  if(!ibMbx)
    {
    printf("Error allocating inbound mailbox\n");
    SAS_Close(ses);
    return -1;
    }
  SAS_Enable(ibMbx);
#ifdef APP_MBX
  app0Mbx = SAS_AppBind(APP0_NAME,(SAS_AppHandler)app0MbxCb,0,ses);
  if(!app0Mbx)
    {
    printf("Error allocating application 0 mailbox\n");
    SAS_Close(ses);
    return -1;
    }
  SAS_AppEnable(app0Mbx);
  app1Mbx = SAS_AppBind(APP1_NAME,(SAS_AppHandler)app1MbxCb,0,ses);
  if(!app1Mbx)
    {
    printf("Error allocating application 1 mailbox\n");
    SAS_Close(ses);
    return -1;
    }
  SAS_AppEnable(app1Mbx);    
#endif    
  
  /* check enable status of mailboxes */
  debug_printf("obMbx    enable %d\n",SAS_ObIsEnabled(obMbx));
  debug_printf("frameMbx enable %d\n",SAS_IbIsEnabled(frameMbx));
  debug_printf("ibMbx    enable %d\n",SAS_IsEnabled(ibMbx));
  
  /* bring the socket offline->online */
  SAS_On(name,ses);
   
#ifdef DEBUG    
  {  
  const SAS_Attributes *attrs = SAS_GetAttributes(name,ses);
  /* dump the socket attributes */
  debug_printf("Socket %s attributes:\n",name);
  debug_printf("  moh  %d\n",(int)attrs->moh);
  debug_printf("  obFd %d\n",(int)attrs->obFd);
  debug_printf("  mib  %d\n",(int)attrs->mib);
  debug_printf("  ibFd %d\n",(int)attrs->ibFd);
  }
#endif  
          
  _ibMid = SAS_Id(ibMbx);
  
  printf("start %s loopback\n",name);

#if defined TIMING
  MARK_START(_st);
#endif
    
  submit(obMbx);
  
  SAS_Wait(ses);

#if defined TIMING
  MARK_END(_et);
#endif
  
  debug_printf("Wait returned\n");
  printf("loopback %s complete\n",name);
 
#if defined TIMING
  printf("loopback cycles %d\n",_et-_st);
#endif
 
  /* free the allocated payload memory */
#ifdef __linux__
  free(_paybase);
#else
  mem_Region_undoAlloc(MEM_REGION_UNCACHED,(char*)_paybase);
#endif  

  printf("payload base 0x%x freed\n",(int)_paybase);
  
  SAS_Off(name,ses);

  /* check enable status of mailboxes */
  debug_printf("obMbx    enable %d\n",SAS_ObIsEnabled(obMbx));
  debug_printf("frameMbx enable %d\n",SAS_IbIsEnabled(frameMbx));
  debug_printf("ibMbx    enable %d\n",SAS_IsEnabled(ibMbx));
  
  return 0;
  }

/*
** ++
**
**
** --
*/

void submit(SAS_ObMbx obMbx)
  {
  uint32_t *fdp;
  SAS_Type type = 0x3;
  SAS_Size size = OBHDR_SIZE>>3;
  
  /* circular payload buffer */
  if(!_ibpay || (((uint32_t)_ibpay+IBPAY_SIZE+OBPAY_SIZE) >= (uint32_t)_paybase+PAYLOAD_SIZE))
    _ibpay = _paybase;
  else
    _ibpay = (uint32_t*)((uint32_t)_ibpay+IBPAY_SIZE+OBPAY_SIZE);
    
  _obpay = (uint32_t*)((uint32_t)_ibpay+OBPAY_SIZE);
  
  debug_printf("Init ibpay 0x%x obpay 0x%x\n",(int)_ibpay,(int)_obpay);
        
  _obpay[0] = 0xdead1234;
  _obpay[1] = 0xbeef5678;
  _obpay[2] = 0xabad9876;
  _obpay[3] = 0xcafe3210;
  
  _obpay[(OBPAY_SIZE>>2)-4] = 0x33445566;
  _obpay[(OBPAY_SIZE>>2)-3] = 0x778899AA;  
  _obpay[(OBPAY_SIZE>>2)-2] = 0x11229988;
  _obpay[(OBPAY_SIZE>>2)-1] = 0x98765432;

  SAS_Frame frame = SAS_ObAlloc(obMbx);
  if(!frame)
    {
    printf("Error allocating ob frame\n");
    return;
    }
  
  SAS_Fd *fd = SAS_ObFd(frame,obMbx);
  debug_printf("Allocated fd 0x%x\n",(int)fd);
    
  fd->payload = (void*)_obpay;
  fd->message = (void*)fd;
  fd->size = OBPAY_SIZE;
  fd->mid = SAS_ObId(obMbx);

  if(OBHDR_SIZE)
    {
    fdp = &fd->header[0];

    debug_printf("fd header addr 0x%x ibpay 0x%x obpay 0x%x rundown to %d\n",
                 (int)&fdp[0],(int)_ibpay,(int)_obpay,(int)fd->mid);

    /* 3 quadwords */
    fdp[0] = (uint32_t)_ibpay;
    fdp[1] = IBPAY_SIZE;
    fdp[2] = _ibMid;
    fdp[3] = (uint32_t)obMbx;
    fdp[4] = (uint32_t)_paybase;
    /* header end marker */
    fdp[(OBHDR_SIZE>>2)-1] = 0x29292929;
    }  
  
  frame = SAS_ObSet(frame,type,size);
  if(!OBPAY_SIZE)
    {
    SAS_ObPost(SAS_OB_HEADER, frame, obMbx);
    debug_printf("Post ob hdr fd 0x%x ibMid 0x%x obMbx 0x%x\n",
                 (int)fd,(int)_ibMid,(int)obMbx);
    }
  else
    {
    SAS_ObPost(SAS_OB_PAYLOAD_RUNDOWN, frame, obMbx);
    debug_printf("Post ob pay fd 0x%x ibMid 0x%x obMbx 0x%x\n",
                 (int)fd,(int)_ibMid,(int)obMbx);
    }    
  }

/*
** ++
**
**
** --
*/

void cleanup(void)
  {
  }
