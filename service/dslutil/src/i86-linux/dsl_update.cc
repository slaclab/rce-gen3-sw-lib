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
#include "cob_ipmc/COB_ipmc.h"

#define DEFAULT_TIMEOUT 1000 // timeout response wait in milliseconds

static struct option long_options[] = {
  {"help",    0, 0, 0},
  {"ifname",  1, 0, 0},
  {"id",      1, 0, 0},
  {"ip",      1, 0, 0},
  {"sm",      1, 0, 0},
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
#define OPTIND_SM             (OPTIND_IP   + 1)
#define OPTIND_SRC            (OPTIND_SM   + 1)
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
  printf("          --sm       Hostname or IP address of shelf manager\n");    
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

static uint16_t parse_slot(const char* slot_s)
  {
  char* endptr;                                    
  int slot;                                        

  if(NULL == slot_s)                               
    {                                              
    return UNDEFINED;                                 
    }                                              
                                                   
  slot = strtol(slot_s, &endptr, 0);               
  if(slot_s == endptr)                             
    {                                                
      if(0 == strcmp(slot_s, "ALL"))                
      {                                             
      return UNDEFINED;
      }                                             
      else if(0 == strcmp(slot_s, "DPM"))           
      {                                             
      return UNDEFINED;
      }                                             
      else if(0 == strcmp(slot_s, "DTM"))           
      {                                             
      return UNDEFINED;
      }                                             
      else                                          
      {                                             
        printf("ERROR - Invalid SLOT %s\n", slot_s);
        return 0x0000;
      }                                             
    }                                                
  else if(slot<1 || slot > 16)                     
    {                                              
    printf("ERROR - Invalid SLOT %s\n", slot_s);   
    return 0x0000;                                 
    }                                              
  else                                             
    {                                                
    return 1<<(slot-1);                           
    }                                                

  return 0x0000;                                   
}

COB_slot_rce_status *check_sm(char *address, char *shelf_mgr, void *buf)
  {
  char *name;
  char temp_input[80];
  char *slot_s;
  unsigned i;
  
  uint16_t slot_mask;

  strncpy(temp_input, address, 80);
  name = strtok(temp_input, "/");

  if((slot_s = strtok(NULL, "/")))
  {
     for(i=0; i<strlen(slot_s); ++i)
     {
	slot_s[i] = toupper(slot_s[i]);
     }
  }

  slot_mask = parse_slot(slot_s);
  if(slot_mask == UNDEFINED)
    slot_mask = SLOT_ALL_WC;
  else
    slot_mask = strtoul(slot_s,0,0);

  return COB_get_slot_rce_status(shelf_mgr,slot_mask,buf);

  }

int main(int argc, char** argv)
{
  int i,c;
  unsigned id      = 0;
  unsigned ip      = 0;
  unsigned timeout = DEFAULT_TIMEOUT;
  char* ifname     = 0;
  char* sm         = 0;
  const char* src        = &default_src[0];
  const char* dst        = &default_dst[0];
  COB_slot_rce_status exp_status;
  COB_slot_rce_status *slot_status;
  char *buf[sizeof(COB_slot_rce_status)];
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
      else if (OPTIND_SM == option_index)
        sm = optarg;
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

  if(sm)
    {
    listener->report(&exp_status,verbose);
    if(exp_status.slot_valid_mask)
      printf("Waiting for update completion");
    while(exp_status.slot_valid_mask)
      {
      tool::concurrency::Thread::threadSleep(1000);
      printf(".");
      fflush(stdout);      
      bzero(buf,sizeof(COB_slot_rce_status));
      slot_status = check_sm(address, sm, buf);
      if(slot_status)
        {
        if((slot_status->slot_valid_mask & exp_status.slot_valid_mask) == exp_status.slot_valid_mask)
          {
          for(i=0;i<slot_status->nslots;i++)
            {
            if((1<<i) & exp_status.slot_valid_mask)
              {
              if((slot_status->slot[i].running & exp_status.slot[i].running) == exp_status.slot[i].running)
                {
                if(!exp_status.slot[i].present)
                  printf("\nSlot %d : Update state not reported. Aborting wait!",i+1);
                else
                  printf("\nSlot %d : Update complete!",i+1);
                exp_status.slot_valid_mask &= ~(1<<i);
                }
              else
                {
                exp_status.slot[i].present |= 1;
                }
              }
            }
          }
        }
      else
        {
        printf("\nNo status report from shelf manager %s\n",sm);
        break;
        }
      }
    printf("\n");
    }
  else
    listener->report(NULL,verbose);

  return 0;

}
