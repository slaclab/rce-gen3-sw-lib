#include "cm_svc/PlxBridge.hh"
#include "cm_svc/Print.hh"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

//
//  PLX Registers
//

//  Configuration Type 0 Accesses
enum { PECS_DEVICEID  = 0,
       PECS_PCICMD    = 0x4,
       PECS_PCIBAR0   = 0x10,
       PECS_PCIBUSNUM = 0x18,
       PECS_IOBASE    = 0x1C,
       PECS_MEMBASE   = 0x20,
       PECS_BRIDGECTL = 0x3C,
       PECS_DEVCTL    = 0x68,
       PECS_IRQSTAT   = 0x1018,
       PECS_GPIOCTL   = 0x1020,
       PECS_CRSTIMER  = 0x1060 };

//  Configuration Type 1 Accesses
enum { LCS_DEVICEID   = 0,
       LCS_PCICSR     = 0x04,
       LCS_PCIBAR0    = 0x10,
       LCS_PCIBAR2    = 0x18 };

//  Memory Accesses via LCS_PCIBAR0 Address
enum { LCS_LAS0RR     = 0x00,
       LCS_LAS0BA     = 0x04,
       LCS_MARBR      = 0x08,
       LCS_BIGEND     = 0x0C,
       LCS_LBRD0      = 0x18,
       LCS_IOCNTL     = 0x6C,
       LCS_LMISC      = 0x8C,
       LCS_LBRD1      = 0xF8 };

enum { PCI_BASE = 0xfffe0000,    //  Map PCI registers to memory
       LCS_BASE = 0xffeffe00 };  //  Map LCS registers to memory

enum { MEM_ACCESS = 0,
       CFG_TYPE0  = 0x4,
       CFG_TYPE1  = 0x5 };

enum { SUCCESS = 0,
       CMPLERR = 1,
       TIMEOUT = 2 };


typedef struct
{
  unsigned long v[4];
} uint128_t;

static char alignBase[48];
static uint128_t* _tx_buffer;
static uint128_t* _rx_buffer;
static unsigned _rx_last;

static volatile unsigned* _reg=0;
static unsigned _tag=0;
static unsigned verbose=0;

static void _rderr();
static int _dev = 4;

static unsigned bswap(unsigned v)
{
  unsigned r = ((v>>24)&0x000000ff) |
               ((v>> 8)&0x0000ff00) |
               ((v<< 8)&0x00ff0000) |
               ((v<<24)&0xff000000);
  return r;
}

static void spin(unsigned ms)
{
  struct timespec tv;
  tv.tv_sec = ms/1000;
  tv.tv_nsec = 1000000*(ms%1000);;
  nanosleep(&tv,0);
}

static void setup_align()
{
  _tx_buffer = (uint128_t*)(((uint32_t)&alignBase[0] + 0xf)&~0xf);
  _rx_buffer = _tx_buffer+1;
}

static unsigned rx_empty()
{
  unsigned v=_reg[5];
  _rx_last=(v&0x40000000);
  return (v&0x80000000)==0;
}

static void rx_word()
{
  uint32_t* bp = (uint32_t*)_rx_buffer;
  bp[1] = _reg[4];
  bp[0] = _reg[3];
  if (!_rx_last) {
    rx_empty();
    bp[3] = _reg[4];
    bp[2] = _reg[3];
    if (!_rx_last) {
      cm::service::prints("rx_word reading more than 4 words\n");
    }
  }
}

static int get_response(unsigned ms, int v)
{
  uint32_t* p = (uint32_t*)_tx_buffer;
  uint32_t* q = (uint32_t*)_rx_buffer;

  unsigned tag = p[1]&0xff00;

  struct timespec now, t;
  clock_gettime(CLOCK_REALTIME,&now);

  t.tv_sec  = now.tv_sec + ms/1000;
  t.tv_nsec = now.tv_nsec + (ms%1000)*1000000;
  if (t.tv_nsec > 1000000000) {
    t.tv_sec++;
    t.tv_nsec -= 1000000000;
  }

  while(1) {
    if (!rx_empty()) {

      rx_word();

      if (verbose) {
        if (*q & 0x40000000)
          cm::service::printv("rx %08x:%08x:%08x:%08x\n",q[0],q[1],q[2],q[3]);
        else
          cm::service::printv("rx %08x:%08x:%08x\n",q[0],q[1],q[2]);
      }

      if ((q[2]&0xff00) == tag)
        return (q[1]&0xe000)==0 ? SUCCESS : CMPLERR;
    }

    if (!((t.tv_sec > now.tv_sec) || ((t.tv_sec==now.tv_sec) && (t.tv_nsec > now.tv_nsec))))
      break;

    clock_gettime(CLOCK_REALTIME,&now);
  }

  if (!v) 
    cm::service::printv("TMO: tx %08x:%08x:%08x:%08x\n",p[0],p[1],p[2],p[3]);
  else
    cm::service::prints("TMO\n");
//  return TIMEOUT;
  return CMPLERR;
}

static void _fifo_wr3()
{
  uint32_t* p = (uint32_t*)_tx_buffer;
  if (verbose)
    cm::service::printv("tx: %08x %08x %08x\n",p[0],p[1],p[2]);
  _reg[0] = p[0];
  _reg[1] = p[1];
  _reg[2] = 0xff;
  _reg[0] = p[2];
  _reg[1] = p[3];
  _reg[2] = 0x4000000f;
}

static void _fifo_wr4()
{
  uint32_t* p = (uint32_t*)_tx_buffer;
  if (verbose)
    cm::service::printv("tx: %08x %08x %08x %08x\n",p[0],p[1],p[2],p[3]);
  _reg[0] = p[0];
  _reg[1] = p[1];
  _reg[2] = 0xff;
  _reg[0] = p[2];
  _reg[1] = p[3];
  _reg[2] = 0x400000ff;
}

static void _pcie_rd (unsigned type, unsigned addr)
{
  uint32_t* p = (uint32_t*)_tx_buffer;

  p[0] = ((type&0xff)<<24) | 1;
  p[1] = ((_tag++&0xff)<<8) | 0xf;
  p[2] = addr;
  p[3] = 0;
  _fifo_wr3();
}

static void _pcie_wr(unsigned type, unsigned addr, unsigned data)
{
  uint32_t* p = (uint32_t*)_tx_buffer;

  p[0] = (((type|0x40)&0xff)<<24) | 1;
  p[1] = ((_tag++&0xff)<<8) | 0xf;
  p[2] = addr;
  p[3] = bswap(data);
  _fifo_wr4();
}

static int pcie_rd (unsigned type, unsigned addr)
{
  _pcie_rd(type,addr);
  int result = get_response(20,0);
  if (result == CMPLERR)
    _rderr();

  return result;
}

static void pcie_wr(unsigned type, unsigned addr, unsigned data)
{
  _pcie_wr(type, addr, data);

  if (type!=MEM_ACCESS) { // No completion for posted writes
    int result = get_response(20,0);
    if (result == CMPLERR)
      _rderr();
  }
}

static void pcie_rmw(unsigned type, unsigned addr, unsigned data)
{
  uint32_t* p = (uint32_t*)_tx_buffer;
  p[0] = (((type)&0xff)<<24) | 1;
  p[1] = ((_tag++&0xff)<<8) | 0xf;
  p[2] = addr;
  p[3] = 0;
  _fifo_wr3();

  if (get_response(200,0)==CMPLERR)
    _rderr();

  p[0] = (((type|0x40)&0xff)<<24) | 1;
  p[1] = ((_tag++&0xff)<<8) | 0xf;
  p[2] = addr;
  p[3] = bswap(data);
  _fifo_wr4();

  if (type!=MEM_ACCESS) { // No completion for posted writes
    if (get_response(200,0)==CMPLERR)
      _rderr();
  }
  else
    spin(20);

  p[0] = (((type)&0xff)<<24) | 1;
  p[1] = ((_tag++&0xff)<<8) | 0xf;
  p[2] = addr;
  p[3] = 0;
  _fifo_wr3();

  if (get_response(200,0)==CMPLERR)
    _rderr();
}

static void config_rmw(unsigned addr, unsigned data)
{
  pcie_rmw(CFG_TYPE1, addr, data);
//  pcie_wr(CFG_TYPE1, addr, data);
}

#if 0
static void pecs_rd(unsigned addr)
{
  pcie_rd(CFG_TYPE0, addr);
}
#endif

static void pecs_rmw(unsigned addr, unsigned data)
{
  pcie_rmw(CFG_TYPE0, addr, data);
}

static void memory_rmw(unsigned addr, unsigned data)
{
  pcie_rmw(MEM_ACCESS, addr, data);
}

static void set_devctl()
{
//  pcie_wr(CFG_TYPE0, PECS_DEVCTL, 0xffff200f);
}

static void _rderr()
{
  static unsigned count=0;
  uint32_t* p = (uint32_t*)_rx_buffer; 
  uint32_t* q = (uint32_t*)_tx_buffer;
  ++count;                           

  cm::service::printv("_RDERR[%d]: tx %08x:%08x:%08x:%08x rx %08x:%08x:%08x:%08x tag %04x\n",
         count, q[0],q[1],q[2],q[3], p[0],p[1],p[2],p[3], _tag);
  _pcie_rd(CFG_TYPE0, PECS_DEVCTL);

  if ( get_response(200,0) == SUCCESS ) {
    unsigned stat = bswap(p[3]);                
    cm::service::printv("PECS_DEVCTL: %08x",stat);
    if (stat&0x00010000) cm::service::printv(" Correctable");
    if (stat&0x00020000) cm::service::printv(" NonFatal");  
    if (stat&0x00040000) cm::service::printv(" Fatal");    
    if (stat&0x00080000) cm::service::printv(" UnsupportedReq");
    cm::service::printv("\n");                                 
  }                                             
  else {                                       
    cm::service::printv("PECS_DEVCTL: ERR\n");              
  }                                          

  //_pcie_wr(CFG_TYPE0, PECS_DEVCTL, 0xffff200f);
  set_devctl();

  _pcie_rd(CFG_TYPE0, PECS_PCICMD);          
  if (get_response(200,0) == SUCCESS) {      
    unsigned stat = bswap(p[3]);                    
    cm::service::printv("PECS_PCICMD/STAT: %08x",stat);              
    if (stat&0x01000000) cm::service::printv(" MasterDataParity");
    if (stat&0x08000000) cm::service::printv(" SigTargetAbort"); 
    if (stat&0x10000000) cm::service::printv(" RcvTargetAbort");
    if (stat&0x20000000) cm::service::printv(" RcvMasterAbort");
    if (stat&0x40000000) cm::service::printv(" SysSignalErr"); 
    if (stat&0x80000000) cm::service::printv(" DetParityErr");
    cm::service::printv("\n");                               
    _pcie_wr(CFG_TYPE0, PECS_PCICMD, stat | 0xffff0000);
  }                                           
  else {                                     
    cm::service::printv("PECS_PCISTAT: ERR\n");           
  }                                        

  _pcie_rd(CFG_TYPE0, PECS_IOBASE);                 
  if (get_response(200,0) == SUCCESS) {             
    unsigned stat = bswap(p[3]);                           
    cm::service::printv("PECS_SECSTAT: %08x",stat);                     
    if (stat&0x01000000) cm::service::printv(" MasterDataParity");
    if (stat&0x08000000) cm::service::printv(" SigTargetAbort"); 
    if (stat&0x10000000) cm::service::printv(" RcvTargetAbort");
    if (stat&0x20000000) cm::service::printv(" RcvMasterAbort");
    if (stat&0x40000000) cm::service::printv(" SysSignalErr"); 
    if (stat&0x80000000) cm::service::printv(" DetParityErr");
    cm::service::printv("\n");                               
    _pcie_wr(CFG_TYPE0, PECS_IOBASE, stat | 0xffff0000);
  }                                           
  else {                                     
    cm::service::printv("PECS_SECSTAT: ERR\n");           
  }                                        

  _pcie_rd(CFG_TYPE0, PECS_IRQSTAT);
  if (get_response(200,0) == SUCCESS) {
    unsigned stat = bswap(p[3]);
    cm::service::printv("PECS_IRQSTAT: %08x\n",stat);
//    _pcie_wr(CFG_TYPE0, PECS_IRQSTAT, 0xffffffff);
  }
  else
    cm::service::printv("PECS_IRQSTAT: ERR\n");
}

void setup_memio()
{
  unsigned device = _dev;
  unsigned cfg_addr = device<<19;
  
  //  Local bus reset
  pecs_rmw  ( PECS_BRIDGECTL, 0x08630100 );
  spin(20);
  
  //  Clear local bus reset
  pecs_rmw  ( PECS_BRIDGECTL, 0x00000100 );
//  pecs_rmw  ( PECS_BRIDGECTL, 0x08230100 );
  spin(20);
//  set_devctl();

  pecs_rmw  ( PECS_GPIOCTL, 0 );
  //  Map memory from 0 through configuration space
  unsigned PLCS_BASE = LCS_BASE>>20;
  pecs_rmw  ( cfg_addr | PECS_MEMBASE , (PLCS_BASE<<20) | (0));
  
  //  Memory access to PECS registers
  pecs_rmw( PECS_PCIBAR0, PCI_BASE );
  
  //  Enable memory space mapping on the upstream port
  pecs_rmw( PECS_PCICMD, 0xff000002 );
  
  memory_rmw( PCI_BASE | PECS_PCIBAR0, PCI_BASE );

  config_rmw( cfg_addr | LCS_PCIBAR0, LCS_BASE );
  config_rmw( cfg_addr | LCS_PCICSR, 0x02b10002 );
  pcie_rd( CFG_TYPE1, cfg_addr | LCS_PCICSR);
  
  memory_rmw( LCS_BASE+LCS_BIGEND, 0x00300104 );
  
  memory_rmw( LCS_BASE+LCS_LAS0RR, 0xFF000000 );

// Try disabling prefetch
  memory_rmw( LCS_BASE+LCS_LBRD0, 0xF0430043 );
  
// Enable PCIr2.2 compliance
//  memory_rmw( LCS_BASE+LCS_MARBR,  0x01200000 );
  memory_rmw( LCS_BASE+LCS_MARBR,  0x00200000 );

  config_rmw( cfg_addr | LCS_PCIBAR2, 0 );  // default
  
  memory_rmw( LCS_BASE+LCS_LAS0BA, 1 );
  set_devctl();
}

#define BUS_BASE_ADDRESS 0xBC001000   // EPC
#define MAP_SIZE         4096UL
#define MAP_MASK         (MAP_SIZE - 1)

using namespace cm::svc;

static PlxBridge* _instance=0;

PlxBridge::PlxBridge() 
{
   int memfd;
   char *mapped_dev_base; 
   off_t dev_base = BUS_BASE_ADDRESS; 

   memfd = open("/dev/mem", O_RDWR | O_SYNC);
      if (memfd == -1) {
      cm::service::printv("Can't open /dev/mem.\n");
      exit(0);
   }
   _mapped_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
                      memfd, dev_base & ~MAP_MASK);
   if (_mapped_base == (void *) -1) {
         cm::service::printv("Can't map the memory to user space.\n");
         exit(0);
   }
   mapped_dev_base = (char*)_mapped_base + (dev_base & MAP_MASK);
   //   cm::service::printv("Memory %lx mapped at address %p. Base=%p\n", 
   //          dev_base,_mapped_base,mapped_dev_base);
   // get the address of the device in user space which will be an offset from the base
   // that was mapped as memory is mapped at the start of a page
   // Set the initial pointer
   _reg = (volatile uint32_t *) (mapped_dev_base);

  setup_align();

  sem_init(&_sem,0,1);
}

PlxBridge::~PlxBridge()
{
  sem_destroy(&_sem);

  // unmap the memory before exiting
  if (munmap(_mapped_base, MAP_SIZE) == -1) {
     cm::service::printv("Can't unmap memory from user space.\n");
     exit(0);
  }
}

#if 0
static void manual_reset()
{
  //
  //  First, PCIE reset.
  //  This is a manual process for now
  //
  {
    cm::service::printv("Resetting: touch COB C72 (bottom side of board)\n");
    _reg[17] = 0;
    usleep(20000);
    _reg[17] = 3;
    while( (_reg[16]&0x18)==0 )
      usleep(100000);
    cm::service::printv("Reset complete\n");
  }
}
#endif

static void auto_reset()
{
  //
  //  Bring the endpoint out of reset, then the root
  //
  unsigned rst_time = 20000;
  do {
    cm::service::printv("Resetting PCIE bus [%d usec]\n",rst_time);
    _reg[17] = 0;
    usleep(rst_time);
    _reg[17] = 2;
    usleep(rst_time);
    _reg[17] = 3;
    usleep(rst_time);
    rst_time *= 2;
  } while( (_reg[16]&0x18)==0 );
  cm::service::printv("Reset complete\n");
}

void PlxBridge::reset()
{
  auto_reset();
  setup_memio();

  cm::service::printv("tag %04x\n",_tag);
}

#include "exception/Exception.hh"

uint32_t PlxBridge::read()
{
  sem_wait(&_sem);
  sem_post(&_sem);
  return -1;
}

//
//  Read an address from the other side of the bridge
//
uint32_t PlxBridge::read(const uint32_t* addr)
{ 
  uint32_t retval = -1;
  sem_wait(&_sem);
  if (pcie_rd(MEM_ACCESS,unsigned(addr))==SUCCESS) 
    retval = reinterpret_cast<uint32_t*>(_rx_buffer)[3];
  sem_post(&_sem);
  return retval;
}

//
//  Write to an address on the other side of the bridge
//
void     PlxBridge::write(uint32_t* addr, uint32_t v)
{
  sem_wait(&_sem);
  pcie_wr(MEM_ACCESS,unsigned(addr),bswap(v)); 
  sem_post(&_sem);
}

void     PlxBridge::set_verbose(unsigned v)
{
  verbose=v;
}

static void dump_reg(unsigned access, unsigned addr, const char* name)
{
  uint32_t* p = (uint32_t*)_rx_buffer;
  _pcie_rd(access,addr);
  if (get_response(200,0)==SUCCESS)
    cm::service::printv("%s: %08x\r\n",name,bswap(p[3]));
  else
    cm::service::printv("%s: ERR\r\n",name);
}

void     PlxBridge::dump() const
{
  unsigned laddr = _dev << 19;

  dump_reg(CFG_TYPE0 ,PECS_DEVICEID      ,"PECS_DEVICEID "); 
  dump_reg(CFG_TYPE0 ,PECS_PCICMD        ,"PECS_PCICMD   "); 
  dump_reg(CFG_TYPE0 ,PECS_PCIBAR0       ,"PECS_PCIBAR0  "); 
  dump_reg(CFG_TYPE0 ,PECS_PCIBUSNUM     ,"PECS_PCIBUSNUM"); 
  dump_reg(CFG_TYPE0 ,PECS_IOBASE        ,"PECS_IOBASE   ");
  dump_reg(CFG_TYPE0 ,PECS_MEMBASE       ,"PECS_MEMBASE  "); 
  dump_reg(CFG_TYPE0 ,PECS_BRIDGECTL     ,"PECS_BRIDGECTL"); 
  dump_reg(CFG_TYPE0 ,PECS_DEVCTL        ,"PECS_DEVCTL   "); 
  dump_reg(CFG_TYPE0 ,PECS_GPIOCTL       ,"PECS_GPIOCTL  "); 
  dump_reg(CFG_TYPE0 ,PECS_CRSTIMER      ,"PECS_CRSTIMER "); 

  dump_reg(CFG_TYPE1 ,LCS_DEVICEID|laddr ,"LCS_DEVICEID"); 
  dump_reg(CFG_TYPE1 ,LCS_PCICSR  |laddr ,"LCS_PCICSR  ");
  dump_reg(CFG_TYPE1 ,LCS_PCIBAR0 |laddr ,"LCS_PCIBAR0 ");
  dump_reg(CFG_TYPE1 ,LCS_PCIBAR2 |laddr ,"LCS_PCIBAR2 ");

  dump_reg(MEM_ACCESS,LCS_BASE+LCS_LAS0RR,"LCS_LAS0RR");
  dump_reg(MEM_ACCESS,LCS_BASE+LCS_LAS0BA,"LCS_LAS0BA");
  dump_reg(MEM_ACCESS,LCS_BASE+LCS_MARBR ,"LCS_MARBR ");
  dump_reg(MEM_ACCESS,LCS_BASE+LCS_BIGEND,"LCS_BIGEND");
  dump_reg(MEM_ACCESS,LCS_BASE+LCS_LBRD0 ,"LCS_LBRD0 ");
  dump_reg(MEM_ACCESS,LCS_BASE+LCS_IOCNTL,"LCS_IOCNTL");
  dump_reg(MEM_ACCESS,LCS_BASE+LCS_LMISC ,"LCS_LMISC ");
  dump_reg(MEM_ACCESS,LCS_BASE+LCS_LBRD1 ,"LCS_LBRD1 ");

  _rderr();
}

PlxBridge& PlxBridge::instance()
{
  if (_instance==0) _instance = new PlxBridge;
  return *_instance;
}

