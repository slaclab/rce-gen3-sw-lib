
#include "cm_svc/CmHw.hh"

#include <stdio.h>

using namespace cm::svc;

#if defined(ppc405) | defined(ppc440)

static char tag_buff[32];

const char* CmHw::get_version_tag()
{
  const unsigned VSNTAG = 0x210;
  unsigned v;
  asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(VSNTAG));
  if (v)
    sprintf(tag_buff,"Tag %c%02x-%02x-%02x\n",
	    char(v>>24),
	    (v>>16)&0xff,
	    (v>> 8)&0xff,
	    (v>> 0)&0xff);
  else
    sprintf(tag_buff,"No Tag");
  return tag_buff;
}


unsigned CmHw::get_version_time()
{
  const unsigned VSNTIME = 0x211;
  unsigned v;
  asm volatile("mfdcr %0, %1" : "=r"(v) : "i"(VSNTIME));
  return v;
}

#else

const char* CmHw::get_version_tag()
{
  return "cmCA9";
}


unsigned CmHw::get_version_time()
{
  return 0;
}

#endif
