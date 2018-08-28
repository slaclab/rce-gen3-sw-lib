/*
** ++
**  Package: PGP
**	
**
**  Abstract: PGP BDI Emulator Test Driver.
**
**  Implementation of PGP BDI Emulator test driver.
**
**  Author:
**      S. Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	000 - Feb 20, 2017
**
**  Revision History:
**	None.
**
** --
*/

#include <strings.h>
#include <stdint.h>
#include <stdio.h>

#define __need_getopt_newlib
#include <getopt.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/socket.h>

#include <rtems/rtems/sem.h>
#include <rtems/shell.h>

#include "elf/linker.h"
#include "sas/Sas.h"
#include "xaui/Xaui.h"
#include "pgp/Pgp.h"
#include "pgp/Bdi.h"
#include "pgp/BdiEm.h"
#include "BdiEm_Test.h"

#define LOOPBACK_NAME "lo"

#define ETHERNET_TYPE_IPV4 0x800
 
#define VIRT_CHAN        1

#define RESOURCE_COUNT   32

#define RESOURCE_SIZE    3749625 /* default ~3.6 MB page size */

#define FRAME_ENCODE(n,v,x) \
    (x |= ((v & n##_MASK)  << n##_OFFSET))

#define FRAME_TYPE_OFFSET 26

#define FRAME_TYPE_MASK   0x0000000F

#define SIZEOF_PGP_FRAME   2040  /* PGP firmware max frame length in bytes */

//#define BDIEM_TIMING
#ifdef BDIEM_TIMING
#define GTC_REGISTER  0xF8F00200
#define READ_GTC(_val) _val = *gtc
#endif

/*
**
*/

uint32_t _Sem[PGP_NUMOF_LINKS];

uint32_t _Links = 0;

uint32_t _client_ip;

Xaui_Protocol _save;

int _srv_addr = -1;

int _vc = -1;

int _sock_udp = 0;

struct sockaddr_in _si_udp;

uint64_t _forwarded;

uint64_t _intercepted;

uint64_t _badport;

char _filename0[BDIEM_FILENAME_STRLEN];
char _filename1[BDIEM_FILENAME_STRLEN];
char _filename2[BDIEM_FILENAME_STRLEN];
char _filename3[BDIEM_FILENAME_STRLEN];
char _filename4[BDIEM_FILENAME_STRLEN];
char _filename5[BDIEM_FILENAME_STRLEN];
char _filename6[BDIEM_FILENAME_STRLEN];
char _filename7[BDIEM_FILENAME_STRLEN];
char _filename8[BDIEM_FILENAME_STRLEN];
char _filename9[BDIEM_FILENAME_STRLEN];
char _filename10[BDIEM_FILENAME_STRLEN];
char _filename11[BDIEM_FILENAME_STRLEN];

char *_filename[PGP_NUMOF_LINKS] = 
  {
  _filename0,_filename1,_filename2,_filename3,
  _filename4,_filename5,_filename6,_filename7,
  _filename8,_filename9,_filename10,_filename11
  };

#ifdef BDIEM_TIMING
static volatile uint32_t *gtc = (uint32_t*)GTC_REGISTER;
uint64_t _page_cnt = 0;
uint64_t _page_elp = 0;
uint32_t _page_st  = 0;
uint32_t _page_et  = 0;
uint32_t _page_bytes = 0;
#endif

/*
** ++
**
** BDIEM test command usage
** 
** --
*/

const char USAGE_BDIEM_TEST[] =
  "\n  BDI Emulator Test Driver\n\
  Usage: bdiEm_test [OPTIONS] [args]\n\
    -i <ip_addr>          Set file server ip\n\
    -v <vc>               Set device virtual channel\n\
    -o <link>             Open device link\n\
    -d <link>             Dump device link statistics\n\
    -r <link>             Reset device link statistics\n\
    -f <filename> <link>  Set device link filename\n\
    -n <link>             Request network file load from server\n\
    -s <link>             Load from local filesystem\n\
    -t <link>             Trigger file transmit\n";
    
static void _usage(void)
  {
  
  optind = 0;
  printf("%s",USAGE_BDIEM_TEST);
  }

#ifdef BDIEM_TIMING
/*
** ++
**
**
** --
*/

static const char line4[] = "BDIEM test input timing\n";
static const char line5[] = "  Frame count           %llu\n";
static const char line6[] = "  Elapsed tics(2.5ns)   %llu\n";
static const char line7[] = "  Average tics/frame    %llu\n";
static const char line8[] = "  Throughput            %.3f MB/s\n";
static const char line9[] = "                        %.3f Mbps\n";

static void _measure(void)
  {
  
  READ_GTC(_page_et);
  if(_page_st)
    {
    if(_page_st>_page_et)
      _page_elp += (0xffffffff-_page_st) + _page_st;
    else
      _page_elp += _page_et-_page_st;
    }    
  _page_st   = _page_et;
  _page_cnt++;  
  }

#endif
  
/*
** ++
**
**
** --
*/

static const char line0[] = "BDIEM test statistics\n";
static const char line1[] = "  Intercepted frames    %llu\n";
static const char line2[] = "  Forwarded frames      %llu\n";
static const char line3[] = "  Error frames          %llu\n";

void BDIEMT_Dump(uint32_t link)
  {

  uint64_t avg = 0;
  double tput = 0;
  
  if(_vc < 0)
    {
    printf("BDIEM_Test: set vc before dumping device\n");
    return _usage();
    }
  
  BDIEM_Dump(_vc,link);  

  printf(line0);  
  printf(line1,_intercepted);
  printf(line2,_forwarded);  
  printf(line3,_badport);

#ifdef BDIEM_TIMING
  if(_page_cnt)
    avg =  _page_elp/_page_cnt;

  if(_page_bytes)
    {
    tput = (_page_bytes/1024)/1024;
    if(tput)
      tput = tput/(0.000000001*(_page_elp*2.5)); //2.5ns per tic
    printf(line4);  
    printf(line5,_page_cnt);
    printf(line6,_page_elp);  
    printf(line7,avg);
    printf(line8,tput);
    printf(line9,tput*8);
    }  
#endif
  }

/*
** ++
**
**
** --
*/

int BDIEMT_Reset(uint32_t link)
  {
  
  if(_vc < 0)
    {
    printf("BDIEM_Test: set vc before resetting device\n");
    _usage();
    return 1;
    }
  
  BDIEM_Reset(_vc,link);

  _forwarded   = 0;
  _intercepted = 0;
  _badport     = 0;

#ifdef BDIEM_TIMING
  _page_cnt    = 0;
  _page_st     = 0;
  _page_et     = 0;
  _page_elp    = 0;  
  _page_bytes  = 0;
#endif
  }

/*
**
**
*/

static void _wait(uint32_t link)
  {
    
  while(1)
    {
    rtems_semaphore_obtain(_Sem[link], RTEMS_WAIT, RTEMS_NO_TIMEOUT);

    /* wait for buffered file transmit completion */
    BDIEM_Post(_vc,link);
    }
  }

/*
**
**
*/

static const char DEV0[] = "0";
static const char DEV1[] = "1";
static const char DEV2[] = "2";
static const char DEV3[] = "3";
static const char DEV4[] = "4";
static const char DEV5[] = "5";

static const char *_devids[] = {DEV0,DEV1,DEV2,DEV3,DEV4,DEV5};

uint32_t _launch(uint32_t link)
  {  
  
  uint32_t tid;        
  rtems_status_code sc;
  
  sc = rtems_task_create(rtems_build_name('B','D','T',*_devids[_Links]),
                         80,
                         RTEMS_MINIMUM_STACK_SIZE * 10,
                         RTEMS_DEFAULT_ATTRIBUTES,
                         RTEMS_DEFAULT_MODES,
                         &tid);
  if (sc != RTEMS_SUCCESSFUL)
    rtems_panic("%s Can't create task: %s", __func__,rtems_status_text(sc));

  /* Create a semaphore to synchronize file transfer trigger */
  sc = rtems_semaphore_create(rtems_build_name('B', 'E', 'M',*_devids[_Links++]), 0,
                              RTEMS_SIMPLE_BINARY_SEMAPHORE , 0,
                              &_Sem[link]);
  if (sc != RTEMS_SUCCESSFUL)
    rtems_panic("%s Can't create semaphore: %s", __func__,rtems_status_text(sc));
    
  sc = rtems_task_start(tid, (rtems_task_entry)_wait, (int)link);
  if (sc != RTEMS_SUCCESSFUL)
    rtems_panic("%s Can't start task: %s", __func__,rtems_status_text(sc));
 
  return 0;
  }

/*
** ++
**
**
** --
*/

static uint32_t _intercept(SAS_Frame frame, void* context, SAS_IbMbx mbx)
  {

  volatile SAS_Fd* fd = SAS_IbFd(frame, mbx);

  UdpPacket *udp = (UdpPacket*)&fd->header[0];

  /* SEMTODO: this is truly evil, substitute our callback for bdiemsrv UDP frames */
  if((ntohs(udp->udp.src) == BDIEM_UDP_PORT) && (udp->ip.src == _srv_addr))
    {
    ++_intercepted;

    /* SEMTODO: this is truly evil, overwrite IP protocol headers with PGP header */
    memcpy(udp,&udp->pgp,sizeof(Pgp_Header));

    /* SEMTODO: this is truly evil, reencode frame type with luser source link */
    FRAME_ENCODE(FRAME_TYPE,udp->pgp.luser,frame);

  #ifdef BDIEM_TIMING  
    _page_bytes += udp->pgp.size;
    _measure();
  #endif
    return BDIEM_IbFrame(frame, 0, mbx);
    }
  else if((!(ntohs(udp->udp.src))) && (udp->ip.src == _srv_addr))
    ++_badport;
  else if(_save.processor)
    {
    ++_forwarded;
    return _save.processor(frame, _save.ctx, mbx);
    }

  return SAS_IbPayload(frame) ? SAS_IB_FLUSH : SAS_IB_FREE;   
  } 

/*
** ++
**
**
** --
*/

static int _override(void)
  {
  
  Xaui_Protocol *prot = Xaui_Lookup(ETHERNET_TYPE_IPV4);
  if(!prot)
    {
    printf("bdiEm_test: xaui prot lookup failed 0x%x\n",ETHERNET_TYPE_IPV4);
    return -1;
    }
    
  _save.processor = prot->processor;
  _save.ctx = prot->ctx;
  
  Xaui_Bind(ETHERNET_TYPE_IPV4,_intercept,0);    
  
  return 0;
  }

/*
** ++
**
**
** --
*/
 
static int _setup(void)
  {
  
  struct ifaddrs *ifAddr, *ifa;
  
  if (getifaddrs(&ifAddr) == -1)
    {
    return -1;
    }
  for (ifa = ifAddr; ifa; ifa = ifa->ifa_next)
    {
    if ( !ifa->ifa_addr )
      continue;
    if (AF_INET == ifa->ifa_addr->sa_family &&
        strncmp(ifa->ifa_name, LOOPBACK_NAME,strlen(LOOPBACK_NAME)))
      {
      _client_ip = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
      break;
      }      
    }    

  return _override();
  }

/*
** ++
**
**
** --
*/

int BDIEMT_Open(uint32_t link)
  {
  
  if(_vc < 0)
    {
    printf("bdiEm_test: set vc before opening device\n");
    _usage();    
    return 1;
    }

  if(_srv_addr < 0)
    {
    printf("bdiEm_test: set server IP before opening device\n");
    _usage();
    return 1;
    }
  
  uint32_t status = BDIEM_Open(link,_vc,RESOURCE_COUNT,RESOURCE_SIZE);
  if(status)
    {
    printf("bdiEm_test: %s error in BDI_Open 0x%x\n",__func__,status);
    return 1;
    }
  
  _launch(link);  
  
  return 0;  
  }

/*
** ++
**
**
** --
*/
     
int BDIEMT_Remote(uint32_t link)
  {
  
  if(link >= PGP_NUMOF_LINKS)
    {
    _usage();
    return 1;
    }
    
  int slen=sizeof(_si_udp);
  
  //printf("%s: request %s from bdiemsrv %s:%d link %d\n", __func__,_filename[link],inet_ntoa(_srv_addr),ntohs(_si_udp.sin_port),link);

  BDIEM_Header hdr;
  bzero(&hdr,sizeof(BDIEM_Header));
  hdr.magic = BDIEM_UDP_MAGIC;
  hdr.link = link;
  hdr.vc   = _vc;

  memcpy(hdr.filename ,_filename[link],(strlen(_filename[link]) > BDIEM_FILENAME_STRLEN ? BDIEM_FILENAME_STRLEN : strlen(_filename[link])));
  
  if (sendto(_sock_udp, &hdr, sizeof(BDIEM_Header), 0, (const struct sockaddr*)&_si_udp, slen)==-1)
    {
    printf("bdiEm_test: error in sendto()\n");
    return 1;
    }
    
  return 0;
  }

/*
** ++
**
**
** --
*/

int BDIEMT_Local(uint32_t link)
  {
  
  uint32_t flags = 0;
  int size,total = 0;
  struct stat st;
  
  char *_buf[SIZEOF_PGP_FRAME];
    
  if(link >= PGP_NUMOF_LINKS)
    {
    _usage();
    return 1;
    }
    
  FILE *fd = fopen(_filename[link],"rb");
  if(!fd)
    {
    printf("%s: file open failure %s\n",__func__,_filename[link]);
    return 1;
    }

  fstat(fileno(fd),&st);

  printf("Open file %s %d bytes link %d vc %d\n",_filename[link],(int)st.st_size,link,_vc);
    
  while((size = fread(_buf,1,SIZEOF_PGP_FRAME,fd)))
    {
    if(!total)
      flags = PGP_SOF;    
    else if((total+size) == st.st_size)
      flags = PGP_EOF;
    else
      flags = 0;      
    
    BDIEM_Process(_vc,link,flags,size,_buf);
    
    total+=size;
    }
  fclose(fd);

  printf("Processed file %s %d bytes\n",_filename[link],(int)st.st_size);
  
  return 0;
  }

/*
** ++
**
**
** --
*/
   
int BDIEMT_Trigger(uint32_t link)
  {
  
  if(link >= PGP_NUMOF_LINKS)
    return 1;
  
  /* send buffered image via BDIEM_Wait */  
  rtems_semaphore_release(_Sem[link]);
    
  return 0;
  }

/*
** ++
**
**
** --
*/
   
int BDIEMT_Vc(uint32_t vc)
  {
  
  _vc = vc;
  
  return 0;
  }

/*
** ++
**
**
** --
*/
   
int BDIEMT_Ip(char *addr)
  {
  
  _srv_addr = inet_addr(addr);

  if ((_sock_udp=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
   {
   fprintf(stderr, "bdiEm_test: file server socket open failed\n");
   return 1;
   }

  memset((char *) &_si_udp, 0, sizeof(_si_udp));
  _si_udp.sin_family = AF_INET;
  _si_udp.sin_port = htons(BDIEM_UDP_PORT);
  if (inet_aton(addr, &_si_udp.sin_addr)==0)
    {
    fprintf(stderr, "bdiEm_test: inet_aton() failed\n");
    return 1;
    }    

  return 0;
  }

/*
** ++
**
**
** --
*/

int BDIEMT_File(char *filename, uint32_t link)
  {
  
  if(link >= PGP_NUMOF_LINKS)
    {
    _usage();
    return 1;
    }
  
  bzero(_filename[link],BDIEM_FILENAME_STRLEN);
  
  memcpy(_filename[link],
         filename,
         (strlen(filename) > BDIEM_FILENAME_STRLEN ? BDIEM_FILENAME_STRLEN : strlen(filename)));
         
  _filename[link][strlen(_filename[link])] = '\0';
      
  return 0;
  }

/*
** ++
**
** This function executes the bdi_test shell command
**
** --
*/
  
void bdiEm_test(int argc, char** argv)
  {
  
  int theOpt;

  if(argc == 1) return _usage();

  struct getopt_data getopt_reent;
  memset(&getopt_reent, 0, sizeof(getopt_data));
  
  while ( (theOpt = getopt_r(argc,argv,
                             "o:n:s:v:i:d:r:f:t:", 
                             &getopt_reent)) != EOF )
    {  
    switch (theOpt)
      {      
      case 'o': BDIEMT_Open(strtoul(getopt_reent.optarg, 0, 0));  break;
      case 'n': BDIEMT_Remote(strtoul(getopt_reent.optarg, 0, 0));  break;
      case 's': BDIEMT_Local(strtoul(getopt_reent.optarg, 0, 0));  break;
      case 'v': BDIEMT_Vc(strtoul(getopt_reent.optarg, 0, 0));    break;
      case 'i': BDIEMT_Ip(getopt_reent.optarg);                   break;
      case 'd': BDIEMT_Dump(strtoul(getopt_reent.optarg, 0, 0));  break;
      case 'r': BDIEMT_Reset(strtoul(getopt_reent.optarg, 0, 0)); break;
      case 'f': if(argc != 4) return _usage();
                BDIEMT_File(getopt_reent.optarg,\
                            strtoul(argv[3], 0, 0));                break;
      case 't': BDIEMT_Trigger(strtoul(getopt_reent.optarg, 0, 0)); break;      
      default:  return _usage();
      }
    }    
  }

/*
** ++
**
** This function registers BDI test commands with the rtems shell.
**
** --
*/

static const char _cmd[]   = "bdiEm_test";
static const char _topic[] = "pgp";

static void _addCommands(void)
  {
  
  rtems_shell_cmd_t * retCmd;
  // Call the long form for adding this command.  This
  // allocates the structure space inside RTEMS

  retCmd = rtems_shell_lookup_cmd(_cmd);
  
  /* cannot add command more than once */
  if(retCmd) return;

  retCmd = rtems_shell_add_cmd(_cmd,
                               _topic,                             
                               USAGE_BDIEM_TEST,                             
                               (rtems_shell_command_t)bdiEm_test);

  rtems_shell_add_cmd_struct( retCmd );
  }

/*
** ++
**
** This function is the .so constructor
**
** --
*/

unsigned lnk_prelude(void* attributes, void* elf)
  {
    
  _addCommands();
  return _setup(); 
  }
  
