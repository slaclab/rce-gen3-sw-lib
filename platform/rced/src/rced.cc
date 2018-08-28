/*
**  Package:
**	
**
**  Abstract:
**      
**
**  Author:
**      Sergio Maldonado, SLAC (smaldona@slac.stanford.edu)
**
**  Creation Date:
**	    000 - August 18, 2013
**
**  Revision History:
**	    None.
**
** --
*/
 


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <net/if.h>

#include <arpa/inet.h>
#include <ifaddrs.h>

#include "concurrency/Thread.hh"
#include "dsl/init.hh"
#include "atca/init.hh"
#include "dslutil/id_init.hh"
#include "dslutil/reboot_init.hh"
#include "dslutil/update_init.hh"
#include "bsi/Bsi_Cfg.h"
#include "bsi/Bsi.h"
#include "map/Lookup.h"

#define DAEMON_NAME "rced"
#define LINK_WAIT       1000000       // link wait in usec - 1 sec

using namespace tool::concurrency;

// We need to provide a main thread to run the tests under Linux.
class Main: public Thread {
public:
  int wait_ip(void) {
  unsigned int _ipAddr = 0;
  struct ifaddrs *ifAddr, *ifa;
  
  if (getifaddrs(&ifAddr) == -1)
    {
    printf("NO IF ADDRS\n");
    return -1;
    }
  for (ifa = ifAddr; ifa; ifa = ifa->ifa_next)
    {
    if ( !ifa->ifa_addr )
      continue;
    if (AF_INET == ifa->ifa_addr->sa_family && strcmp(ifa->ifa_name,"lo"))
      {
      _ipAddr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
      if(_ipAddr)
        {
        syslog(LOG_NOTICE,"using ip address 0x%x on interface %s\n",_ipAddr,ifa->ifa_name);
        break;
        }
      }      
    }
    return _ipAddr;
  }
  
  Main(int argc, char **argv) : Thread(false) {}

  virtual void body() {  
  char hostname[1024];
  const char *group;
  uint32_t addr;
  uint32_t cluster;
  uint32_t bay;
  uint32_t element;
  int      ip;
  
  Bsi bsi = LookupBsi();    
  if (!bsi)
    {
      printf("failure mapping bsi\n");
      return;
    }

  hostname[1023] = '\0';
  group = BsiReadGroup(bsi, hostname);
  addr = BsiRead32(bsi,BSI_CLUSTER_ADDR_OFFSET);
  cluster = BSI_CLUSTER_FROM_CLUSTER_ADDR(addr);
  bay = BSI_BAY_FROM_CLUSTER_ADDR(addr);
  element = BSI_ELEMENT_FROM_CLUSTER_ADDR(addr);
  if(!(*hostname == '\0'))
    {
    sprintf((char *)hostname,"%s/%d/%d/%d",group,cluster,bay,element);
    sethostname(hostname, strlen(hostname));
    }

  //Set our Logging Mask and open the Log
  setlogmask(LOG_UPTO(LOG_NOTICE));
  openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
  
  while((ip = wait_ip()) == 0)
    {
    syslog(LOG_NOTICE,"waiting for ip address assignment...");
    usleep(LINK_WAIT);
    }
  
  /* Start the DSL server */
  service::dsl::initialize();

  /* Start the ATCA service */
  service::atca::initialize();

  /* Start the rebooter service */
  service::dslutil::rebooter::initialize();

  /* Start the identifier service */
  service::dslutil::identifier::initialize();

  /* Start the update service */
  service::dslutil::updater::initialize();
  
  BsiWrite32(bsi,BSI_BOOT_RESPONSE_OFFSET,BSI_BOOT_RESPONSE_SUCCESS);
  syslog(LOG_NOTICE,"set bsi status to boot success");
  }

  virtual ~Main() {}
};

int main(int argc, char **argv) {
  Main tmain(argc,argv);
  tmain.graduate();
  while(1){
  Thread::threadYield();
  };
  
  return 0;
}

