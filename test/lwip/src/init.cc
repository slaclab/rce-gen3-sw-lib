// Define USE_DHCP to use DHCP, else provide a hard-coded IP address below
#define USE_DHCP

#include "datCode.hh"
#include DAT_PUBLIC( test,          lwip,        init.h)
#include DAT_PUBLIC( test,          lwip,        init.hh)
#include DAT_PRIVATE(test,          lwip,        include/netif/xemacpsif.h)


#ifdef USE_DHCP
#include "lwip/dhcp.h"
#endif

#include "lwip/tcpip.h"

#define THREAD_STACKSIZE (64*1024)

#ifdef XPAR_XEMACPS_0_BASEADDR
# ifdef XPAR_XEMACPS_1_BASEADDR
#  if XLWIP_CONFIG_EMAC_NUMBER == 0
#   define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR
#  else
#   define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_1_BASEADDR
#  endif
# else
#  define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR
# endif
#elif XPAR_XEMACPS_1_BASEADDR
# ifdef XPAR_XEMACPS_0_BASEADDR
#  if XLWIP_CONFIG_EMAC_NUMBER == 0
#   define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_0_BASEADDR
#  else
#   define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_1_BASEADDR
#  endif
# else
#  define PLATFORM_EMAC_BASEADDR XPAR_XEMACPS_1_BASEADDR
# endif
#endif

#ifndef PLATFORM_EMAC_BASEADDR
# error "No Ethernet Interface Selected"
#endif


#ifdef __cplusplus
extern "C" {
#endif

static void lwip_initialize_interface(void)
{
  static struct netif netif;
  struct ip_addr      addr, mask, gate;
  unsigned char       mac[6] = {0x08, 0x00, 0x56, 0x00, 0x43, 0x0f};

  // Set up a network interface.
#ifndef USE_DHCP
  IP4_ADDR(&addr, 172,  21,   7,  80);  // IP address
  IP4_ADDR(&mask, 255, 255, 255, 192);  // netmask
  IP4_ADDR(&gate, 172,  21,   7,  78);  // gateway
#else
  IP4_ADDR(&addr, 0, 0, 0, 0);          // IP address
  IP4_ADDR(&mask, 0, 0, 0, 0);          // netmask
  IP4_ADDR(&gate, 0, 0, 0, 0);          // gateway
#endif
  netif.num = 0;                        // Interface to use

  /* Add network interface to the netif_list, and set it as default */
  xemac_add(&netif, &addr, &mask, &gate, mac, PLATFORM_EMAC_BASEADDR);
  netif_set_default(&netif);

  /* Specify that the network if is up */
  netif_set_up(&netif);

#ifdef USE_DHCP
  dhcp_start(&netif);
#endif

  /* Start packet receive thread - required for lwIP operation */
  sys_thread_new("Xinp", (void(*)(void*))xemacif_input_thread,
                 &netif,
                 THREAD_STACKSIZE,
                 DEFAULT_THREAD_PRIO);
}

static void tcpip_init_done(void *arg)
{
  sys_sem_t *sem;
  sem = (sys_sem_t *)arg;

  lwip_initialize_interface();
  printf("\"Network\" initialized!\n");

  sys_sem_signal(sem);
}

void TEST_LWIP_initialize(void)
{
  printf("\n\"Network\" initializing!\n");

  sys_sem_t sem;
  if(sys_sem_new(&sem, 0) != ERR_OK)
    LWIP_ASSERT("Failed to create semaphore", 0);

  tcpip_init(tcpip_init_done, &sem);
  sys_sem_wait(&sem);
  printf("TCP/IP initialized.\n");
}

#ifdef __cplusplus
}
#endif

namespace test {

  namespace lwip {

    void initialize() throw (tool::exception::Error)
    {
      TEST_LWIP_initialize();
    }

  } // lwip

} // test
