

#include "../FmBali_init.h"
#include "../FmBali.h"
#include "../FmEplHandler.h"

#include <malloc.h>

/* control verbosity here */
#define DEBUG_PRINTF(...)
//#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
//#define PROFILE

/* u-boot specific routine */
extern void udelay(unsigned long);
extern int printf(const char* fmt,...);
extern unsigned long get_timer(unsigned long);

static const unsigned _testPort = 5;

static int  boot_status(plx*,FmBali*);
static void releasePort(plx*,FmBali*,int);


/*
**  Boot Bali switch
**    Do not setup interrupts, MAC table management, or LCI port
**    (will do with a daemon)
*/
int FmBali_init(plx* _plx, FmConfig* cfg)
{
  /*
  **  Initialize PLL
  **  Wait for PLL lock
  **  Write 0 to SOFT_RESET
  **  Enable MSB and EPL0 by setting ELP_PORT_CTRL[0].ResetN=1 and ELP_PORT_CTRL.InitializeN=1
  **  Enable FFU by setting SYS_CFG_8.0=1
  **  Write "Repair asynchronous RAM" to BOOT_CTRL::Command
  **  Wait for BOOT_STATUS::CommandDone=1
  **  Write "Repair synchronous RAM" to BOOT_CTRL::Command
  **  Wait for BOOT_STATUS::CommandDone=1
  **  Program FFU_INIT_SLICE
  **  Wait for BOOT_STATUS::CommandDone=1
  **  Deassert FH reset
  **  Write "Initialize scheduler" to BOOT_CTRL::Command
  **  Wait for BOOT_STATUS::CommandDone=1
  **  Write "Execute memory initialization" to BOOT_CTRL::Command
  **  Wait for BOOT_STATUS::CommandDone=1
  **  Disable FFU
  */
  volatile unsigned v;
  unsigned j,k;
  FmBali* this = 0;

#ifdef PROFILE
  unsigned long time = get_timer(0);
#endif

   /*  Read vital product data register */
   {
     v = PLX_READ(vpd);
     DEBUG_PRINTF("FmBali_init: Read vpd %08x\n",v);
     if (((v >> 12) & 0xffff) != 0xae19) {
       DEBUG_PRINTF("FmBali_init: Incorrect vpd %08x (0xae19xxxx)\n",v);
       return 0;
     }
   }

#ifdef PROFILE
   time = get_timer(time);
   printf("FmBali_init boot %lu ms\n", time);
#endif

   DEBUG_PRINTF("FmBali_init:  Async memory repair\n");
   PLX_WRITE(boot_ctrl,1);
   if (!boot_status(_plx,this)) return 0;

   DEBUG_PRINTF("FmBali_init:  Initialize the scheduler\n");
   PLX_WRITE(boot_ctrl,0);
   if (!boot_status(_plx,this)) return 0;

   DEBUG_PRINTF("FmBali_init:  Initialize the frame handler PLL\n");
   v = PLX_READ(pll_fh_stat);
   v = PLX_READ(pll_fh_ctrl);
   v &= ~2;
   PLX_WRITE(pll_fh_ctrl,v);

   { unsigned k=1024;
     do {
       v = PLX_READ(pll_fh_stat);
     } while( v==0 && --k );
     if (!k) {
       printf("FAIL: FmBali_init pll_fh_stat poll timeout (%x)\n",v);
       return 0;
     }
   }

   DEBUG_PRINTF("FmBali_init:  Bring all modules out of reset\n");
   PLX_WRITE(soft_reset,0);
   v = PLX_READ(soft_reset);
              
   DEBUG_PRINTF("FmBali_init:  Bring EPL0 out of reset\n");
   releasePort(_plx,this,0);

   DEBUG_PRINTF("FmBali_init:  Enable the FFU\n");
   v = PLX_READ(sys_cfg_8);
   v |= 1;
   PLX_WRITE(sys_cfg_8,v);

   DEBUG_PRINTF("FmBali_init:  Initialize the sync memory repair\n");
   PLX_WRITE(boot_ctrl,3);
   if (!boot_status(_plx,this)) return 0;

   DEBUG_PRINTF("FmBali_init:  Initialize the FFU\n");
   v = PLX_READ(ffu_init_slice);
   PLX_WRITE(ffu_init_slice,v);
              
   DEBUG_PRINTF("FmBali_init:  Bring all EPL out of reset\n");
   for(j=1; j<=MAX_PORT; j++) {
     releasePort(_plx,this,j);
   }

   DEBUG_PRINTF("FmBali_init:  Memory initialization\n");
   PLX_WRITE(boot_ctrl,2);
   if (!boot_status(_plx,this)) return 0;

   DEBUG_PRINTF("FmBali_init:  Disable the FFU\n");
   PLX_WRITE(sys_cfg_8,0);

   /***************************************************
    * Fix For Errata #17 on CPU port.
    * Setting priority counters to count by
    * traffic class
    **************************************************/
   v = PLX_READ(stats_cfg[0]._data[0]);
   v |= (1<<15);
   PLX_WRITE(stats_cfg[0]._data[0],v);

   for(j=1; j<=MAX_PORT; j++) {
     /***************************************************
      * Turn off check end bit, due to bali bringup
      * issues. Turn on local fault and remore fault 
      * response. 
      **************************************************/
     //  Not sure this accomplishes anything for us
     v = PLX_READ(epl_phy[j].pcs_cfg_1);
     v &= (1<<31);
     v |= (3<<21);
     PLX_WRITE(epl_phy[j].pcs_cfg_1,v);

     /***************************************************
      * Enable the drain bit.  The link debouncer will
      * properly reset this bit when the link goes up.
      **************************************************/
     v = PLX_READ(epl_phy[j].mac_cfg_2);
     v |= (1<<9);  // drain Tx
     PLX_WRITE(epl_phy[j].mac_cfg_2,v);

     /***************************************************
      * Set sync buf config for proper PCS configuration.
      **************************************************/
     PLX_WRITE(epl_phy[j].syncbuf_cfg,0x3f00);

     /***************************************************
      * Set the jitter watermark to the proper 
      * experimentally derived value.
      **************************************************/
     PLX_WRITE(epl_phy[j].jitter_timer,0x1c1006);

     /***************************************************
      * Lower the number of active stats groups.
      **************************************************/
     // if switch version is fm4224_a1

     /***************************************************
      * Fix For Errata #17
      * Setting priority counters to count by
      * traffic class
      **************************************************/ 
     v = PLX_READ(stats_cfg[j]._data[0]);
     v |= (1<<15);
     PLX_WRITE(stats_cfg[j]._data[0],v);

     /***************************************************
      * Set the learning mode as needed.
      **************************************************/
//      v = PLX_READ(port_cfg_3[j]);
//      v &= ~(3<<5);
//      PLX_WRITE(port_cfg_3[j],v);

     /***************************************************
      * Set default src glort
      **************************************************/
     PLX_WRITE(port_cfg_isl[j],j);

     /***************************************************
      * Set the learning rate limiter 
      **************************************************/
     // skipping
  }

              
#ifdef PROFILE
   time = get_timer(time);
   printf("FmBali_init startup complete %lu ms\n", time);
#endif

   if ( ((v = PLX_READ(sys_cfg_1))&0x7c2f) != 0x1c2f) {
     printf("FmBali_init fail: sys_cfg_1 = 0x%x\n",v);
     return 0;
   }

#ifdef PROFILE
   time = get_timer(0);
#endif

   const unsigned ports312MHz = cfg->portCfg.ports10Gb | cfg->portCfg.ports2_5Gb;

   for(k=1; k<=MAX_PORT; k++) {
     v = PLX_READ(epl_port_ctrl[k]);
     v = 0x1;
     PLX_WRITE(epl_port_ctrl[k],v);

     if (ports312MHz & (1<<k))
       v &= ~0x4;
     else
       v |= 0x4;
     PLX_WRITE(epl_port_ctrl[k],v);

     v &= ~0x8;
     PLX_WRITE(epl_port_ctrl[k],v);

     FmEplRegsB* p = &this->epl_phy[k-0];
     const unsigned pmask = (1<<k);
     int lb     = cfg->portCfg.portsLoopback & (1<<k);
     int rxFlip = cfg->portCfg.portsRxFlip   & (1<<k);
     int txFlip = cfg->portCfg.portsTxFlip   & (1<<k);
     if (cfg->portCfg.ports10Gb & pmask)
       FmEplRegsB_setup10Gb (_plx, p, lb, rxFlip, txFlip, 0);
     else if (cfg->portCfg.ports4Gb & pmask)
       FmEplRegsB_setup4Gb  (_plx, p, lb, rxFlip, txFlip);
     else if (cfg->portCfg.ports2_5Gb & pmask)
       FmEplRegsB_setup2_5Gb(_plx, p, lb, rxFlip, txFlip);
     else if (cfg->portCfg.ports1Gb & pmask)
       FmEplRegsB_setup1Gb  (_plx, p, lb, rxFlip, txFlip);
     else if (cfg->portCfg.ports100Mb & pmask)
       FmEplRegsB_setup100Mb(_plx, p, lb, rxFlip, txFlip);
     else if (cfg->portCfg.ports10Mb & pmask)
       FmEplRegsB_setup10Mb (_plx, p, lb, rxFlip, txFlip);
     else
       FmEplRegsB_disable   (_plx, p);
   }

#ifdef PROFILE
   time = get_timer(time);
   printf("FmBali_init epl config %lu ms\n", time);
#endif

#ifdef PROFILE
   time = get_timer(0);
#endif

   /*  Setup forwarding tables (STP) */
   for(k=0; k<Entries; k++) {
     FmIngressFidTableEntry_write(_plx,&this->ingress_fid_table[k],&cfg->swCfg.fid_table[k]);
     FmEgressFidTableEntry_write (_plx,&this->egress_fid_table [k],&cfg->swCfg.fid_table[k]);
   }

   /* Vlan Port association */
   for(k=0; k<MAX_PORT; k++) {
     FmPort p = k+1;
     unsigned defv = cfg->swCfg.port_vlan.defaultVlan[k]&0xfff;
     unsigned v = PLX_READ(port_cfg_1[k+1]);
     v &= ~0xfff;
     v |= defv;
     if      (cfg->swCfg.port_vlan.untagged & p)
       v |= (1<<22);
     else if (cfg->swCfg.port_vlan.tagged & p)
       v |= (1<<23);
     PLX_WRITE(port_cfg_1[k+1],v);

     v = PLX_READ(port_cfg_3[k+1]);
     if (cfg->swCfg.port_vlan.strict & p)
       v |= (1<<0);
     PLX_WRITE(port_cfg_3[k+1],v);

     for(j=0; j<0x80; j++)
       PLX_WRITE(epl_phy[k].vlantag_table[j],0);

     if (cfg->swCfg.port_vlan.tagged & p)
       PLX_WRITE(epl_phy[k].vlantag_table[defv>>5],1<<(defv&0x1f));
   }

   /* Port forwarding */
   for(k=1; k<=MAX_PORT; k++)
     PLX_WRITE(port_cfg_2[k],cfg->swCfg.portfwd_table.forward[k-1]);

   PLX_WRITE(glort_dest_table[0].a,(PHY_PORTS<<1));
   PLX_WRITE(glort_dest_table[0].b,0);
   PLX_WRITE(glort_cam[0],0xffff0000);
   PLX_WRITE(glort_ram[0].a,0x80000006);
   PLX_WRITE(glort_ram[0].b,0);

   for(k=1; k<=MAX_PORT; k++) {
     PLX_WRITE(glort_dest_table[k].a,2<<k);
     PLX_WRITE(glort_dest_table[k].b,0);
     PLX_WRITE(glort_cam[k],0xffff0000|k);
     PLX_WRITE(glort_ram[k].a,0x80000006|(k<<3));
     PLX_WRITE(glort_ram[k].b,0);
   }

   for(k=0; k<Trunks; k++) {
     unsigned m = cfg->swCfg.trunk[k];
     unsigned q = k+MAX_PORT+1;
     if (m) {
       unsigned i;
       unsigned n=0;

       for(i=1; i<=MAX_PORT; i++) {
         if (m & (1<<i)) n++;
       }

       unsigned p=0;
       for(i=1; i<=MAX_PORT; i++) {
         if (m & (1<<i)) {
           v = ((n&0xf)<<0) |
             ((p&0xf)<<4) |
             (1<<9);
           PLX_WRITE(lag_cfg[i],v);
           p++;
         }
       }

       v = ((k+1)<<8) | (8<<16);
       PLX_WRITE(canonical_glort_cam[k],v);

       PLX_WRITE(glort_dest_table[q].a,m<<1);

       v = ((k+1)<<8) | (0xff00);
       PLX_WRITE(glort_cam[q],v);

       v = (2<<1) | (q<<3) | (4<<19) | ((n&0xf)<<31);
       PLX_WRITE(glort_ram[q].a,v);

       v = ((n&0xf)>>1);
       PLX_WRITE(glort_ram[q].b,v);
     }
   }

   /* VLAN table */
   for(k=0; k<Entries; k++) {
     FmIngressVlanTableEntry_write(_plx,&this->ingress_vid_table[k],&cfg->swCfg.vid_table[k]); 
     FmEgressVlanTableEntry_write (_plx,&this->egress_vid_table [k],&cfg->swCfg.vid_table[k]); 
   } 

#ifdef PROFILE
   time = get_timer(time);
   printf("FmBali init switch tables %lu ms\n", time);
#endif

   /* Turn off learning
   **    for(k=0; k<24; k++) {
   **      v = _dev.port_cfg_1[k];
   **      _dev.port_cfg_1[k] = v &~ 0x100000;
   **    } */
   /*
   PLX_WRITE(frame_ctrl_im, 
             PLX_READ(frame_ctrl_im) & 
             ~(Entry_Aged | Entry_Learned | Bin_Full | FIFO_Full));
   PLX_WRITE(mgr_im,0);
   */
   PLX_WRITE(perr_im,0);
   PLX_WRITE(lci_im, RxEOT | RxRdy);
   PLX_WRITE(port_vlan_im_1,0);
   PLX_WRITE(port_vlan_im_2,0);
   PLX_WRITE(port_mac_sec_im,0);
   PLX_WRITE(trigger_im[0],0);
   PLX_WRITE(trigger_im[1],0);

   //  Clear interrupt status registers
   PLX_WRITE(global_epl_int_detect,0xffffffff);
   PLX_WRITE(lci_ip               ,0xffffffff);
   PLX_WRITE(interrupt_detect     ,0xffffffff);

   PLX_WRITE(sys_cfg_7,0x7530);   /* age entries once/2.4ms, whole table/40s */
/*    PLX_WRITE(global_pause_wm[0],0xffffffff); */
/*    PLX_WRITE(rx_pause_wm[0],0xffffffff);  /\* no pause for CPU *\/ */

/*    PLX_WRITE(queue_cfg_5,1000); */
/*    for(k=0; k<=24; k++) */
/*      PLX_WRITE(queue_cfg_1[k],0x03ff02ff);  /\* allow hog ports   { [9:0] = 0xff (works), 0x3ff (broken) } *\/ */

   PLX_WRITE(sys_cfg_1,0);              /*  Prevent traps  */
   PLX_WRITE(cpu_trap_mask_fh,0);       /* drop all traps */

   PLX_WRITE(frame_time_out,0x989680);  /* 5 minutes */

   /*  Set store and forward matrix */
   for(k=1; k<=MAX_PORT; k++) {
     const unsigned pmask = (1<<k);
     unsigned mask = 0xffffffff;
     if ( cfg->portCfg.ports10Gb  & pmask )  mask = ~cfg->portCfg.ports10Gb;
     if ( cfg->portCfg.ports4Gb   & pmask )  mask = ~cfg->portCfg.ports4Gb;
     if ( cfg->portCfg.ports2_5Gb & pmask )  mask = ~cfg->portCfg.ports2_5Gb;
     if ( cfg->portCfg.ports1Gb   & pmask )  mask = ~cfg->portCfg.ports1Gb;
     if ( cfg->portCfg.ports100Mb & pmask )  mask = ~cfg->portCfg.ports100Mb;
     if ( cfg->portCfg.ports10Mb  & pmask )  mask = ~cfg->portCfg.ports10Mb;
     PLX_WRITE(saf_matrix[k],mask);
   }

   free(cfg);

   DEBUG_PRINTF("FmBali_init boot done\n");

   return 1;
}

int boot_status(plx* _plx, FmBali* this)
{
  unsigned v;
#if 0
  unsigned k=1024;
  do {
    v = PLX_READ(boot_status);
  } while( v==0 && --k );
  if (!k) {
    printf("FAIL: FmBali_init boot_status poll timeout (%x)\n",v);
    return 0;
  }
#else
  do {
    v = PLX_READ(boot_status);
  } while ( v==0 );
#endif
  return 1;   
}

void releasePort(plx* _plx, FmBali* this, int port)
{
  volatile unsigned v;

  v = PLX_READ(epl_port_ctrl[port]);

  /***************************************************
   * We must set the datapath initialization bit
   * before we exit reset.
   **************************************************/
  v &= ~8;
  PLX_WRITE(epl_port_ctrl[port],v);


  /***************************************************
   * We must set the clock selection bit before we
   * exit reset.
   **************************************************/
  // TODO A or B clock?

  /***************************************************
   * Now put the port out of reset.
   **************************************************/
  v &= ~1;
  PLX_WRITE(epl_port_ctrl[port],v);
}
