#ifndef FmReg_hh
#define FmReg_hh

#define FM_DEV_BASE 0
#define FM_DEV_SIZE (0x1<<24)
//#define TAHOE_ADDR_REG 0x200  // synchronous transaction design
//#define TAHOE_DATA_REG 0x201
#define TAHOE_DATA_REG 0x200   // asynchronous transaction
#define TAHOE_CMND_REG 0x201
#define TAHOE_STAT_REG 0x202
#define TAHOE_FIFO_IS_REG 0x205
#define TAHOE_FIFO_IM_REG 0x204
#define TAHOE_FIFO_RX_REG 0x208
//
//
//

class FmReg {
public:
  static void init ();
  static int status();
  static void clear();
protected:
  FmReg() {}
  FmReg& operator=(const unsigned);
  operator unsigned() const;
private:
  ~FmReg() {}
  
  friend class FmMReg;
  friend class FmCReg;
  friend class FmSReg;

protected:
  unsigned _v;  // local copy is unused, but this variable reserves
                // space in the register map
};

#endif
