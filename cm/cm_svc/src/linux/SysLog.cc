
#include "cm_svc/Print.hh"
#include "logger/FormatMessage.hh"

#include <syslog.h>
#include <stdint.h>

void cm::service::print_init(const char* p)
{
  openlog(p,LOG_NOWAIT|LOG_PID,LOG_USER);
}

void cm::service::prints(const char* msg)
{
  syslog(LOG_NOTICE,msg);
}

void cm::service::printv(const char* msgformat, ...)
{
  char bigMsg[256];
  uint32_t len;
  FORMAT_MESSAGE(msgformat, 256, bigMsg, len);
  syslog(LOG_NOTICE,bigMsg);
}
