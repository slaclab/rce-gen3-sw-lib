#include "bldInfo.h"

#include "rtems/devel.hh"
#include "exception/Exception.hh"
#include "debug/Debug.hh"
#include "debug/Print.hh"
#include "fci/Directory.hh"
#include "fci/Controller.hh"
#include "fci/impl/BootParams.hh"
#include "fci/impl/BlockList.hh"
#include "ffs/IdFile.hh"

extern "C" {
#include <rtems/rtems_bsdnet.h> // needed before any other bsdnet include
}
#include <stdio.h>
#include <exception>

unsigned doFormat=0xbadcafe;
unsigned lowerMacAddrByte=0xdeadbeef;

static const unsigned MaxPrint=20;
struct rtems_bsdnet_config rtems_bsdnet_config = {0}; // set ifconfig to 0

// To format a board download the executable with gdb/xmd, and then
// set doFormat to 1.
extern "C" void init_executive() {
  try {
    service::debug::clearMessages();
    service::debug::printv("brdinit: SVN %s, built %s",
                           cm::control::bldRevision(),
                           cm::control::bldDate());
    unsigned nprint=0;
    if (doFormat == 1) {
      doFormat=0;
      service::fci::Controller cntrl;
      service::debug::printv("ffs:controller");
      service::fci::Directory dir(cntrl, service::fci::Directory::ForFormat);
      service::debug::printv("ffs:directory");
      dir.format();
      service::debug::printv("ffs:formatted");
      unsigned bad_blocks = dir.gbl().numBad();
      service::debug::printv("ffs:numbad");
      service::debug::printv("found %d bad blocks out of %d",
                             bad_blocks, service::fci::Params::Blocks);
      for (unsigned j=0; j<service::fci::Params::Blocks; j++) {
        if (nprint >= MaxPrint) {
          service::debug::printv("Too many bad blocks. ",
                                 "Suppressing further bad block printout.");
          break;

        }
        if (!dir.gbl().isGood(j)) {
          service::debug::printv("[%d] block %d is not good", nprint++, j);
        }
      }
      dir.write();
    }
    if (lowerMacAddrByte <= 0xffff) {
      char vsn  [32];
      char serno[32];
      char macB [32];
      char macF [32];
      unsigned ubrd = (lowerMacAddrByte>>12)&0xf;
      unsigned uvsn = (lowerMacAddrByte>> 8)&0xf;
      unsigned user = (lowerMacAddrByte>> 0)&0xff;
      sprintf(vsn  ,"%c%02d",ubrd==0 ? 'C':'D',uvsn);
      sprintf(serno,"%03d",user&0x1f);
      sprintf(macB ,"08:00:56:00:40:%02x",user);
      sprintf(macF ,"08:00:56:00:41:%02x",user);

      cm::ffs::IdFile* idf = new cm::ffs::IdFile("CM",vsn,serno,macB,macF);
      service::debug::printv("Wrote IDFILE : %s %s %s %s %s\n",
                             idf->name(), idf->version(), idf->serialNumber(),
                             idf->macBase(), idf->macFabric());
      delete idf;
    }
    service::debug::printv("brdinit end");
  } catch (tool::exception::Exception& e) {
    service::debug::printv("*** rce exception %s", e.what());
  } catch (std::exception& e) {
    service::debug::printv("*** c++ exception %s", e.what());
  }
}
