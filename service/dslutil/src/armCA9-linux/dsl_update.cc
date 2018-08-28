// -*-Mode: C++;-*-
//                               Copyright 2015
//                                    by
//                       The Board of Trustees of the
//                    Leland Stanford Junior University.
//                           All rights reserved.

#include <stdlib.h>
#include <getopt.h>
#include <cstdio>
#include <arpa/inet.h>

#include "dslutil/Base.h"
#include "dslutil/UpdateClient.hh"
#include "dslutil/UpdateBase.hh"

#define DEFAULT_TIMEOUT 1000 // timeout response wait in milliseconds

static struct option long_options[] = {
  {"help",    0, 0, 0},
  {"ifname",  1, 0, 0},
  {"id",      1, 0, 0},
  {"ip",      1, 0, 0},
  {"src",     1, 0, 0},
  {"file",    1, 0, 0},
  {"tmo",     1, 0, 0},
  {0,0,0,0}
};

struct sockaddr_in sa;

#define OPTIND_HELP           0
#define OPTIND_NAME           (OPTIND_HELP + 1)
#define OPTIND_ID             (OPTIND_NAME + 1)
#define OPTIND_IP             (OPTIND_ID   + 1)
#define OPTIND_SRC            (OPTIND_IP   + 1)
#define OPTIND_DST            (OPTIND_SRC  + 1)
#define OPTIND_TIMEOUT        (OPTIND_DST  + 1)

const char default_src[] = {"/srv/nfs4/dsl"};
const char default_dst[] = {"tools/update-target.sh"};

// TODO: handle --ifaddr argument

void usage() {
  printf("usage: dsl_update --ip ip [options] location\n");
  printf("\n");
  printf("          -v         Verbose output:  report things which don't respond.\n");  
  printf("          --ip       IP address of update server\n");  
  printf("          --src      Override default path of update source directory\n");
  printf("                     on server (max 31 chars)\n");
  printf("          --file     Override default path of update file relative to\n");
  printf("                     source directory (max 31 chars)\n");
  printf("          --ifname   The name of a NIC which can see the shelf.\n");
  printf("          --id       32-bit identifier for update transaction\n");
  printf("          --tmo      Response timeout in milliseconds\n");
  printf("          location   An RCE location.  May be wildcarded\n");
  printf("\n");
  printf("Broadcast 'update yourself' packet to a network.  All running\n");
  printf("   RCEs matching the wildcarded location on that network will\n");
  printf("   update target filesystem by executing ip:src/file.\n");
  
  exit(-1);
}


int main(int argc, char** argv)
{
  int c;
  unsigned id      = 0;
  unsigned ip      = 0;
  unsigned timeout = DEFAULT_TIMEOUT;
  char* ifname     = 0;
  const char* src        = &default_src[0];
  const char* dst        = &default_dst[0];
  bool verbose = false;

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
        ifname = optarg;
      else if (OPTIND_ID == option_index)
        id = strtoul(optarg,0,0);
      else if (OPTIND_TIMEOUT == option_index)
        timeout = strtoul(optarg,0,0);
      else if (OPTIND_IP == option_index)
        {
        if(!inet_pton(AF_INET, optarg, &(sa.sin_addr)))
          {
          printf("Invalid ip address format %s\n",optarg);
          return -1;
          }
       ip = sa.sin_addr.s_addr;
        }
      else if (OPTIND_SRC == option_index)
        {
        if(strlen(optarg) >= MAX_PATH_LEN)
          {
          printf("Source path exceeds max char length of %d\n",MAX_PATH_LEN-1);
          usage();
          break;
          }
        else
          src = optarg;
        }
      else if (OPTIND_DST == option_index)
        {
        if(strlen(optarg) >= MAX_PATH_LEN)
          {
          printf("File path exceeds max char length of %d\n",MAX_PATH_LEN-1);
          usage();
          break;
          }
        else
          dst = optarg;
        }
      break;
      
    case 'v':
      verbose=true;
      break;
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

  if(!ip)
    {
    printf("Missing required IP options\n");
    usage();    
    }

  service::dslutil::updater::Client* client;
  service::dslutil::updater::Listener* listener;
  
  // TODO handle ifaddr also
  
  if (ifname) {
    listener = new service::dslutil::updater::Listener(ifname);
    client = new service::dslutil::updater::Client(listener, ifname);
  }
  else {
    listener = new service::dslutil::updater::Listener();
    client = new service::dslutil::updater::Client(listener);
  }

  client->sendUpdate(address, id, ip, src, dst);
  
  tool::concurrency::Thread::threadSleep(timeout);
  listener->report(NULL,verbose);

  return 0;

}
