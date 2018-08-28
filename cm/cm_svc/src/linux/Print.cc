
#include "cm_svc/Print.hh"
#include "logger/FormatMessage.hh"

#include <stdio.h>
#include <stdint.h>

void cm::service::print_init(const char*) {}

void cm::service::prints(const char* msg)
{
  printf(msg);
}

void cm::service::printv(const char* msgformat, ...)
{
  char bigMsg[256];
  uint32_t len;
  FORMAT_MESSAGE(msgformat, 256, bigMsg, len);
  printf(bigMsg);
}
