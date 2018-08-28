
#include "control/playTask.hh"
#include "control/play.hh"
#include "ffs/IdFile.hh"
#include "time/Time.hh"

#include <stdio.h>
#include <string.h>
#include <time.h>

DtmDisplayTask::DtmDisplayTask(cm::ffs::IdFile& idf) :
  _display(ChrDisplay::instance())
{
}

void DtmDisplayTask::run()
{
  timespec ts;
  //    ts.tv_nsec = 500000000;
  ts.tv_nsec = 0;
  ts.tv_sec = 1;

  char buff[8];

  while(1) {
    nanosleep(&ts, 0);

    unsigned sec,nsec;
    tool::time::time(sec,nsec);

    sprintf(buff,"%04d",sec%10000);
    _display->writeStr(buff);

    printf("DtmDisplayTask : %s\n", buff);
  }
}
