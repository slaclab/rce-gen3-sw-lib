

#include "../FmTahoe_init.h"
#include "../FmTahoe.h"
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

/*
**  Boot Tahoe switch
**    Do not setup interrupts, MAC table management, or LCI port
**    (will do with a daemon)
*/
int FmTahoe_init(plx* _plx, FmConfig* cfg)
{
  volatile unsigned v;
  unsigned j,k;
  FmTahoe* this = 0;
#if 0
  FmEplHandler epl;
  epl._dev = this;
  epl._oneLanePorts = 0;
#endif

#ifdef PROFILE
  unsigned long time = get_timer(0);
#endif

  /*  Poll boot status until zero or timeout */
   { unsigned k=1024;
     do {
       v = PLX_READ(boot_status);
     } while( v && --k );
     if (!k) {
       printf("FAIL: FmTahoeMgr boot_status poll1 timeout (%x)\n",v);
       return 0;
     }
   }

   /*  Begin boot state machine
   **  ( Enable DFT/JTAG with 0xd05 )
   **  Errata item #10 "Contention between 1Gb Mode and LED Interface
   **    Port Status
   **   _dev.chip_mode = 0xd04; */

   PLX_WRITE(chip_mode,0xd00);

   DEBUG_PRINTF("FmTahoeMgr waiting for boot complete\n");

   /*  Poll until boot is complete */
   { unsigned k=20480;
     do {
       v = PLX_READ(boot_status);
     } while( v && --k );
     if (!k) {
       printf("FmTahoeMgr boot status didn't clear in 20480 reads (%x)\n",v);
       return 0;
     }
     else {
       DEBUG_PRINTF("FmTahoeMgr boot status cleared in %d reads\n",20480-k);
     }
   }

#ifdef PROFILE
   time = get_timer(time);
   printf("FmTahoe boot %lu ms\n", time);
#endif
   
   /*  Read vital product data register */
   {
     v = PLX_READ(vpd);
     if ( (v & 0x0fffffff) != 0xae1842b ) {
       DEBUG_PRINTF("FAIL: FmTahoe_init: Read incorrect product code %08x (%08x)\n",v,0xae1842b);
       return 0;
     }
   }

   /*  Ref design writes and reads from shadow fusebox here as a test */

#ifdef PROFILE
   time = get_timer(0);
#endif

   /*  Program frame handler clock for 300MHz
   **  (some contradiction in the description of bit 0 for this register)
   **  pllout<<15 | m << 4 | n << 11 | p << 2
   **  p=0; m=1b; n=3  (297 MHz) */

   while(1) {
     //     printf("pll fh ctrl = %x\n", PLX_READ(pll_fh_ctrl));
     PLX_WRITE(pll_fh_ctrl,0x19b0);
     //     printf("pll fh ctrl = %x\n", PLX_READ(pll_fh_ctrl));

     DEBUG_PRINTF("waiting\n");
     //     udelay(1000000);
     udelay(200);
     DEBUG_PRINTF("polling\n");

     /*  Poll until frame handler pll locks */
     { unsigned k = 10240;
       do {
         v = PLX_READ(pll_fh_stat);
       } while ( v==0 && --k );
       if (!k) {
         printf("FmTahoeMgr timeout waiting for FH_PLL to lock\n");
       }
       else {
         DEBUG_PRINTF("FmTahoeMgr fh pll locked\n");
         break;
       }
     }
     DEBUG_PRINTF("pll fh ctrl = %x\n", PLX_READ(pll_fh_ctrl));
   }

#ifdef PROFILE
   time = get_timer(time);
   printf("FmTahoe pll lock %lu ms\n", time);
#endif

   /*  Release pll_fh
   **   service::debug::printv("FmTahoeMgr releasing FH reset"); */
   PLX_WRITE(soft_reset,0x0);

   if ( (v = PLX_READ(sys_cfg_1)) != 0x4ff) {
     printf("FmTahoeMgr fail: fm_sys_cfg_1 = 0x%x != 0x4ff\n",v);
     return 0;
   }

   if ( (v = PLX_READ(port_reset)) != 0x1fffffe ) {
     printf("FmTahoeMgr fail: port reset = 0x%x != 0x1fffffe\n",v);
     return 0;
   }

   if ( (v = PLX_READ(epl_phy[_testPort].pcs_cfg_1)) != 0 ) {
     printf("FmTahoeMgr fail: ports in reset but EPL reg reads= 0x%x\n",v);
     return 0;
   }

   PLX_WRITE(epl_phy[_testPort].pcs_cfg_1,0xffffffff);
   if ( (v = PLX_READ(epl_phy[_testPort].pcs_cfg_1)) != 0 ) {
     printf("FmTahoeMgr fail: ports in reset but EPL reg allowed write\n");
     return 0;
   }

   const unsigned ports312MHz = cfg->portCfg.ports10Gb | cfg->portCfg.ports2_5Gb;
   const unsigned _defaultPorts =
     0x1 |
     cfg->portCfg.ports10Gb  |
     cfg->portCfg.ports4Gb  |
     cfg->portCfg.ports2_5Gb |
     cfg->portCfg.ports1Gb   |
     cfg->portCfg.ports100Mb |
     cfg->portCfg.ports10Mb  ;

   v = 0x1fffffe & ~ports312MHz;
   PLX_WRITE(port_clk_sel,v);

   PLX_WRITE(port_reset,0x1fffffe & ~_defaultPorts);
   /* _dev.port_reset = 0; */

#ifdef PROFILE
   time = get_timer(0);
#endif

   for(k=1; k<=MAX_PORT; k++) {
     FmEplRegs* p = &this->epl_phy[k-1];
     PLX_WRITE(epl_phy[k].serdes_ctrl_1,0x88884444);
     const unsigned pmask = (1<<k);
     int lb     = cfg->portCfg.portsLoopback & (1<<k);
     int rxFlip = cfg->portCfg.portsRxFlip   & (1<<k);
     int txFlip = cfg->portCfg.portsTxFlip   & (1<<k);
     if (cfg->portCfg.ports10Gb & pmask)
       FmEplRegs_setup10Gb (_plx, p, lb, rxFlip, txFlip, 0);
     else if (cfg->portCfg.ports4Gb & pmask)
       FmEplRegs_setup4Gb  (_plx, p, lb, rxFlip, txFlip);
     else if (cfg->portCfg.ports2_5Gb & pmask)
       FmEplRegs_setup2_5Gb(_plx, p, lb, rxFlip, txFlip);
     else if (cfg->portCfg.ports1Gb & pmask)
       FmEplRegs_setup1Gb  (_plx, p, lb, rxFlip, txFlip);
     else if (cfg->portCfg.ports100Mb & pmask)
       FmEplRegs_setup100Mb(_plx, p, lb, rxFlip, txFlip);
     else if (cfg->portCfg.ports10Mb & pmask)
       FmEplRegs_setup10Mb (_plx, p, lb, rxFlip, txFlip);
     else
       FmEplRegs_disable   (_plx, p);
   }

#ifdef PROFILE
   time = get_timer(time);
   printf("FmTahoe epl config %lu ms\n", time);
#endif

   /*  Set default watermarks (jitter [bug#6892] and pause [bug#6896])
   **   _dev.jitter_cfg = 0x00181818;
   **  Increase jitter watermark to avoid rare tx uflow */
   PLX_WRITE(jitter_cfg,0x00383818);
   PLX_WRITE(queue_cfg_3,0xffffffff);  /* no WRED */
   for(k=0; k<=24; k++) {
     PLX_WRITE(queue_cfg_2[k],0x10); /* rx private = 16kB */
     PLX_WRITE(global_pause_wm[k],0x01200144);
     PLX_WRITE(rx_pause_wm    [k],0xe500ef);  /* pause hog watermarks for jumbo frames */
   }

   /*  Set in FmMacTable.cc
   **   _dev.ma_table_cfg = 1;    best-effort lookup */

#ifdef PROFILE
   time = get_timer(0);
#endif

   /*  Setup forwarding tables (STP) */
   for(k=0; k<Entries; k++)
     FmFidTableEntry_write(_plx,&this->fid_table[k],&cfg->swCfg.fid_table[k]);

   /* Vlan Port association */
   for(k=0; k<MAX_PORT; k++) {
     FmPort p = k+1;
     unsigned v = 0;
     v |= EnableLearning;
     v |= cfg->swCfg.port_vlan.defaultVlan[k]&0xfff;
     if      (cfg->swCfg.port_vlan.untagged & p)
       v |= DropTaggedFrames;
     else if (cfg->swCfg.port_vlan.tagged & p)
       v |= DropUntaggedFrames;
     else if (cfg->swCfg.port_vlan.strict & p)
       v |= FilterIngVlanViolations;

     PLX_WRITE(port_cfg_1[k+1],v);
   }

   /* Port forwarding */
   for(k=1; k<=MAX_PORT; k++)
     PLX_WRITE(port_cfg_2[k],cfg->swCfg.portfwd_table.forward[k-1]);

   for(k=1; k<=MAX_PORT; k++)
     PLX_WRITE(trunk_port_map[k],0);

   for(k=0; k<Trunks; k++) {
     PLX_WRITE(trunk_group_1[k],0);
     PLX_WRITE(trunk_group_2[k],0);
     PLX_WRITE(trunk_group_3[k],0);
   }

   for(k=0; k<Trunks; k++) {
     unsigned m = cfg->swCfg.trunk[k];
     unsigned n = 0;
     unsigned g1 = 0;
     unsigned g2 = 0;
     unsigned i;

     unsigned forward = (2<<MAX_PORT)-1;
     forward ^= cfg->swCfg.trunk[k];
     for(i=1; i<=MAX_PORT; i++) {
       if (cfg->swCfg.trunk[k]&(1<<i))
         PLX_WRITE(port_cfg_2[i],cfg->swCfg.portfwd_table.forward[i-1] & forward);
     }

     for(i=1; i<=MAX_PORT && m; i++) {
       FmPortMask p = (1<<i);
       if (m & p) {
         m &= ~p;
         if (n>=12) {
           // link-aggregate group is full
           // isolate this port
           PLX_WRITE(port_cfg_2[i],0);
           // 
           // We also have a problem if we receive traffic
           // on this port (MAC table gets updated)
           for(j=1; j<=MAX_PORT; j++) {
             v = PLX_READ(port_cfg_2[j]) & ~p;
             PLX_WRITE(port_cfg_2[j],v);
           }
         }
         else {
           if (n<6)       g1 |= i<<(5*n);
           else if (n<12) g2 |= i<<(5*(n-6));

           PLX_WRITE(trunk_port_map[i],((1<<4)|k));
           n++;
         }
       }
     }
     
     PLX_WRITE(trunk_group_1[k], g1);
     PLX_WRITE(trunk_group_2[k], g2);
     PLX_WRITE(trunk_group_3[k], n);
   }

   /* VLAN table */
   for(k=0; k<Entries; k++) {
     PLX_WRITE(vid_table[k]._data[0],cfg->swCfg.vid_table[k]._data[0]);
     PLX_WRITE(vid_table[k]._data[1],cfg->swCfg.vid_table[k]._data[1]);
   }

   PLX_WRITE(sys_cfg_2,VlanTagEnable);

#ifdef PROFILE
   time = get_timer(time);
   printf("FmTahoe switch tables %lu ms\n", time);
#endif

   /* Turn off learning
   **    for(k=0; k<24; k++) {
   **      v = _dev.port_cfg_1[k];
   **      _dev.port_cfg_1[k] = v &~ 0x100000;
   **    } */

   PLX_WRITE(frame_ctrl_im, 
             PLX_READ(frame_ctrl_im) & 
             ~(Entry_Aged | Entry_Learned | Bin_Full | FIFO_Full));
   PLX_WRITE(mgr_im,0);
   PLX_WRITE(perr_im,0);
   PLX_WRITE(lci_im, RxEOT | RxRdy);
   PLX_WRITE(port_vlan_im_1,0);
   PLX_WRITE(port_vlan_im_2,0);
   PLX_WRITE(port_mac_sec_im,0);
   PLX_WRITE(trigger_im,0);

#if 0
   if (!_macTable)
     _macTable = new FmMacTable(_dev);
   for(unsigned k=0; k<0x4000; k++)
     if (cfg.swCfg.mac_table.entry[k].valid())
       _macTable->addEntry(cfg.swCfg.mac_table.entry[k]);
   if (!_eplHandler)
     _eplHandler = new FmEplHandler(_dev);
   { const unsigned oneLanePorts =
       cfg->portCfg.ports10Mb |
       cfg->portCfg.ports100Mb |
       cfg->portCfg.ports1Gb |
       cfg->portCfg.ports2_5Gb;
     const unsigned fourLanePorts =
       cfg->portCfg.ports4Gb |
       cfg->portCfg.ports10Gb;
     for(k=0; k<24; k++) {
       FmPort port = k+1;
       unsigned pmask = 1<<port;
       if (pmask & oneLanePorts)
         FmEplHandler_enableOneLanePort(_plx,&epl,port);
       if (pmask & fourLanePorts)
         FmEplHandler_enableFourLanePort(_plx,&epl,port);
     }
   }
   //   service::debug::printv("Registering handlers");
   registerHandler(LCI ,new FmDefaultHandler("LCI"));
   registerHandler(TRG ,new FmDefaultHandler("TRG"));
   registerHandler(FCTL,_macTable);
   registerHandler(MGR ,new FmMgrHandler    (&_dev));
   registerHandler(EPL ,_eplHandler);
   registerHandler(SEC ,new FmDefaultHandler("SEC"));
   registerHandler(VLN2,new FmDefaultHandler("VLN2"));
   registerHandler(VLN1,new FmDefaultHandler("VLN1"));
   registerHandler(PERR,new FmDefaultHandler("PERR"));
#endif
   //  Clear interrupt status registers
   v = PLX_READ(global_epl_int_detect);
   v = PLX_READ(lci_ip);
   v = PLX_READ(interrupt_detect);

#if 0
   //
   //  Setup the CPU interface
   //
   v = PLX_READ(lci_cfg);
   v |= BigEndian;  //  Setup the CPU as big-endian
   v |= TxCompCRC;  //  Compute the CRC for tx packets
   //   v |= RxEnable;   //  Enable the receipt of packets
   PLX_WRITE(lci_cfg,v);
#endif

   PLX_WRITE(sys_cfg_1,0x400);    /* Drop pause frames; disable traps */
   PLX_WRITE(sys_cfg_7,0x7530);   /* age entries once/2.4ms, whole table/40s */
   PLX_WRITE(global_pause_wm[0],0xffffffff);
   PLX_WRITE(rx_pause_wm[0],0xffffffff);  /* no pause for CPU */
   PLX_WRITE(queue_cfg_5,1000);
   for(k=0; k<=24; k++)
     PLX_WRITE(queue_cfg_1[k],0x03ff02ff);  /* allow hog ports   { [9:0] = 0xff (works), 0x3ff (broken) } */

   PLX_WRITE(frame_time_out,0x989680);  /* 5 minutes */

   /*  Set store and forward matrix */
   for(k=1; k<=24; k++) {
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

#if 0
   //
   //  Configure the triggers
   //
   for(unsigned k=0; k<16; k++)
     configureTrigger(k,cfg->trigCfg[k]);
#endif

   free(cfg);

   DEBUG_PRINTF("FmTahoeMgr boot done\n");

   return 1;
}

