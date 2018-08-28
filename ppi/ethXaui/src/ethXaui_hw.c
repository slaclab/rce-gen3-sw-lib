/**
 * @file
 *
 * @ingroup eth_xaui
 *
 * @brief Xaui Ethernet driver.
 */

/*
 *                              Copyright 2014
 *                                     by
 *                        The Board of Trustees of the
 *                      Leland Stanford Junior University.
 *
 */

#define __INSIDE_RTEMS_BSD_TCPIP_STACK__ 1
#define __BSD_VISIBLE                    1

#include <assert.h>

#include "network/BsdNet_Config.h"

#include "svt/Svt.h"    

#include "xaui/Xaui.h"

#include "ethernet.h"

int eth_xaui_init_mac(EthXaui* xaui)
{
  int      xs       = 0;

  PRINTK("%-40s: beg\n", __func__);

  PRINTK("%-40s: MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
         ((char*)xaui->arpcom.ac_enaddr)[0],
         ((char*)xaui->arpcom.ac_enaddr)[1],
         ((char*)xaui->arpcom.ac_enaddr)[2],
         ((char*)xaui->arpcom.ac_enaddr)[3],
         ((char*)xaui->arpcom.ac_enaddr)[4],
         ((char*)xaui->arpcom.ac_enaddr)[5]);

  /* Set up the hardware */
  xaui->linkSpeed = 10000;

  PRINTK("%-40s: end\n", __func__);
  return xs;
}

void eth_xaui_wait(EthXaui *xaui)
  {  
  while(1)
    Xaui_Wait(xaui->rxDevice);
  }
  
static const char BSDNET_ATTRS[] = "BSDNET_ATTRS";
static const char _attrs_error[] = "eth_xaui_setup_ethd lookup failed for symbol %s\n";
  
int eth_xaui_setup_ethd(EthXaui *xaui)
  {
  rtems_status_code sc;
  rtems_id tid;
  int error;

  BsdNet_Attributes *prefs = (BsdNet_Attributes*)Svt_Translate(BSDNET_ATTRS, SVT_SYS_TABLE);
  error = prefs ? SVT_SUCCESS: SVT_NOSUCH_SYMBOL;
  if(error != SVT_SUCCESS) 
    {
    printf(_attrs_error,BSDNET_ATTRS);    
    return error;
    }

  xaui->txDevice = Xaui_TxOpen(ETH_XAUI_TX_BUF_CNT);
  if(!xaui->txDevice ) return -1;  

  xaui->rxDevice = Xaui_RxOpen(ETH_XAUI_RX_BUF_CNT);
  if(!xaui->rxDevice ) return -1;  

  Xaui_RxBind(ETHERNET_TYPE_IPV4,xaui->rxDevice, (Xaui_RxProcessor)eth_xaui_process_rx, (void*)xaui);
  Xaui_RxBind(ETHERNET_TYPE_ARP, xaui->rxDevice, (Xaui_RxProcessor)eth_xaui_process_rx, (void*)xaui);
  
  sc = rtems_task_create(rtems_build_name('n','t','r','x'),
                         prefs->priority,
                         RTEMS_MINIMUM_STACK_SIZE * 10,
                         RTEMS_DEFAULT_ATTRIBUTES,
                         RTEMS_DEFAULT_MODES,
                         &tid);
  if (sc != RTEMS_SUCCESSFUL)
    rtems_panic("Can't create task: %s", rtems_status_text(sc));

  sc = rtems_task_start(tid, (rtems_task_entry)eth_xaui_wait, (int)xaui);
  if (sc != RTEMS_SUCCESSFUL)
    rtems_panic("Can't start task: %s", rtems_status_text(sc));
  
  return sc;
  }

void eth_xaui_start(EthXaui *xaui)
{
  PRINTK("%-40s: beg\n", __func__);

  eth_xaui_setup_ethd(xaui);

  PRINTK("%-40s: end\n", __func__);
}

void eth_xaui_stop(EthXaui *xaui)
{
  PRINTK("%-40s: beg\n", __func__);

  PRINTK("%-40s: end\n", __func__);
}

void eth_xaui_reset(EthXaui *xaui)
{
  PRINTK("%-40s: beg\n", __func__);

  PRINTK("%-40s: end\n", __func__);
}
