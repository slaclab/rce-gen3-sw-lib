// -*-Mode: C++;-*-
//                               Copyright 2014
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#include <stdlib.h>
#include <getopt.h>
#include <cstdio>

#include "dslutil/Base.h"
#include "dslutil/RebootClient.hh"
#include "dslutil/RebootBase.hh"

static struct option long_options[] = {
  { (char*)"help",    no_argument,       0, 'h' },
  { (char*)"ifname",  required_argument, 0, 'i' },
  { (char*)"type",    required_argument, 0, 't' },
  { (char*)"bitload", required_argument, 0, 'b' },
  { 0, 0, 0, 0 }
};

#define OPTIND_HELP           0
#define OPTIND_NAME           (OPTIND_HELP + 1)
#define OPTIND_OS             (OPTIND_NAME + 1)
#define OPTIND_LOADBIT        (OPTIND_OS   + 1)

// TODO: handle --ifaddr argument

void usage() {
  printf("usage: dsl_reboot [options] location\n");
  printf("\n");
  printf("          -i, --ifname   The name of a NIC which can see the shelf.\n");
  printf("          -t, --type     The destination OS type (linux/rtems/rescue/default)  Default is nochange\n");
  printf("          -b, --bitload  The bitfile load directive [0:no_load|1:load]  Default is nochange\n");  
  printf("          location   An RCE location.  May be wildcarded\n");
  printf("\n");
  printf("Broadcast 'reboot yourself' packet to a network.  All running\n");
  printf("   RCEs matching the wildcarded location on that network will\n");
  printf("   reboot to the target OS.  Default target OS is 'default',\n");
  printf("   meaning 'reboot to the default OS'.\n");
  
  exit(-1);
}


int main(int argc, char** argv)
{
  int c;
  char* ifname    = 0;
  char* os_s      = 0;
  char* loadbit_s = 0;

  while (true) {
    int option_index = 0;

    c = getopt_long(argc, argv, "hi:t:b:",
                    long_options, &option_index);
    if (-1 == c) break;

    switch(c) {
    case 0:
      if (OPTIND_HELP == option_index) 
        usage();
      else if (OPTIND_NAME == option_index)
        ifname    = optarg;
      else if (OPTIND_OS == option_index)
        os_s      = optarg;
      else if (OPTIND_LOADBIT == option_index)
        loadbit_s = optarg;
      break;
    case 'i':
      ifname    = optarg; break;
    case 't':
      os_s      = optarg; break;
    case 'b':
      loadbit_s = optarg; break;      
    case '?':  // 
    case 'h':
    default:
      usage();
      break;
    }
  }

  if ((argc - optind) < 1)       usage();

  char* address = argv[optind];
  if (!address)                  usage();

  service::dslutil::rebooter::Client* client;
  // TODO handle ifaddr also
  if (ifname)
    client = new service::dslutil::rebooter::Client(ifname);
  else
    client = new service::dslutil::rebooter::Client();

  unsigned os = OS_NOTSET;
  if(os_s != NULL)
    {
    if(!strcmp(os_s,"rtems"))       os = OS_RTEMS;
    else if(!strcmp(os_s,"linux"))  os = OS_LINUX;
    else if(!strcmp(os_s,"rescue")) os = OS_RESCUE;
    }
  
  unsigned loadbit = LOADBIT_NOTSET;
  if( loadbit_s != NULL)
    {
    if(!strcmp(loadbit_s,"1"))      loadbit = 1;
    else if(!strcmp(loadbit_s,"0")) loadbit = 0;
    }
  
  client->sendReboot(address, os, loadbit);

  return 0;

}
