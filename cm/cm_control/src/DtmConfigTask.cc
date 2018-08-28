
#include "control/DtmConfigTask.hh"
#include "fm/FmTahoeMgr.hh"
#include "fm/FmTahoe.hh"

#include <stdio.h>
#include <string.h>
#include <time.h>

DtmConfigTask::DtmConfigTask(FmTahoeMgr& fm) :
  _fm    (fm)
{
}

void DtmConfigTask::run()
{
}
