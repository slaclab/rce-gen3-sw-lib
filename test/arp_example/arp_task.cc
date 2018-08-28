#include <stdio.h>

#include "task/Task.h"
#include "debug/print.h"
#include "net/Interface.hh"
#include "net/MacAddress.hh"

#include "PseudoARP.hh"

#define PRINT dbg_printv

extern examples::PseudoARP* common_pseudoarp_instance;

uint64_t _test;

extern "C"
void Task_Start(int argc, const char** argv)
{

  // Grab my known mac address for comparison
  service::net::Interface interface;
  uint64_t mac        = interface.hwAddr() << 16;
  service::net::MacAddress mine(mac);

  PRINT("Looking up my MAC address.\n");
  uint64_t mac_lookup = common_pseudoarp_instance->lookup(interface.ipAddr());
  service::net::MacAddress found(mac_lookup);

  
  if ( mac != mac_lookup ) {
    PRINT("ERROR:  My interface's MAC address doesn't match!\n");
    PRINT("   Me:%s  Lookup:%s\n", mine.string(), found.string());
  } else {
    PRINT("Found my MAC address: %s == %s\n", mine.string(), found.string());
  }

}

extern "C"
void Task_Rundown()
{


}


