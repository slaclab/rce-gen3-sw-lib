
#include "fm/FmReg.hh"

#include "debug/Print.hh"

#if defined(ppc405) || defined(ppc440)
#include <rtems/system.h>
#include <rtems/rtems/status.h>
#include <rtems/score/isr.h>
#include <rtems/rtems/intr.h>  // rtems_interrupt_disable/enable
#ifdef ppc440
#include "cpu/apu.hh"
#include "apu/apu.hh"
#include <time.h>
#endif
#endif

#include <stdlib.h>

static int _status = 0;

int  FmReg::status() { return _status; }
void FmReg::clear () { _status = 0; }

#ifdef ppc405

void FmReg::init() {}

//
//  Write the value to the device
//
FmReg& FmReg::operator=(const unsigned v)
{
  unsigned reg = (unsigned)this;

  unsigned level;
  rtems_interrupt_disable( level );


  //  The synchronous transaction
  //  asm volatile("mtdcr %0,%1" : : "i"(TAHOE_ADDR_REG), "r"(reg));
  //  asm volatile("mtdcr %0,%1" : : "i"(TAHOE_DATA_REG), "r"(v));

  //  Setup the asynchronous transaction
  asm volatile("mtdcr %0,%1" : : "i"(TAHOE_DATA_REG), "r"(v));
  asm volatile("eieio");
  asm volatile("mtdcr %0,%1" : : "i"(TAHOE_CMND_REG), "r"(reg));

  //  Poll for completion (1, sometimes 2 reads)
  //  A timeout is implemented in the firmware
  unsigned s;
  do {
    asm volatile("eieio");
    asm volatile("mfdcr %0,%1" : "=r"(s) : "i"(TAHOE_CMND_REG));
  } while ( s & 0x80000000 );

  //  Report unacknowledged transactions
  if (~s&2) {
    service::debug::printv("No ack writing register %p/%x\n",this,reg>>2);
    _status = -1;
    /*
    const int LED_REG = 0x2f7;
    unsigned err = 0xEE00 | (reg>>24);
    asm volatile("mtdcr %0,%1" : : "i"(LED_REG), "r"(err));
    */
  }

  rtems_interrupt_enable( level );

  return *this;
}

//
//  Read the value directly from the device
//
FmReg::operator unsigned() const
{
  unsigned v;
  unsigned reg = (unsigned)this | 1;

  unsigned level;
  rtems_interrupt_disable( level );


  //  The synchronous transaction
  //  asm volatile("mtdcr %0,%1" : : "i"(TAHOE_ADDR_REG), "r"(reg));
  //  asm volatile("mfdcr %0,%1" : "=r"(v) : "i"(TAHOE_DATA_REG));

  //  Setup the asynchronous transaction
  asm volatile("mtdcr %0,%1" : : "i"(TAHOE_CMND_REG), "r"(reg));
  unsigned s;
  do {
    asm volatile("eieio");
    asm volatile("mfdcr %0,%1" : "=r"(s) : "i"(TAHOE_CMND_REG));
  } while ( s & 0x80000000 );
  asm volatile("eieio");
  asm volatile("mfdcr %0,%1" : "=r"(v) : "i"(TAHOE_DATA_REG));

  //  Report unacknowledged transactions
  if (~s&2) {
    service::debug::printv("No ack reading register %p/%x (%x)\n",this,reg>>2,v);
    _status = -2;
    const int LED_REG = 0x2f7;
    unsigned err = 0xEE00 | (reg>>24);
    asm volatile("mtdcr %0,%1" : : "i"(LED_REG), "r"(err));
  }

  rtems_interrupt_enable( level );

  return v;
}
#else
#ifdef ppc440

namespace mytime {

  static void tb_read(unsigned* tsu, unsigned* tsl)
  {
    register unsigned tbu,tbl,tbr;
    do {
      __asm__ volatile("mftbu %0": "=r"(tbu));
      __asm__ volatile("mftbl %0": "=r"(tbl));
      __asm__ volatile("mftbu %0": "=r"(tbr));
    } while( tbr!=tbu );
    *tsu = tbu;
    *tsl = tbl;
  }

  static void tb_read64(uint64_t* tb)
  {
    unsigned tbu, tbl;
    tb_read(&tbu,&tbl);

    *tb = tbu;
    *tb <<= 32;
    *tb += tbl;
  }

  static void spin(unsigned ms)
  {
    uint64_t now, t, tp;

    tb_read64(&t);

    tp = ms;
    tp *= 15000000;
    tp /= 32;
    t += tp;

    while(1) {
      tb_read64(&now);
      if (now > t)
        break;
    }
  }

  static void nanosleep_(timespec* tv, timespec*)
  {
    unsigned level;
    rtems_interrupt_disable( level );

    unsigned ms = tv->tv_sec*1000 + tv->tv_nsec/1000000;
    mytime::spin(ms);

    rtems_interrupt_enable( level );
  }
};

using mytime::nanosleep_;

static unsigned tag = 0;  // pciE message tag

static char alignBase[32];

using tool::cpu::uint128_t;

static uint128_t* align_p;

static void pcie_rst()
{
  timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 20000000; // (20 msec should be sufficient)

  tool::cpu::APU apu;
  unsigned i=0;
  uint32_t t;
  unsigned a=0, b=0;
  apu.read<APU_PCIE_READ>(&t, a, b);

  //  assert reset
  b = t | 1;
  apu.write<APU_PCIE_WRITE>(t, a, b);

  apu.read<APU_PCIE_READ>(&t, a, b);

  asm volatile ("eieio");

  nanosleep_(&ts,0);

  //  release reset
  b = t & ~1;
  apu.write<APU_PCIE_WRITE>(t, a, b);

  asm volatile ("eieio");

  nanosleep_(&ts,0);

  service::debug::printv("pcie_rst waiting for link up");

  //  wait for link up
  uint32_t prev = 0;
  do {
    apu.read<APU_PCIE_READ>(&t, a, b);
    if ((t&0xffff0000)!=(prev&0xffff0000))
      service::debug::printv("read %08x",prev=t);
  } while (!(((t>>29)&1)==0 && ((t>>20)&0x7)==0x6));
}

static inline bool tx_empty()
{
  tool::cpu::APU apu;
  uint32_t t;
  unsigned a=0, b=0;
  apu.read<APU_PCIE_READ>(&t, a, b);

  return (t&0x30000) == 0x2000;
}

static inline bool rx_empty()
{
  tool::cpu::APU apu;
  uint32_t t;
  unsigned a=0, b=0;
  apu.read<APU_PCIE_READ>(&t, a, b);

  return t&0x80000;
}

static inline void tx_word(uint128_t* bp, int i)
{
  //  uint32_t* p = (uint32_t*)bp;
  //  service::debug::printv(" tx: %08x %08x %08x %08x",p[0],p[1],p[2],p[3]);

//   unsigned k=0;
//   while(!tx_empty()) k++;
//   if (k)
//     service::debug::printv(" tx_empty %d",k);

  tool::cpu::APU apu;
  apu.lqfcmx<APU_PCIE_LOAD>(bp, i);
}

static inline void rx_word()
{
  tool::cpu::APU apu;
  int i = 0;
  uint128_t* bp = align_p;

  apu.stqfcmx<APU_PCIE_STORE>(bp, i);

  //  uint32_t* p = (uint32_t*)bp;
  //  service::debug::printv(" rx: %08x %08x %08x %08x",p[0],p[1],p[2],p[3]);
}

static void get_response()
{
  int i;
  for(i=0; i<1000; i++) {
    if (!rx_empty()) {
      //      service::debug::printv(" rx_empty %d",i);
      rx_word();
      return;
    }
  }

  service::debug::printv(" get response tmo");
  _status = -1;
}

int query_device(int type, int bus, int device)
{
  uint128_t* bp = align_p;
  uint32_t* p = (uint32_t*)bp;

  while(!rx_empty()) {
    rx_word();
  }

  //
  // Configuration Read Type 0 : 0x04000001
  // Configuration Read Type 1 : 0x05000001
  // Memory Read               : 0x00000001
  // Memory Write              : 0x40000001
  //

  *p++ = ((type&0xff)<<24) | 1;
  *p++ = ((tag++&0xff)<<8) | 0xf;
  *p++ = (bus<<24) | (device<<19);
  *p++ = 0;

  int i = 0;
  tx_word(bp,i);

  get_response();
  return (_status==0);
}

//
//  Write the value to the device
//
FmReg& FmReg::operator=(const unsigned v)
{
  //
  //  Workaround for fact that the PEX8311 asserts Data[31:0] one LCLK after
  //    address strobe -
  //    issue the first write to a readonly register, then 
  //    issue a second write to the correct register before the tri-stated data
  //    bus recovers.
  //

  uint128_t* bp = align_p;
  uint32_t* p = (uint32_t*)bp;
  uint32_t* a = p+2;
  const unsigned READONLY_ADDR = 0xc10; // VITAL_PRODUCT_DATA

  unsigned level;
  rtems_interrupt_disable( level );

#if 0
  if (!tx_empty()) {
    service::debug::printv(" tx not empty");
  }
#endif

  *p++ = 0x40000001;
  *p++ = ((tag++&0xff)<<8) | 0xf;
  *p++ = READONLY_ADDR;
  *p   = v;

  int i = 0;
  tx_word(bp,i);

  *a = unsigned(this);
  tx_word(bp,i);

  {
    tool::cpu::APU apu;
    uint32_t t;
    unsigned a=0, b=0;
    apu.read<APU_PCIE_READ>(&t, a, b);

    //    uint32_t* p = (uint32_t*)align_p;
    //    service::debug::printv(" write %08x:%08x:%08x %08x",p[1],p[2],p[3],t);
  }

  asm volatile("eieio");

  rtems_interrupt_enable( level );

  return *this;
}

//
//  Read the value directly from the device
//
FmReg::operator unsigned() const
{
  uint128_t* bp = align_p;
  uint32_t* p = (uint32_t*)bp;
  *p++ = 1;
  *p++ = ((tag++&0xff)<<8) | 0xf;
  *p++ = unsigned(this);
  *p   = 0;

  unsigned level;
  rtems_interrupt_disable( level );

  int i = 0;
  tx_word(bp,i);
  
  get_response();

  {
    tool::cpu::APU apu;
    uint32_t t;
    unsigned a=0, b=0;
    apu.read<APU_PCIE_READ>(&t, a, b);

    //    uint32_t* p = (uint32_t*)align_p;
    //    service::debug::printv(" read %08x:%08x %08x",p[1],p[2],t);
    if (_status==-1)
      service::debug::printv("FmReg read failed %p\n",this);
  }

  rtems_interrupt_enable( level );

  return *p;
}

enum { MEM_ACCESS = 0,
       CFG_TYPE0  = 0x4,
       CFG_TYPE1  = 0x5 };

void pcie_wr(unsigned type, unsigned addr, unsigned data)
{
  uint128_t* bp = align_p;
  uint32_t* p = (uint32_t*)bp;

  *p++ = (((type|0x40)&0xff)<<24) | 1;
  *p++ = ((tag++&0xff)<<8) | 0xf;
  *p++ = addr;
  __asm__ volatile("stwbrx %0,%1,%2" : : "r"(data), "r"(0), "r"(p));

  int i = 0;
  tx_word(bp,i);

  if (type!=MEM_ACCESS)  // No completion for posted writes
    get_response();
}

void pecs_wr (unsigned addr, unsigned data)
{
  pcie_wr(CFG_TYPE0, addr, data);
}

void config_wr (unsigned addr, unsigned data)
{
  pcie_wr(CFG_TYPE1, addr, data);
}

void memory_wr (unsigned addr, unsigned data)
{
  return pcie_wr (MEM_ACCESS, addr, data);
}

//  Configuration Type 0 Accesses
enum { PECS_DEVICEID  = 0,
       PECS_PCICMD    = 0x4,
       PECS_PCIBAR0   = 0x10,
       PECS_PCIBUSNUM = 0x18,
       PECS_MEMBASE   = 0x20,
       PECS_BRIDGECTL = 0x3C };

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
       LCS_LMISC      = 0x8C };

enum { LCS_BASE = 0xfffffe00 };  //  Map LCS registers to memory

void FmReg::init()
{
  timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 20000000; // (20 msec should be sufficient)

  align_p = (uint128_t*)(((uint32_t)&alignBase[0] + 0xf)&~0xf);

  pcie_rst();

  service::debug::printv("pcie_rst complete with status %d",_status);

  { unsigned dev = 0;
    unsigned nresponse = 0;
    while(nresponse<2) {
      if ( query_device(CFG_TYPE0, 0, dev) ) {
        service::debug::printv("Query device type0 %d success",dev);
        nresponse++;
      }
      else {
        service::debug::printv("Query device type0 %d failed",dev);
      }
      dev = (dev+1)%8;
    }
    
    nresponse = 0;
    dev = 4;
    while(nresponse<2) {
      if ( query_device(CFG_TYPE1, 0, dev) ) {
        service::debug::printv("Query device type0 %d success",dev);
        nresponse++;
      }
      else {
        service::debug::printv("Query device type1 %d failed",dev);
      }
    }
  }

  unsigned device   = 4;
  unsigned cfg_addr = device<<19;

  //  Local bus reset
  pecs_wr  ( PECS_BRIDGECTL, 0x00400100 );

  nanosleep_(&ts, 0);

  //  Clear local bus reset
  pecs_wr  ( PECS_BRIDGECTL, 0x100 );
  nanosleep_(&ts, 0);

  service::debug::printv("local bus reset complete with status %d\n",_status);

  //  Map memory from 0 through configuration space
  unsigned PLCS_BASE = LCS_BASE>>20;
  pecs_wr  ( cfg_addr | PECS_MEMBASE , (PLCS_BASE<<20) | (0));

  //  Memory access to PECS registers
  pecs_wr( PECS_PCIBAR0, 0xfffe0000 );

  //  Enable memory space mapping on the upstream port
  pecs_wr( PECS_PCICMD, 0xff000002 );

  config_wr( cfg_addr | LCS_PCIBAR0, LCS_BASE );
  config_wr( cfg_addr | LCS_PCICSR, 0x02b00002 );

  memory_wr( LCS_BASE+LCS_BIGEND, 0x00300104 );

  //  Default
  //  memory_wr( LCS_BASE+LCS_LBRD0, 0x40430043 );
  //  Disable read prefetches
  //  Extend read retry delay to 64 clocks
  memory_wr( LCS_BASE+LCS_LBRD0, 0x80430143 );

  memory_wr( LCS_BASE+LCS_LAS0RR, 0xFF000000 );

  config_wr( cfg_addr | LCS_PCIBAR2, 0 );  // default

  memory_wr( LCS_BASE+LCS_LAS0BA, 1 );

  service::debug::printv("pcie link up with status %d\n",_status);
}


#else

#include "cm_svc/PlxBridge.hh"
#include <stdio.h>

//  Memory map PCIE interface
//    (assume PLX setup is done elsewhere and persists)
void FmReg::init() { printf("FmReg::init\n"); }

//  Write a register over PCIE
FmReg& FmReg::operator=(const unsigned v)
{
  uint32_t* addr = reinterpret_cast<uint32_t*>(this);
  cm::svc::PlxBridge::instance().write(addr,v);
  return *this;
}

//  Read a register over PCIE
FmReg::operator unsigned() const
{
  const uint32_t* addr = reinterpret_cast<const uint32_t*>(this);
  return cm::svc::PlxBridge::instance().read(addr);
}

#endif
#endif

