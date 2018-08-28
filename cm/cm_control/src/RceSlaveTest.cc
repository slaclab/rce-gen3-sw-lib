
#include "control/RceSlaveTest.hh"

#include "cm_svc/opbintctrl.h"
#include "console/RemoteConsole.hh"

#include "cpu/apu.hh"


extern "C" {
#include <rtems.h>
}

#include <stdio.h>
#include <string.h>
#include <strings.h>

//#define DBUG

typedef unsigned (*VALFCN)(unsigned,unsigned);

static unsigned _readIicReg(unsigned addr);
static unsigned _readIicCmd();
static void     _readIicCmdA(char*);
static unsigned _readApuCfg();
static void     _writeIicReg(unsigned addr, unsigned w);
static void     _writeIicCmd(unsigned w);
static void     _writeIicCmdA(const char* args);
static void     _writeApuCfg(unsigned w);
static void     _iicmstest(const char* title,
                           uint8_t     dev,
                           unsigned    nbytes,
                           VALFCN      fcn);
static unsigned _readChipScope();
static void     _writeChipScope(unsigned);

static void     _i2cs_isr(rtems_vector_number);
#define I2CS_IRQ 5

static unsigned REG_LO=0x0010;
static unsigned REG_HI=0x0080;
static unsigned _i2cs_reg=0;
static unsigned _i2cs_nb;
static char     _i2cs_msg[1024] = {0};

static char qwbuf[32];
static unsigned* _qwbuf = (unsigned*)((unsigned(qwbuf)+0xf)&~0xf);

static unsigned register_value(unsigned base, unsigned off)
{ return ((base+off)&0xff)|0x80; }

static unsigned register_clear(unsigned base, unsigned off)
{ return 0xff; }

#define SCANUARG(var,name) \
  unsigned var; \
  if (!sscanf(args,"%x",&var)) { \
    o.printv("Error scanning %s\n",name); \
    continue; \
  }

RceSlaveTest::RceSlaveTest()
{
//   rtems_isr_entry oldhdlr;
//   opb_intc_set_vector(_i2cs_isr, I2CS_IRQ , &oldhdlr);
}

RceSlaveTest::~RceSlaveTest()
{
}

bool RceSlaveTest::handle(service::console::RemoteConsole& o,
                          const char* cmd,
                          const char* args)
{
  do {
    if (!strcasecmp(cmd,"IICREAD")) {
      SCANUARG(regaddr,"addr");
      unsigned regval = _readIicReg(regaddr);
      o.printv("Read IIC Reg[%x] = %08x\n",regaddr,regval);
      continue;
    }
    else if (!strcasecmp(cmd,"IICWRITE")) {
      unsigned regaddr, regval;
      if (sscanf(args,"%x %x",&regaddr,&regval)!=2) {
        o.printv("Error scanning <regaddr> <regval>");
        continue;
      }
      o.printv("Write IIC Reg[%x] = %08x\n",regaddr,regval);
      _writeIicReg(regaddr,regval);
      continue;
    }
    else if (!strcasecmp(cmd,"IICREADM")) {
      SCANUARG(ncmds,"ncommand");
      char buff[256];
      char* p = buff;
      p += sprintf(p,"Read IIC Cmds[%d]:",ncmds);
      for(unsigned i=0; i<ncmds; i++) {
        unsigned v = _readIicCmd();
        p += sprintf(p," %08x",v);
      }
      o.printv("%s\n",buff);
      continue;
    }
    else if (!strcasecmp(cmd,"IICWRITEMA")) {
      _writeIicCmdA(args);
      continue;
    }
    else if (!strcasecmp(cmd,"IICREADMA")) {
      SCANUARG(ncmds,"ncommand");
      char buff[256];
      char* p = buff;
      p += sprintf(p,"Read IIC Cmds[%d]:",ncmds);
      for(unsigned i=0; i<ncmds; i++) {
        _readIicCmdA(p);
        p += strlen(p);
      }
      o.printv("%s\n",buff);
      continue;
    }
    else if (!strcasecmp(cmd,"ENAPU")) {
      tool::cpu::APU().enable();

      o.printv("APU enabled\n");
      continue;
    }
    else if (!strcasecmp(cmd,"APUCFGR")) {
      unsigned w = _readApuCfg();
      o.printv("APUCFG = %x\n",w);
      continue;
    }
    else if (!strcasecmp(cmd,"APUCFGW")) {
      SCANUARG(cfg,"config");
      _writeApuCfg(cfg);
      continue;
    }
    else if (!strcasecmp(cmd,"IICWRITEM")) {
      SCANUARG(cmdval,"command");
      o.printv("Write IIC Cmd[%x]\n",cmdval);
      _writeIicCmd(cmdval);
      continue;
    }
    else if (!strcasecmp(cmd,"IICSTEST")) {
      SCANUARG(cmdval,"command");
      if (cmdval) {
        _i2cs_reg = REG_LO;
        _i2cs_nb  = 1;
        _i2cs_msg[0] = 0;
      }
      else {
        o.printv("disabled @ reg %04x  nb %d\n",
                 _i2cs_reg, _i2cs_nb);
        if (_i2cs_msg[0])
          o.printv("%s\n",_i2cs_msg);
        _i2cs_reg = 0;
      }
      continue;
    }
    else if (!strcasecmp(cmd,"IICMTEST")) {
      SCANUARG(cmdval,"start/stop");
      //      _mtest->enable(cmdval);
      continue;
    }
    else if (!strcasecmp(cmd,"IICMSTEST")) {
      SCANUARG(cmdval,"bytes");
      uint8_t dev = 0x10;
      for(unsigned nbytes=1; nbytes<cmdval; nbytes++) {
        _iicmstest("Clearing Registers",dev,nbytes,register_clear);
        _iicmstest("Writing Registers ",dev,nbytes,register_value);
      }
    }
    else if (!strcasecmp(cmd,"DUMPCS")) {
      char buff[64];
      unsigned v(0);
      printf("T0....5....0....5....0....5....0\n");
      for(unsigned i=0; i<256; i++) {
        v = _readChipScope();
        for(unsigned k=0; k<32; k++, v<<=1)
          buff[k] = (v&(1<<31)) ? '1':'0';
        buff[32] = (i&1) ? '+':'-';
        buff[33] = 0;
        printf("%s\n",buff);
      }
      _writeChipScope(0);
    }
    else
      return false;

  } while(0);

  return true;
}


static const unsigned RI2CCMD = 0x218;
static const unsigned RI2CDAT = 0x219;
static const unsigned RI2CM   = 0x21A;
static const unsigned RAPU    = 0x105;

unsigned _readIicReg(unsigned addr)
{
  asm volatile("mtdcr %0, %1" : : "i"(RI2CCMD), "r"(addr));
  asm volatile("eieio");

  unsigned w;
  asm volatile("mfdcr %0, %1" : "=r"(w) : "i"(RI2CDAT));
  asm volatile("eieio");
  return w;
}

void _writeIicReg(unsigned addr, unsigned w)
{
  asm volatile("mtdcr %0, %1" : : "i"(RI2CDAT), "r"(w));
  asm volatile("eieio");

  unsigned a = addr | (1<<8); // write cmd
  asm volatile("mtdcr %0, %1" : : "i"(RI2CCMD), "r"(a));
  asm volatile("eieio");
}

unsigned _readIicCmd()
{
  unsigned w;
  asm volatile("mfdcr %0, %1" : "=r"(w) : "i"(RI2CM));
  asm volatile("eieio");
  return w;
}

void _writeIicCmd(unsigned w)
{
  asm volatile("mtdcr %0, %1" : : "i"(RI2CM), "r"(w));
  asm volatile("eieio");
}

void _readIicCmdA(char* p)
{
  uint8_t* wbuf = reinterpret_cast<uint8_t*>(_qwbuf);

  asm volatile("stqfcmx 0,%0,%1" : : "r"(0), "r"(wbuf));

  unsigned nbyt = wbuf[15^3];
  p += sprintf(p,"(%02x)",nbyt);

  for(unsigned i=0; i<nbyt; i++)
    p += sprintf(p," %02x",wbuf[i^3]);
}

void _writeIicCmdA(const char* args)
{
  uint8_t* cmd = reinterpret_cast<uint8_t*>(_qwbuf);

  uint8_t nbyt;
  args = strtok(0," ");
  if (!sscanf(args,"%hhx",&nbyt))
    return;

  for(int i=0; i<nbyt; i++)
    if (!sscanf(strtok(0," "),"%hhx",&cmd[i^3]))
      continue;

  cmd[15^3] = nbyt;

  //  I suspect that the PPC405 errata describes a problem I've
  //  seen where the APU can generate neither a WRITEBACKOK nor
  //  FLUSH signal (hanging the FCM). Described software fixes
  //  use a "nop" instruction before/after the "lqfcmx".
  asm volatile("nop");
  asm volatile("lqfcmx 0,%0,%1" : : "r"(0), "r"(cmd));
  asm volatile("nop");
}

unsigned _readApuCfg()
{
  unsigned w;
  asm volatile("mfdcr %0, %1" : "=r"(w) : "i"(RAPU));
  asm volatile("eieio");
  return w;
}

void _writeApuCfg(unsigned w)
{
  asm volatile("mtdcr %0, %1" : : "i"(RAPU), "r"(w));
  asm volatile("eieio");
}

void     _i2cs_isr(rtems_vector_number)
{
  _writeIicReg(2,0);
  if (_i2cs_reg) {
    unsigned w = _readIicReg(1)&0xffff;
    if (w!=_i2cs_reg) {
      sprintf(_i2cs_msg+strlen(_i2cs_msg),"Reg[1] = %x (%x)[%d]",
              w,_i2cs_reg,_i2cs_nb);
      _i2cs_reg = 0;
    }
    _i2cs_reg+=_i2cs_nb;
    if (_i2cs_reg>=REG_HI) {
      _i2cs_reg = REG_LO;
      _i2cs_nb++;
    }
  }
}

static void _iicmapu()
{
  uint32_t* cmd = reinterpret_cast<uint32_t*>(_qwbuf);
#ifdef DBUG
  printf("push %08x:%08x:%08x:%08x\n",cmd[0],cmd[1],cmd[2],cmd[3]);
#endif
  asm volatile("nop");
  asm volatile("lqfcmx 0,%0,%1" : : "r"(0), "r"(cmd));
  asm volatile("nop");
  unsigned nbyt=0;
  unsigned ntries;
  memset(cmd,0,16);
  for(ntries=0; ntries<100000 && nbyt<0x80; ntries++) {
    asm volatile("nop");
    asm volatile("stqfcmx 0,%0,%1" : : "r"(0), "r"(cmd));
    asm volatile("nop");
    nbyt = cmd[3]>>24;
  }
#ifdef DBUG
  printf("pop  %08x:%08x:%08x:%08x [%d]: ",
         cmd[0],cmd[1],cmd[2],cmd[3],ntries);
  printf(" %02x:%02x:%02x:%02x\n",
         (cmd[3]>>18)&0x3f,
         (cmd[3]>>12)&0x3f,
         (cmd[3]>> 6)&0x3f,
         (cmd[3]>> 0)&0x3f);
#endif
}

void _iicmstest(const char* title,
                uint8_t     dev,
                unsigned    nbytes,
                VALFCN      fcn)
{
  uint8_t* cmd = reinterpret_cast<uint8_t*>(_qwbuf);
  _i2cs_msg[0] = 0;

  printf("%s : %d bytes per op\n",title,nbytes);
  for(uint16_t i=REG_LO; i<=REG_HI; i+=nbytes) {
    unsigned nb = (REG_HI-i < nbytes) ? REG_HI-i : nbytes;
    _i2cs_reg = i;

    cmd[0^3] = dev;
    cmd[1^3] = i&0xff;
    cmd[2^3] = i>>8;
    for(unsigned j=0; j<nb; j++)
      cmd[(j+3)^3] = fcn(i,j);
    cmd[15^3] = nb+3;
    _iicmapu();
  }

  _i2cs_reg = 0;
  if (_i2cs_msg[0])
    printf("%s\n",_i2cs_msg);

  for(int k=0; k<2; k++) {
    printf("Verifying %d\n",k+1);

    cmd[0^3]  = dev;
    cmd[1^3]  = REG_LO&0xff;
    cmd[2^3]  = REG_LO>>8;
    cmd[15^3] = 3;
    _iicmapu();

    for(uint16_t i=REG_LO; i<=REG_HI-nbytes; i+=nbytes) {
      unsigned nb = (REG_HI-i < nbytes) ? REG_HI-i : nbytes;

      cmd[0^3]  = dev|1;
      cmd[15^3] = nb+1;
      _iicmapu();

      for(unsigned j=0; j<nb; j++)
        if (cmd[(j+1)^3] != fcn(i,j))
          printf("Read[%x] %02x(%02x)\n",i+j,cmd[(j+1)^3],fcn(i,j));
    }
  }

  printf("Verifying 3\n");
  for(uint16_t i=REG_LO; i<=REG_HI-nbytes; i+=nbytes) {
    unsigned nb = (REG_HI-i < nbytes) ? REG_HI-i : nbytes;

    cmd[0^3]  = dev;
    cmd[1^3]  = i&0xff;
    cmd[2^3]  = i>>8;
    cmd[15^3] = 3;
    _iicmapu();

    cmd[0^3]  = dev|1;
    cmd[15^3] = nb+1;
    _iicmapu();

    for(unsigned j=0; j<nb; j++)
      if (cmd[(j+1)^3] != fcn(i,j))
        printf("Read[%x] %02x(%02x)\n",i+j,cmd[(j+1)^3],fcn(i,j));
  }

#if 0
  printf("Verifying 3\n");
  toRepeat.sort();
  toRepeat.unique();
  for(std::list<uint16_t>::const_iterator it=toRepeat.begin();
      it!=toRepeat.end(); it++) {
    uint16_t i = (*it);
    cmd[0^3] = dev|1;
    cmd[1^3] = i&0xff;
    cmd[2^3] = i>>8;


    i2cBus->write(dev, 2, toWrite);

    toRead[0] = 0;
    i2cBus->read(dev, 1, toRead);
    if (toRead[0] != fcn(i,0)) {
      printf("Read[%x] %02x(%02x)\n",i,toRead[0],fcn(i,0));
    }
  }
#endif
}

static const unsigned RCSCOPE    = 0x232;

unsigned _readChipScope()
{
  unsigned w(0);
  asm volatile("mfdcr %0, %1" : "=r"(w) : "i"(RCSCOPE));
  asm volatile("eieio");
  return w;
}

void _writeChipScope(unsigned w)
{
  asm volatile("mtdcr %0, %1" : : "i"(RCSCOPE), "r"(w));
  asm volatile("eieio");
}
