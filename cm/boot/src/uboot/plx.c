

//#include <stdio.h>
#include <malloc.h>

#include "../plx.h"

/* control verbosity here */
#define DEBUG_PRINTF(...)
//#define DEBUG_PRINTF(...)  printf(__VA_ARGS__)

/* u-boot specific routines */
extern void udelay(unsigned long);
extern int printf(const char* fmt,...);
extern unsigned long get_timer(unsigned long);

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

static void _rderr(plx*);
static int _dev = 4;
static int verbose = 0;

static unsigned bswap(unsigned v)
{
  unsigned r = ((v>>24)&0x000000ff) |
               ((v>> 8)&0x0000ff00) |
               ((v<< 8)&0x00ff0000) |
               ((v<<24)&0xff000000);
  return r;
}

static unsigned rx_empty(plx* _plx)
{
  unsigned v=_plx->_reg[5];
  _plx->_rx_last=(v&0x40000000);
  return (v&0x80000000)==0;
}

static void rx_word(plx* _plx)
{
  uint32_t* bp = (uint32_t*)_plx->_rx_buffer;
  bp[1] = _plx->_reg[4];
  bp[0] = _plx->_reg[3];
  if (!_plx->_rx_last) {
    rx_empty(_plx);
    bp[3] = _plx->_reg[4];
    bp[2] = _plx->_reg[3];
    if (!_plx->_rx_last) {
      printf("rx_word reading more than 4 words\n");
    }
  }
}

static int get_response(plx* _plx, unsigned ms, int v)
{
  uint32_t* p = (uint32_t*)_plx->_tx_buffer;
  uint32_t* q = (uint32_t*)_plx->_rx_buffer;

  unsigned tag = p[1]&0xff00;

  uint64_t now = get_timer(0);
  uint64_t t   = now + ms;

  while(1) {
    if (!rx_empty(_plx)) {

      rx_word(_plx);

      if (verbose) {
        if (*q & 0x40000000)
          printf("rx %08x:%08x:%08x:%08x\n",(int)q[0],(int)q[1],(int)q[2],(int)q[3]);
        else
	      printf("rx %08x:%08x:%08x\n",(int)q[0],(int)q[1],(int)q[2]);
      }

      if ((q[2]&0xff00) == tag)
        return (q[1]&0xe000)==0 ? SUCCESS : CMPLERR;
    }

    if (now > t)
      break;

    now = get_timer(0);
  }

  if (!v) 
    printf("TMO: tx %08x:%08x:%08x:%08x\n",(int)p[0],(int)p[1],(int)p[2],(int)p[3]);
  else
    printf("TMO\n");
//  return TIMEOUT;
  return CMPLERR;
}

static void _fifo_wr3(plx* _plx)
{
  uint32_t* p = (uint32_t*)_plx->_tx_buffer;
  if (verbose)
    printf("tx: %08x %08x %08x\n",(int)p[0],(int)p[1],(int)p[2]);
  _plx->_reg[0] = p[0];
  _plx->_reg[1] = p[1];
  _plx->_reg[2] = 0xff;
  _plx->_reg[0] = p[2];
  _plx->_reg[1] = p[3];
  _plx->_reg[2] = 0x4000000f;
}

static void _fifo_wr4(plx* _plx)
{
  uint32_t* p = (uint32_t*)_plx->_tx_buffer;
  if (verbose)
    printf("tx: %08x %08x %08x %08x\n",(int)p[0],(int)p[1],(int)p[2],(int)p[3]);
  _plx->_reg[0] = p[0];
  _plx->_reg[1] = p[1];
  _plx->_reg[2] = 0xff;
  _plx->_reg[0] = p[2];
  _plx->_reg[1] = p[3];
  _plx->_reg[2] = 0x400000ff;
}

static void _pcie_rd (plx* _plx, unsigned type, unsigned addr)
{
  uint32_t* p = (uint32_t*)_plx->_tx_buffer;

  p[0] = ((type&0xff)<<24) | 1;
  p[1] = ((_plx->_tag++&0xff)<<8) | 0xf;
  p[2] = addr;
  p[3] = 0;
  _fifo_wr3(_plx);
}

static void _pcie_wr(plx* _plx, unsigned type, unsigned addr, unsigned data)
{
  uint32_t* p = (uint32_t*)_plx->_tx_buffer;

  p[0] = (((type|0x40)&0xff)<<24) | 1;
  p[1] = ((_plx->_tag++&0xff)<<8) | 0xf;
  p[2] = addr;
  p[3] = bswap(data);
  _fifo_wr4(_plx);
}

static int pcie_rd (plx* _plx, unsigned type, unsigned addr)
{
  _pcie_rd(_plx,type,addr);
  int result = get_response(_plx,20,0);
  if (result == CMPLERR)
    _rderr(_plx);

  return result;
}

static void pcie_wr(plx* _plx, unsigned type, unsigned addr, unsigned data)
{
  _pcie_wr(_plx, type, addr, data);

  if (type!=MEM_ACCESS) { // No completion for posted writes
    int result = get_response(_plx,20,0);
    if (result == CMPLERR)
      _rderr(_plx);
  }
}

static void pcie_rmw(plx* _plx, unsigned type, unsigned addr, unsigned data)
{
  uint32_t* p = (uint32_t*)_plx->_tx_buffer;
  p[0] = (((type)&0xff)<<24) | 1;
  p[1] = ((_plx->_tag++&0xff)<<8) | 0xf;
  p[2] = addr;
  p[3] = 0;
  _fifo_wr3(_plx);

  if (get_response(_plx,200,0)==CMPLERR)
    _rderr(_plx);

  p[0] = (((type|0x40)&0xff)<<24) | 1;
  p[1] = ((_plx->_tag++&0xff)<<8) | 0xf;
  p[2] = addr;
  p[3] = bswap(data);
  _fifo_wr4(_plx);

  if (type!=MEM_ACCESS) { // No completion for posted writes
    if (get_response(_plx,200,0)==CMPLERR)
      _rderr(_plx);
  }
  else
    udelay(20);

  p[0] = (((type)&0xff)<<24) | 1;
  p[1] = ((_plx->_tag++&0xff)<<8) | 0xf;
  p[2] = addr;
  p[3] = 0;
  _fifo_wr3(_plx);

  if (get_response(_plx,200,0)==CMPLERR)
    _rderr(_plx);
}

static void config_rmw(plx* _plx, unsigned addr, unsigned data)
{
  pcie_rmw(_plx, CFG_TYPE1, addr, data);
}

static void pecs_rmw(plx* _plx, unsigned addr, unsigned data)
{
  pcie_rmw(_plx, CFG_TYPE0, addr, data);
}

static void memory_rmw(plx* _plx, unsigned addr, unsigned data)
{
  pcie_rmw(_plx, MEM_ACCESS, addr, data);
}

static void set_devctl(plx* _plx)
{
//  pcie_wr(_plx, CFG_TYPE0, PECS_DEVCTL, 0xffff200f);
}

static void _rderr(plx* _plx)
{
  static unsigned count=0;
  uint32_t* p = (uint32_t*)_plx->_rx_buffer; 
  uint32_t* q = (uint32_t*)_plx->_tx_buffer;
  ++count;                           

  printf("_RDERR[%d]: tx %08x:%08x:%08x:%08x rx %08x:%08x:%08x:%08x tag %04x\n",
         count, (int)q[0],(int)q[1],(int)q[2],(int)q[3], (int)p[0],(int)p[1],(int)p[2],(int)p[3], _plx->_tag);
  _pcie_rd(_plx, CFG_TYPE0, PECS_DEVCTL);

  if ( get_response(_plx,200,0) == SUCCESS ) {
    unsigned stat = bswap(p[3]);                
    printf("PECS_DEVCTL: %08x",stat);
    if (stat&0x00010000) printf(" Correctable");
    if (stat&0x00020000) printf(" NonFatal");  
    if (stat&0x00040000) printf(" Fatal");    
    if (stat&0x00080000) printf(" UnsupportedReq");
    printf("\n\n");                                 
  }                                             
  else {                                       
    printf("PECS_DEVCTL: ERR\n");              
  }                                          

  //_pcie_wr(_plx, CFG_TYPE0, PECS_DEVCTL, 0xffff200f);
  set_devctl(_plx);

  _pcie_rd(_plx, CFG_TYPE0, PECS_PCICMD);          
  if (get_response(_plx,200,0) == SUCCESS) {      
    unsigned stat = bswap(p[3]);                    
    printf("PECS_PCICMD/STAT: %08x",stat);              
    if (stat&0x01000000) printf(" MasterDataParity");
    if (stat&0x08000000) printf(" SigTargetAbort"); 
    if (stat&0x10000000) printf(" RcvTargetAbort");
    if (stat&0x20000000) printf(" RcvMasterAbort");
    if (stat&0x40000000) printf(" SysSignalErr"); 
    if (stat&0x80000000) printf(" DetParityErr");
    printf("\n\n");                               
    _pcie_wr(_plx, CFG_TYPE0, PECS_PCICMD, stat | 0xffff0000);
  }                                           
  else {                                     
    printf("PECS_PCISTAT: ERR\n");           
  }                                        

  _pcie_rd(_plx, CFG_TYPE0, PECS_IOBASE);                 
  if (get_response(_plx,200,0) == SUCCESS) {             
    unsigned stat = bswap(p[3]);                           
    printf("PECS_SECSTAT: %08x",stat);                     
    if (stat&0x01000000) printf(" MasterDataParity");
    if (stat&0x08000000) printf(" SigTargetAbort"); 
    if (stat&0x10000000) printf(" RcvTargetAbort");
    if (stat&0x20000000) printf(" RcvMasterAbort");
    if (stat&0x40000000) printf(" SysSignalErr"); 
    if (stat&0x80000000) printf(" DetParityErr");
    printf("\n\n");                               
    _pcie_wr(_plx, CFG_TYPE0, PECS_IOBASE, stat | 0xffff0000);
  }                                           
  else {                                     
    printf("PECS_SECSTAT: ERR\n");           
  }                                        

  _pcie_rd(_plx, CFG_TYPE0, PECS_IRQSTAT);
  if (get_response(_plx,200,0) == SUCCESS) {
    printf("PECS_IRQSTAT: %08x\n",bswap(p[3]));
//    _pcie_wr(_plx, CFG_TYPE0, PECS_IRQSTAT, 0xffffffff);
  }
  else
    printf("PECS_IRQSTAT: ERR\n");
}

void setup_memio(plx* _plx)
{
  unsigned device = _dev;
  unsigned cfg_addr = device<<19;
  
  //  Local bus reset
  pecs_rmw  ( _plx, PECS_BRIDGECTL, 0x08630100 );
  udelay(20);
  
  //  Clear local bus reset
  pecs_rmw  ( _plx, PECS_BRIDGECTL, 0x00000100 );
  //  pecs_rmw  ( _plx, PECS_BRIDGECTL, 0x08230100 );
  udelay(50);
//  set_devctl(_plx);

  pecs_rmw  ( _plx, PECS_GPIOCTL, 0 );
  //  Map memory from 0 through configuration space
  unsigned PLCS_BASE = LCS_BASE>>20;
  pecs_rmw  ( _plx, cfg_addr | PECS_MEMBASE , (PLCS_BASE<<20) | (0));
  
  //  Memory access to PECS registers
  pecs_rmw( _plx, PECS_PCIBAR0, PCI_BASE );
  
  //  Enable memory space mapping on the upstream port
  pecs_rmw( _plx, PECS_PCICMD, 0xff000002 );
  
  memory_rmw( _plx, PCI_BASE | PECS_PCIBAR0, PCI_BASE );

  config_rmw( _plx, cfg_addr | LCS_PCIBAR0, LCS_BASE );
  config_rmw( _plx, cfg_addr | LCS_PCICSR, 0x02b10002 );
  pcie_rd( _plx, CFG_TYPE1, cfg_addr | LCS_PCICSR);
  
  memory_rmw( _plx, LCS_BASE+LCS_BIGEND, 0x00300104 );
  
  memory_rmw( _plx, LCS_BASE+LCS_LAS0RR, 0xFF000000 );

// Try disabling prefetch
  memory_rmw( _plx, LCS_BASE+LCS_LBRD0, 0xF0430043 );
  
// Enable PCIr2.2 compliance
//  memory_rmw( _plx, LCS_BASE+LCS_MARBR,  0x01200000 );
  memory_rmw( _plx, LCS_BASE+LCS_MARBR,  0x00200000 );

  config_rmw( _plx, cfg_addr | LCS_PCIBAR2, 0 );  // default
  
  memory_rmw( _plx, LCS_BASE+LCS_LAS0BA, 1 );
  set_devctl(_plx);
}

static void auto_reset(plx* _plx)
{
  //
  //  Bring the endpoint out of reset, then the root
  //
  unsigned rst_time = 20;
  do {
    DEBUG_PRINTF("Resetting PCIE bus [%d usec]\n",rst_time);
    _plx->_reg[17] = 0;
    udelay(rst_time);
    _plx->_reg[17] = 2;
    udelay(rst_time);
    _plx->_reg[17] = 3;
    udelay(rst_time);
    rst_time *= 2;
  } while( (_plx->_reg[16]&0x18)==0 );
  DEBUG_PRINTF("Reset complete\n");
}

plx* plx_init(uint32_t addr)
{
  plx* _plx  = malloc(sizeof(plx));

  _plx->_tx_buffer = (uint128_t*)(((uint32_t)&_plx->_buffer[0] + 0xf)&~0xf);
  _plx->_rx_buffer = _plx->_tx_buffer+1;
  _plx->_tag = 0;
  _plx->_reg = (volatile unsigned*)addr;

  auto_reset (_plx);
  setup_memio(_plx);

  return _plx;
}

unsigned plx_read(plx* _plx, const uint32_t* addr)
{
  uint32_t retval = -1;
  if (pcie_rd(_plx,MEM_ACCESS,(unsigned)addr)==SUCCESS) {
    uint32_t* p = (uint32_t*)_plx->_rx_buffer;
    retval = p[3];
  }
  return retval;
}

void plx_write(plx* _plx, uint32_t* addr, uint32_t v)
{
  pcie_wr(_plx,MEM_ACCESS,(unsigned)addr,bswap(v)); 
}
