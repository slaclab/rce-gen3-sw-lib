
#include "control/DisplayTask.hh"
#include "control/ChrDisplay.hh"
#include "ffs/IdFile.hh"
#include "fm/FmTahoeMgr.hh"
#include "fm/FmTahoe.hh"
#include "fm/SwmMibCounter.hh"
#include "phy/BxPhy.hh"

#include <stdio.h>
#include <string.h>
#include <time.h>

DisplayTask::DisplayTask(cm::ffs::IdFile& idf,
			 FmTahoeMgr& fm0,
			 FmTahoeMgr& fm1) :
  _fm0    (fm0),
  _fm1    (fm1),
  _hasBx  ( strcmp(idf.version(),"C00") ),
  _baseMap(32,25),
  _fabrMap(32,25),
  _display(ChrDisplay::instance())
{
  if (!strcmp(idf.version(),"C00")) {
    //  Fabric switch mapping
    _fabrMap.add( 0 , 17 );  // Zone 2  Slot 2
    _fabrMap.add( 1 , 21 );  // Zone 2  Slot 3
    _fabrMap.add( 2 , 19 );  // Zone 2  Slot 4
    _fabrMap.add( 3 , 13 );
    _fabrMap.add( 4 , 15 );
    _fabrMap.add( 5 , 11 );
    _fabrMap.add( 6 ,  9 );
    _fabrMap.add( 7 ,  7 );
    _fabrMap.add( 8 ,  5 );
    _fabrMap.add( 9 ,  3 );
    _fabrMap.add( 10,  1 );
    _fabrMap.add( 11,  2 );
    _fabrMap.add( 12,  4 );
    _fabrMap.add( 13,  6 );
    _fabrMap.add( 14,  8 );  // Zone 2  Slot 16
    _fabrMap.add( 15, 16 );  // Zone 3  RTM Channel A
    _fabrMap.add( 16, 14 );  // Zone 3  RTM Channel B
    _fabrMap.add( 17, 10 );
    _fabrMap.add( 18, 12 );
    _fabrMap.add( 19, 18 );  // Zone 3  Hub Interconnect
    _fabrMap.add( 20, 22 );
    _fabrMap.add( 21, 20 );
    _fabrMap.add( 23, 24 );  // Front panel BaseT auxilliary channel
    _fabrMap.add( 24, 23 );  // Front panel XFP   auxilliary channel
    //  Base switch mapping
    _baseMap.add( 0 ,  8 );  // Zone 2  Slot 2
    _baseMap.add( 1 ,  6 );  // Zone 2  Slot 3
    _baseMap.add( 2 ,  4 );  // Zone 2  Slot 4
    _baseMap.add( 3 ,  2 );
    _baseMap.add( 4 ,  1 );
    _baseMap.add( 5 ,  3 );
    _baseMap.add( 6 ,  5 );
    _baseMap.add( 7 ,  7 );
    _baseMap.add( 8 ,  9 );
    _baseMap.add( 9 , 11 );
    _baseMap.add( 10, 15 );
    _baseMap.add( 11, 13 );
    _baseMap.add( 12, 19 );
    _baseMap.add( 13, 21 );
    _baseMap.add( 14, 17 );  // Zone 2  Slot 16
    _baseMap.add( 15, 16 );  // Zone 3  RTM Channel A
    _baseMap.add( 16, 14 );  // Zone 3  RTM Channel B
    _baseMap.add( 17, 10 );
    _baseMap.add( 18, 12 );
    _baseMap.add( 19, 18 );  // Zone 3  Hub Interconnect
    _baseMap.add( 20, 22 );
    _baseMap.add( 21, 20 );
    _baseMap.add( 23, 24 );  // Front panel BaseT auxilliary channel
    _baseMap.add( 24, 23 );  // Front panel XFP   auxilliary channel
  }
  else {
    //  Fabric switch mapping
    _fabrMap.add( 0 , 17 );  // Zone 2  Slot 2
    _fabrMap.add( 1 , 21 );  // Zone 2  Slot 3
    _fabrMap.add( 2 , 19 );  // Zone 2  Slot 4
    _fabrMap.add( 3 , 13 );
    _fabrMap.add( 4 , 15 );
    _fabrMap.add( 5 , 11 );
    _fabrMap.add( 6 ,  9 );
    _fabrMap.add( 7 ,  7 );
    _fabrMap.add( 8 ,  5 );
    _fabrMap.add( 9 ,  3 );
    _fabrMap.add( 10,  1 );
    _fabrMap.add( 11,  2 );
    _fabrMap.add( 12,  4 );
    _fabrMap.add( 14, 16 );  // Zone 3  RTM Channel A
    _fabrMap.add( 15, 14 );  // Zone 3  RTM Channel B
    _fabrMap.add( 16, 10 );
    _fabrMap.add( 17, 12 );
    _fabrMap.add( 18,  8 );
    _fabrMap.add( 19, 18 );  // Zone 3  Hub Interconnect
    _fabrMap.add( 20, 22 );
    _fabrMap.add( 21, 20 );
    _fabrMap.add( 22,  6 );
    _fabrMap.add( 23, 24 );  // Front panel BaseT auxilliary channel
    _fabrMap.add( 24, 23 );  // Front panel XFP   auxilliary channel
    //  Base switch mapping
    _baseMap.add( 0 ,  8 );  // Zone 2  Slot 2
    _baseMap.add( 1 ,  6 );  // Zone 2  Slot 3
    _baseMap.add( 2 ,  4 );  // Zone 2  Slot 4
    _baseMap.add( 3 ,  2 );
    _baseMap.add( 4 , 19 );
    _baseMap.add( 5 ,  9 );
    _baseMap.add( 6 , 11 );
    _baseMap.add( 7 , 13 );
    _baseMap.add( 8 , 15 );
    _baseMap.add( 9 ,  1 );
    _baseMap.add( 10,  3 );
    _baseMap.add( 11,  5 );
    _baseMap.add( 12,  7 );
    _baseMap.add( 13, 21 );  // Zone 2  Slot 2 extension
    _baseMap.add( 14, 14 );  // Zone 3  RTM Channel A
    _baseMap.add( 15, 10 );  // Zone 3  RTM Chxannel B
    _baseMap.add( 16, 12 );
    _baseMap.add( 20, 18 );  // Zone 3  Hub Interconnect
    _baseMap.add( 21, 22 );
    _baseMap.add( 23, 24 );  // Front panel BaseT auxilliary channel
    _baseMap.add( 24, 23 );  // Front panel XFP   auxilliary channel
  }
}

void DisplayTask::run()
{
  timespec ts;
  //    ts.tv_nsec = 500000000;
  ts.tv_nsec = 0;
  ts.tv_sec = 2;
  unsigned bmp[4],bmpT[4];
  unsigned portsUp[2];
  memset(portsUp,0,2*sizeof(unsigned));

  //    unsigned prescale=1000, count=prescale;
  unsigned sw = 0;

  while(1) {
    nanosleep(&ts, 0);

    sw = 1-sw;

    unsigned long long stat;
    if (sw==0)
      {
        stat = SwmMibCounter(_fm0.dev().port_mib[24].rxGoodOctets);
	bmp[3] = _display->charToBmp('I');
      }
    else
      {
        stat = SwmMibCounter(_fm0.dev().port_mib[ 0].rxGoodOctets);
	bmp[3] = _display->charToBmp('O');
      }

    bmp[0]  = stat & 0xFFFFFF;
    bmpT[0] = _display->transform(bmp[0], ChrDisplay::Rotate270);

    bmp[1]  = (stat>>24) & 0xFFFFFF;
    bmpT[1] = _display->transform(bmp[1], ChrDisplay::Rotate270);

    bmp[2]  = (stat>>48) & 0xFFFFFF;
    bmpT[2] = _display->transform(bmp[2], ChrDisplay::Rotate270);

    bmpT[3] = _display->transform(bmp[3], ChrDisplay::Rotate90);

    _display->writeBmp(bmpT);

    printf("DisplayTask : %c : %llx\n", sw ? 'O':'I', stat);
  }
}
