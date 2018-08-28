
#include "rtems/iic_config.hh"
#include "debug/Debug.hh"
#include "debug/Print.hh"

#include "concurrency/Procedure.hh"
#include "concurrency/Semaphore.hh"
#include "concurrency/OldThread.hh"

#include "fm/FmTahoe.hh"
#include "cm_svc/opbintctrl.h"

extern "C" {
#include <rtems/rtems_bsdnet.h> // needed before any other bsdnet include
}
#include <stdio.h>
#include <exception>

struct rtems_bsdnet_config rtems_bsdnet_config = {0}; // set ifconfig to 0


class I2CHandler : public tool::concurrency::Procedure {
  enum {RST=0,IOW=1,RSP=2};
  enum {RADDR=0x218,RDATA=0x219};
public:
  I2CHandler() : _sem(tool::concurrency::Semaphore::Red) {}
public:
  void run() {
    _write_register(RSP,0);
    while(1) {
      _sem.take();

      unsigned a = _read_register(IOW);
      unsigned v = _read_register(a>>2);
      printf("iic wrote %x to addr %x\n",v,a);
    }
  }
  void unblock()
  {
    _write_register(RSP,0);
    _sem.give();
  }
private:
  void _write_register(unsigned addr, unsigned val)
  {
    asm volatile("mtdcr %0, %1" : : "i"(RADDR), "r"(addr));
    asm volatile("eieio");
    asm volatile("mtdcr %0, %1" : : "i"(RDATA), "r"(val));
    asm volatile("eieio");
  }
  unsigned _read_register(unsigned addr)
  {
    asm volatile("mtdcr %0, %1" : : "i"(RADDR), "r"(addr));
    asm volatile("eieio");
    unsigned w;
    asm volatile("mfdcr %0, %1" : "=r"(w) : "i"(RDATA));
    asm volatile("eieio");
    return w;
  }
private:
  tool::concurrency::Semaphore _sem;
};

static I2CHandler* iic_hdl;

#define IIC_IRQ 4

static void iic_isr(rtems_vector_number)
{
  iic_hdl->unblock();
}

extern "C" void init_executive()
{
  service::debug::clearMessages();

  //
  //  Hold Network Switch in Reset
  //
  FmTahoe* sw = new(0) FmTahoe;
  sw->resetChip();

  new tool::concurrency::OldThread("iichdl",100,8*1024,*(iic_hdl = new I2CHandler));

  //
  //  Register the I2C interrupt handler
  //
  rtems_isr_entry oldhdlr;
  opb_intc_set_vector(iic_isr, IIC_IRQ , &oldhdlr);
}
