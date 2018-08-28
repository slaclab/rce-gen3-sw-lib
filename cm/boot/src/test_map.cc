#include <stdio.h>
#include "FmBali.h"

#define PRINTP(arg) printf("%s [0x%08x]\n",#arg,((unsigned long long)(&b->arg)>>2))

int main()
{
  FmBali* b = 0;
  PRINTP(sys_cfg_1);
  PRINTP(sys_cfg_7);
  PRINTP(reserved);
  PRINTP(reserved_7);
  PRINTP(reserved207);
  PRINTP(reserved305);
  PRINTP(reserved_40006);
  PRINTP(reserved_4010C);
  PRINTP(reserved_40140);
  PRINTP(reserved_40179);
  PRINTP(reserved_41702);
  PRINTP(reserved_41718);
  PRINTP(reserved_41728);
  PRINTP(epl_phy[0]);
  PRINTP(epl_phy[1]);
  PRINTP(reserved_56400);
  PRINTP(reserved_60000);
  PRINTP(reserved_60072);
  PRINTP(reserved_80000);
  PRINTP(reserved_90620);
  PRINTP(reserved_91770);
  PRINTP(reserved_92690);
  PRINTP(reserved_92930);
  PRINTP(reserved_C0000);
  PRINTP(reserved_CC000);
  PRINTP(reserved_100001);
  PRINTP(reserved_100004);
  PRINTP(reserved_100008);
  PRINTP(reserved_10001A);
  PRINTP(reserved_100024);
  PRINTP(reserved_100034);
  PRINTP(reserved_100036);
  PRINTP(reserved_100059);
  PRINTP(reserved_100079);
  PRINTP(reserved_100081);
  PRINTP(reserved_100084);
  PRINTP(reserved_101019);
  PRINTP(reserved_101039);
  PRINTP(reserved_101059);
  PRINTP(reserved_101079);
  PRINTP(reserved_101099);
  PRINTP(reserved_101110);
  PRINTP(reserved_103000);
  PRINTP(reserved_104019);
  PRINTP(reserved_104030);
  PRINTP(reserved_106000);
  PRINTP(reserved_1064E2);
  PRINTP(reserved_1064E6);
  PRINTP(reserved_1065C0);
  PRINTP(reserved_10A300);
  PRINTP(reserved_10C000);
  PRINTP(reserved_120000);
  PRINTP(reserved_140000);
  PRINTP(reserved_17B801);
  PRINTP(reserved_199000);
  PRINTP(reserved_19E818);
  PRINTP(reserved_1A0380);
  PRINTP(reserved_1A0432);
  PRINTP(reserved_1A04B2);
  PRINTP(reserved_1A0532);
  return 0;
}
