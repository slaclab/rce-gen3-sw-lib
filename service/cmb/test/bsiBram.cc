
#include "cmb/bsi.hh"


#if tgt_gen2
void bramInit_1()
{
  // Normally done by the IPMC
  static const uint32_t cfg[] = {
    0x00000000,                         //  0: bootstrap
    0xfe000009,                         //  1: version_size ( -2, 9 )
    0x00000001,                         //  2: cluster_element (0, 1)
    0xfffffffc,                         //  3: config_version
    0x5368656c, 0x66000000,             //  4: group name [8]
      0, 0, 0, 0, 0, 0,
    0x0000001a,                         // 12: network prefix
    0xac1507c9,                         // 13: gateway: rdsrv101d
    0xac1507c9,                         // 14: logger:  rdsrv101d
    0xac1507c9,                         // 15: ntp:     rdsrv101d
    0xac1507c9,                         // 16: dns:     rdsrv101d
    0x6c616231, 0x2e726567,             // 17: domain name [16]
          0, 0, 0, 0, 0, 0,             //     continued
    0, 0, 0, 0, 0, 0, 0, 0,             //     continued
    0x01020304, 0x05060708, 0x090a0b0c, // 33: int endpoint [3]
    0x11121314, 0x15161718, 0x191a1b1c, // 36: ext endpoint [7]
    0x1d1e1f20, 0x21222324, 0x25262728, 0x292a2b2c,
    0x00000000, 0x00000000,             // 43: mac DTM 0.0
    0x00000000, 0x00000000,             // 45: mac DTM 0.1
    0x56004224, 0x00000800,             // 47: mac DPM 0.0
    0x00000000, 0x00000000,             // 49: mac DPM 0.1
    0x00000000, 0x00000000,             // 51: mac DPM 1.0
    0x00000000, 0x00000000,             // 53: mac DPM 1.1
    0x00000000, 0x00000000,             // 55: mac DPM 2.0
    0x00000000, 0x00000000,             // 57: mac DPM 2.1
    0x00000000, 0x00000000,             // 59: mac DPM 3.0
    0x00000000, 0x00000000,             // 61: mac DPM 3.1
    0x00000000, 0x00000000,             // 63: ip: DTM 0.0, 0.1
    0xac1507ca, 0x00000000,             // 64: ip: DPM 0.0: rce36, 0.1
    0x00000000, 0x00000000,             // 66: ip: DPM 1.0, 1.1
    0x00000000, 0x00000000,             // 68: ip: DPM 2.0, 2.1
    0x00000000, 0x00000000,             // 70: ip: DPM 3.0, 3.1
    0xffffffff                          // 72: booted
  };

  BSI::bram(cfg, sizeof(cfg) / sizeof(*cfg));

  // End 'normally done by the IPMC'
}


void bramInit_2()
{
  using service::cmb::BSI;

  const BSI* const bsi = BSI::instance();

  // Normally done by the IPMC
  bsi->bs_loader(0x0, 0, 0);           //  0: bootstrap
  bsi->ver_size(0xfe, 9);              //  1: version_size ( -2, 9 )
  bsi->cluster_addr(0, 2);             //  2: cluster_element (0, 2)
  bsi->cfgVersion(0xfffffffc);         //  3: config_version
  bsi->group(0, 0x5368656c);           //  4: group name [8]
  bsi->group(1, 0x66000000);
  bsi->group(2, 0);
  bsi->group(3, 0);
  bsi->group(4, 0);
  bsi->group(5, 0);
  bsi->group(6, 0);
  bsi->group(7, 0);
  bsi->network_prefix(0x0000001a);     // 12: network prefix
  bsi->gw(0xac1507c9);                 // 13: gateway: rdsrv101d
  bsi->logger(0xac1507c9);             // 14: logger:  rdsrv101d
  bsi->ntp(0xac1507c9);                // 15: ntp:     rdsrv101d
  bsi->dns(0xac1507c9);                // 16: dns:     rdsrv101d
  bsi->domain( 0, 0x6c616231);         // 17: domain name [16]
  bsi->domain( 1, 0x2e726567);
  bsi->domain( 2, 0);  bsi->domain( 3, 0);
  bsi->domain( 4, 0);  bsi->domain( 5, 0);
  bsi->domain( 6, 0);  bsi->domain( 7, 0);
  bsi->domain( 8, 0);  bsi->domain( 9, 0);
  bsi->domain(10, 0);  bsi->domain(11, 0);
  bsi->domain(12, 0);  bsi->domain(13, 0);
  bsi->domain(14, 0);  bsi->domain(15, 0);
  bsi->int_endpoint(0, 0x01020304);    // 33: int endpoint [0]
  bsi->int_endpoint(1, 0x05060708);    // 34: int endpoint [1]
  bsi->int_endpoint(2, 0x090a0b0c);    // 35: int endpoint [2]
  bsi->ext_endpoint(0, 0x11121314);    // 36: ext endpoint [0]
  bsi->ext_endpoint(1, 0x15161718);    // 37: ext endpoint [1]
  bsi->ext_endpoint(2, 0x191a1b1c);    // 38: ext endpoint [2]
  bsi->ext_endpoint(3, 0x1d1e1f20);    // 39: ext endpoint [3]
  bsi->ext_endpoint(4, 0x21222324);    // 40: ext endpoint [4]
  bsi->ext_endpoint(5, 0x25262728);    // 41: ext endpoint [5]
  bsi->ext_endpoint(6, 0x292a2b2c);    // 42: ext endpoint [6]
  bsi->mac(0, 0x000008015600430full);  // 43: mac DTM 0.0
  bsi->mac(1, 0x000008025600430full);  // 43: mac DTM 0.1
  //bsi->mac(2, 0x0000080056004224ull);  // 44: mac DPM 0.0
  bsi->mac(2, 0x000008005600430full);  // 44: mac DPM 0.0
  bsi->mac(3, 0x000008045600430full);  // 45: mac DPM 0.1
  bsi->mac(4, 0x000008055600430full);  // 46: mac DPM 1.0
  bsi->mac(5, 0x000008065600430full);  // 47: mac DPM 1.1
  bsi->mac(6, 0x000008075600430full);  // 48: mac DPM 2.0
  bsi->mac(7, 0x000008085600430full);  // 49: mac DPM 2.1
  bsi->mac(8, 0x000008095600430full);  // 50: mac DPM 3.0
  bsi->mac(9, 0x0000080a5600430full);  // 51: mac DPM 3.1
  bsi->ip(0, 0x00000000);              // 52: ip: DTM 0.0
  bsi->ip(1, 0x00000000);              // 52: ip: DTM 0.1
  bsi->ip(2, 0xac1507ca);              // 53: ip: DPM 0.0: rce36
  bsi->ip(3, 0x00000000);              // 54: ip: DPM 0.1
  bsi->ip(4, 0x00000000);              // 55: ip: DPM 1.0
  bsi->ip(5, 0x00000000);              // 56: ip: DPM 1.1
  bsi->ip(6, 0x00000000);              // 57: ip: DPM 2.0
  bsi->ip(7, 0x00000000);              // 58: ip: DPM 2.1
  bsi->ip(8, 0x00000000);              // 59: ip: DPM 3.0
  bsi->ip(9, 0x00000000);              // 60: ip: DPM 3.1
  bsi->booted(0xffffffff);             // 61: booted flag

  // End 'normally done by the IPMC'
}
#endif
#if tgt_gen3
void bramInit_3()
{
  using service::cmb::BSI;

  const BSI* const bsi = BSI::instance();

  bsi->ver_size(0xfe, 9);                    // 0x00: version_size ( -2, 9 )
  bsi->network_phy(0xa5);                    // 0x01: Network PHY
  bsi->mac(0x000008005600430full);           // 0x02: MAC
  //bsi->mac(0x0000000a35000122ull);           // 0x02: MAC
  bsi->interconnect(0xdeadbeef);             // 0x04: Interconnect
  bsi->serial_number(0x0000abadcafebabeull); // 0x10: Serial number
  bsi->cluster_addr(0, 1, 2);                // 0x12: Site, bay and element
  bsi->group("Shelf is not a very long name, but this apparently is.");                       // 0x14: Group name [8]
  bsi->ext_interconnect(0x01020304);         // 0x1c: ext interconnect
  bsi->switch_config( 0, 0x05060708);        // 0x30: Switch configuration
  bsi->switch_config( 1, 0x090a0b0c);        // 0x31: Switch configuration
  bsi->switch_config( 2, 0x0d0e0f10);        // 0x32: Switch configuration
  bsi->switch_config( 3, 0x11121314);        // 0x33: Switch configuration
  bsi->switch_config( 4, 0x15161718);        // 0x34: Switch configuration
  bsi->switch_config( 5, 0x191a1b1c);        // 0x35: Switch configuration
  bsi->switch_config( 6, 0x1d1e1f20);        // 0x36: Switch configuration
  bsi->switch_config( 7, 0x21222324);        // 0x37: Switch configuration
  bsi->switch_config( 8, 0x25262728);        // 0x38: Switch configuration
  bsi->switch_config( 9, 0x292a2b2c);        // 0x39: Switch configuration
  bsi->switch_config(10, 0x2d2e2f30);        // 0x3a: Switch configuration
  bsi->switch_config(11, 0x31323334);        // 0x3b: Switch configuration
  bsi->switch_config(12, 0x35363738);        // 0x3c: Switch configuration
  bsi->switch_config(13, 0x393a3b3c);        // 0x3d: Switch configuration
  bsi->switch_config(14, 0x3d3e3f40);        // 0x3e: Switch configuration
  bsi->switch_config(15, 0x41424344);        // 0x3f: Switch configuration
  bsi->switch_config(16, 0x45464748);        // 0x40: Switch configuration
  bsi->switch_config(17, 0x494a4b4c);        // 0x41: Switch configuration
  bsi->switch_config(18, 0x4d4e4f50);        // 0x42: Switch configuration
  bsi->switch_config(19, 0x51525354);        // 0x43: Switch configuration
  bsi->switch_config(20, 0x55565758);        // 0x44: Switch configuration
  bsi->switch_config(21, 0x595a5b5c);        // 0x45: Switch configuration
  bsi->switch_config(22, 0x5d5e5f60);        // 0x46: Switch configuration
  bsi->switch_config(23, 0x61626364);        // 0x47: Switch configuration
  bsi->ce_interconnect( 0, 0x65666768);      // 0x48: Cluster Element interconnect
}
#endif
