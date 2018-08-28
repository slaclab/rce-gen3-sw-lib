// -*-Mode: C++;-*-
/*!
*
* @brief   Functions for handling times
*
* @author  Anonymous -- REG/DRD - (someone@slac.stanford.edu)
*
* @date    March 29, 2011 -- Created
*
* $Revision: 3125 $
*
* @verbatim
*                               Copyright 2011
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#include "time/Time.hh"

#include <time.h>
#include <stdio.h>

unsigned long long tool::time::time()
{
  timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  unsigned long long nano = now.tv_nsec;
  nano += now.tv_sec*1000000000ULL;
  return nano;
}

void tool::time::time(unsigned& sec, unsigned& nsec)
{
  timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  sec = now.tv_sec;
  nsec = now.tv_nsec;
}

void tool::time::settime(unsigned sec, unsigned nsec)
{
  timespec now;
  now.tv_sec = sec;
  now.tv_nsec = nsec;
  clock_settime(CLOCK_REALTIME, &now);
}

const char* tool::time::asctime(unsigned sec, unsigned nsec, char* buf)
{
  unsigned usec = nsec / 1000;
  time_t now = sec;
  struct tm result;
  struct tm* t = localtime_r(&now, &result);
  int sz = strftime(buf, 128, "%Y/%m/%d %H:%M:%S", t);
  sprintf(buf+sz, ".%06d", usec);
  return buf;
}

const char* tool::time::asctime(unsigned long long t, char* buf)
{
  unsigned sec  = t / 1000000000;
  unsigned nsec = t % 1000000000;
  asctime(sec, nsec, buf);
  return buf;
}
