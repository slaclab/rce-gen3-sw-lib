// -*-Mode: C++;-*-
//                               Copyright 2014
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <cstdio>

#include "dslutil/Base.h"
#include "dslutil/IdClient.hh"
#include "dslutil/IdBase.hh"

#include "concurrency/Sleep.hh"
#include "concurrency/Thread.hh"

static struct option long_options[] = {
  {"help",   0, 0, 0},
  {"ifname", 1, 0, 0},
  {0,0,0,0}
};

#define OPTIND_HELP           0
#define OPTIND_NAME           (OPTIND_HELP + 1)
// TODO: handle --ifaddr argument

void usage() {
  printf("usage: dsl_identify [-v] [--ifname ifname] location\n");
  printf("\n");
  printf("          -v         Verbose output:  report things which don't respond.\n");
  printf("          --ifname   The name of a NIC which can see the shelf.\n");
  printf("          location   An RCE location.  May be wildcarded\n");
  printf("\n");
  printf("Broadcasts 'identify yourself' packet to a network. All\n");
  printf("   running RCEs matching the wildcarded location on that network\n");
  printf("   will respond.\n");
  printf("\n");
 
  exit(-1);
}


int main(int argc, char** argv)
{
  int c;
  char* ifname   = 0;
  bool verbose = false;
  // TODO: handle --ifaddr argument

  while (true) {
    int option_index = 0;

    c = getopt_long(argc, argv, "hv",
                    long_options, &option_index);
    if (-1 == c) break;

    switch(c) {
    case 0:
      if (OPTIND_HELP == option_index) 
        usage();
      else if (OPTIND_NAME == option_index)
        ifname   = optarg;
      break;
    case 'v':
      verbose=true;
      break;
    case '?':
    case 'h':
    default:
      usage();
      break;
    }
  }

  if ((argc - optind) < 1)       usage();

  char* address = argv[optind];
  if (!address)                  usage();

  service::dslutil::identifier::Client* client;
  service::dslutil::identifier::Listener* listener;
  // TODO handle ifaddr also
  
  if (ifname) {
    listener = new service::dslutil::identifier::Listener(ifname);
    client = new service::dslutil::identifier::Client(listener, ifname);
  }
  else {
    listener = new service::dslutil::identifier::Listener();
    client = new service::dslutil::identifier::Client(listener);
  }

  client->identify(address);

  tool::concurrency::Thread::threadSleep(1000);
  listener->report(verbose);

  return 0;

}
