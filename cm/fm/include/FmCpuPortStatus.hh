#ifndef FmCpuPortStatus_hh
#define FmCpuPortStatus_hh

namespace Bali { class FmBali; };

class FmCpuPortStatus {
private:
  FmCpuPortStatus(unsigned v) : _v(v) {}
public:
  FmCpuPortStatus(const FmCpuPortStatus& c) : _v(c._v) {}

  ~FmCpuPortStatus() {}
  
  enum { TxRdy=1<<0,
	 RxRdy=1<<1,
	 RxEOT=1<<2,
	 IS   =1<<3 };

  unsigned txReady  (unsigned i) const { return (_v>>(i*4))&TxRdy; }
  unsigned rxReady  (unsigned i) const { return (_v>>(i*4))&RxRdy; }
  unsigned rxDone   (unsigned i) const { return (_v>>(i*4))&RxEOT; }
  unsigned intStatus(unsigned i) const { return (_v>>(i*4))&IS; }

  operator unsigned() const { return _v; }
private:
  unsigned _v;

  friend class FmTahoe;
  friend class Bali::FmBali;
};

#endif
