static volatile int mode = 1;

#include <stdio.h>


#include "fm/FmTahoe.hh"

int myusleep(int arg)
{
  int i=0;
  while (i<20*arg)
    i++;
  return i;
}

#define alloc_test(s) { \
  printf("Alloc 0x%x at %p\n",s,new char[s]); \
}

int main()
{
  unsigned v;
  printf("===========\n");
  printf("Hello world\n");

  printf("Sizeof FmTahoe is 0x%x\n",sizeof(FmTahoe));
  printf("Sizeof FmMReg is 0x%x\n",sizeof(FmMReg));
  printf("Sizeof FmCReg is 0x%x\n",sizeof(FmCReg));
  printf("Sizeof FmSReg is 0x%x\n",sizeof(FmSReg));
  /*
  alloc_test(0x100);
  alloc_test(0x400);
  alloc_test(0x1000);
  alloc_test(0x4000);
  alloc_test(0x10000);
  alloc_test(0x40000);
  alloc_test(0x100000);
  alloc_test(0x400000);
  */
  int devID = 0;
  printf("devID at %p\n",&devID);

  printf("Mode at %p\n",&mode);
  volatile int old_mode=-1;

  FmTahoe* t = 0;

  do {

    if (mode!=old_mode) {
      old_mode = mode;
      printf("Mode = %d\n",mode);
    }

    switch(mode) {
    case 0: //
      myusleep(200000);
      if (t) t->destroy();
      t = new(devID) FmTahoe;
      break;
    case 1: // toggle reset
      myusleep(200000);
      printf("Assert reset\n");
      t->resetChip();
      myusleep(200000);
      printf("Deassert reset\n");
      t->releaseChip();
      break;
    case 2: // read status
      myusleep(200000);
      v = t->cpuPortStatus();
      printf("Read status %d\n",v);
      break;
    case 3: // write to FM_Addr 0x315 *4
      myusleep(200000);
      t->pll_fh_ctrl = 0xf;
      break;
    case 4: // read from FM_Addr 0x316 *4
      myusleep(200000);
      v = t->pll_fh_stat;
      printf("Read pll_fh_stat %d\n",v);
      break;
    default:
      printf("Unknown mode\n");
      myusleep(1000000);
      break;
    }

  } while(1);
  printf("Done\n");
  return 1;
}
