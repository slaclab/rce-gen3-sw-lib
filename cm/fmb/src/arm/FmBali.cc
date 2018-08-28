
#include "fmb/FmBali.hh"

#include <time.h>

#include <stdio.h>

using namespace Bali;

FmBali::FmBali()
{
}

void* FmBali::operator new(unsigned s, int id)
{
  void* p = (void*)(FM_DEV_BASE + id*FM_DEV_SIZE);
  return p;
}

void  FmBali::operator delete(void*)
{
}

void  FmBali::destroy()
{
  //  delete this;
}

unsigned FmBali::id() const
{
  return unsigned(this)>>24;
}

#include "cm_svc/PlxBridge.hh"

FmCpuPortStatus FmBali::cpuPortStatus()
{
  return FmCpuPortStatus(0);
}

void FmBali::resetCore()
{
  printf("entered %s\n", __PRETTY_FUNCTION__);
}

void FmBali::resetChip()
{
  cm::svc::PlxBridge::instance().reset();
}

void FmBali::releaseChip()
{
}

int FmBali::txPacket(const unsigned* data,
		      unsigned   len,
		      FmPortMask fmPortMask)
{
  unsigned dmask(fmPortMask);

  //
  //  In lieu of a DMA controller in the Switch Manager firmware
  //
  FmBali& t = *this;

  if (!len) return 0;

  //  Poll this register before every word written to txfifo
  if (!_poll(lci_status,
	     FmBali::Lci_Status::TxRdy,
	     FmBali::Lci_Status::TxRdy,
	     1024)) {
    printf("CPU port not ready for Tx (-2)\n");
    return 0;
  }

  len+=4;  // add extra 4 bytes for CRC
  t.lci_tx_fifo = (dmask ? (1<<31) : 0) | (len>64 ? len : 64);

  const unsigned* d = data;
  const unsigned* const end = d + ((len+3)>>2);

  if (!_poll(lci_status,
	     FmBali::Lci_Status::TxRdy,
	     FmBali::Lci_Status::TxRdy,
	     1024)) {
    printf("CPU port not ready for Tx (-1)\n");
    return 0;
  }
  t.lci_tx_fifo = dmask;  // destination mask (mask of ports + 0 = lookup)

  do {
    if (!_poll(lci_status,
	       FmBali::Lci_Status::TxRdy,
	       FmBali::Lci_Status::TxRdy,
	       1024)) {
      printf("CPU port not ready for Tx (%d)\n",d-data);
      return 0;
    }
    t.lci_tx_fifo = *d;
    d++;
  } while ( d < end );

  //  Pad to the minimum ethernet frame length (64 octets)
  while( len < 64 ) {
    if (!_poll(lci_status,
	       FmBali::Lci_Status::TxRdy,
	       FmBali::Lci_Status::TxRdy,
	       1024)) {
      printf("CPU port not ready for Tx (l%d)\n",len);
      return 0;
    }
    t.lci_tx_fifo = 0;
    len+=4;
  }

  return ~t.lci_ip & Lci_Ip::TxOverrun;
}

int FmBali::rxPacket(unsigned* d)
{
  if (!(lci_status & FmBali::Lci_Status::RxRdy))
    return 0;

  unsigned* p = d;
  unsigned* const end = p + (1512>>2);

  bool ldone=false;
  while(!ldone) {
    ldone = (lci_status & FmBali::Lci_Status::RxEOT);
    if (!_poll(lci_status,
	       FmBali::Lci_Status::RxRdy,
	       FmBali::Lci_Status::RxRdy,
	       1024)) {
      printf("CPU port not ready for Rx\n");
      return 0;
    }
    *p++ = lci_rx_fifo;
    if (p >= end) return 0;
  }
  return p-d;
}

int FmBali::_poll(FmMReg& r,unsigned mask,unsigned val,unsigned timeout)
{
  volatile unsigned v;
  for(unsigned k=0; k<timeout; k++) {
    v = r;
    if ((v&mask) == val)
      return 1;
  }
  return 0;
}

class FmEplRegsList {
public:
  static FmEplRegs* epl() { return (FmEplRegs*)0; }

  FmEplRegsList(FmEplRegs* regs,unsigned mask)
  {
    _n = 0;
    for(unsigned j=1; j<32; j++) {
      if (mask & (1<<j)) {
	//	printf("RegsList port %d element %d regs %p\n",
	//	       _n, j, &regs[j-1]);
	_epls[_n++] = &regs[j-1];
      }
    }
  }
  void write(FmMReg* r,unsigned v) {
    for(unsigned j=0; j<_n; j++) {
      FmMReg* rr = (FmMReg*)_epls[j] - (FmMReg*)epl() + r;
      //      printf("Writing %p %x\n",rr,v);
      *rr = v;
    }
  }

  void read(FmMReg* r,unsigned* v) {
    for(unsigned j=0; j<_n; j++) {
      FmMReg* rr = (FmMReg*)_epls[j] - (FmMReg*)epl() + r;
      v[j] = *rr;
      //      printf("Read %p %x\n",rr,v[j]);
    }
  }

  unsigned n() const { return _n; }

private:
  FmEplRegs* _epls[32];
  unsigned _n;
};

unsigned FmBali::run_bist(FmPortMask pmask,
			   unsigned itest,
			   unsigned len,
			   unsigned* results)
{
  FmEplRegsList list(epl_phy,pmask);

  // Put all ports into test mode
  unsigned test_mode = 0x60 | (itest&8);
  list.write(&list.epl()->serdes_test_mode, test_mode);

  timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 20000;
  nanosleep(&ts,0);

  // check for sigdet
  list.read (reinterpret_cast<FmMReg*>(&list.epl()->serdes_status), results);
  for(unsigned j=0; j<list.n(); j++)
    if ((results[j]&0xF) != 0xF)
      printf("Element %d unable to attain sigdet (%d)\n",
	     j, results[j]);

  test_mode &= ~0x40; // disable PCS framer
  list.write(&list.epl()->serdes_test_mode, test_mode);
  test_mode |= itest&0xF; // enable BIST transmitter
  list.write(&list.epl()->serdes_test_mode, test_mode);
  test_mode = itest&0xF;  // enable BIST checker
  list.write(&list.epl()->serdes_test_mode, test_mode);

  ts.tv_sec = 0;
  ts.tv_nsec = len;

  // clear BIST errcnt
  list.read (&list.epl()->serdes_bist_err_cnt, results);

  nanosleep(&ts,0);

  // record BIST errcnt
  list.read (&list.epl()->serdes_bist_err_cnt, results);

  // reset mode to normal
  test_mode = 0x60;
  list.write(&list.epl()->serdes_test_mode, test_mode);

  return list.n();
}

const FmPortMibCounters& FmBali::port_mib(unsigned i) const
{
  return *reinterpret_cast<const FmPortMibCounters*>(&Stat0[i]);
}

FmPortMibCounters& FmBali::port_mib(unsigned i)
{
  return *reinterpret_cast<FmPortMibCounters*>(&Stat0[i]);
}
