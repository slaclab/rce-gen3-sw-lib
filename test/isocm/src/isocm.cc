#include DAT_PUBLIC(tool, cpu, dcr.hh)

void isocm()
{
  uint32_t isInit = tool::cpu::DCR().read<0>();
  uint32_t isFill = tool::cpu::DCR().read<1>();
  uint32_t isArc  = tool::cpu::DCR().read<2>();
  uint32_t isCntl = tool::cpu::DCR().read<3>();

  printf("ISOCM: isInit = %08lx, isFill = %08lx, isArc = %08lx, isCntl = %08lx\n",
         isInit, isFill, isArc, isCntl);

  volatile uint32_t* p = (volatile uint32_t*)0xfffffffc;
  printf("1: 0xfffffffc = %08lx\n", *p);

  tool::cpu::DCR().write<3>(isCntl & ~0x20000000ul);

  printf("2: 0xfffffffc = %08lx\n", *p);
}
