#!@file     bsiBram.tcl
#
# @brief     BSI BRAM initialization script for use with XMD
#
# @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
#
# @date      August 14, 2013 -- Created
#
# $Revision: 1918 $
#
# @verbatim                    Copyright 2013
#                                     by
#                        The Board of Trustees of the
#                      Leland Stanford Junior University.
#                             All rights reserved.
# @endverbatim

puts "bsiInit_3"

proc bsiInit_3 {} {
    mwr 0x84000000  0xfe000009; # 0x00: version_size ( -2, 9 )
    mwr 0x84000004  0x000000a5; # 0x01: Network PHY
    mwr 0x84000008  0x5600430f; # 0x02: MAC address
    mwr 0x8400000c  0x00000800;
    mwr 0x84000010  0xdeadbeef; # 0x04: Interconnect

    mwr 0x84000040  0xcafebabe; # 0x10: Serial number
    mwr 0x84000044  0x0000abad;
    mwr 0x84000048  0x00000102; # 0x12: Site, bay and element
    mwr 0x8400004c  0x6c656853; # 0x13: "Shelf"
    mwr 0x84000050  0x73692066;
    mwr 0x84000054  0x746f6e20;
    mwr 0x84000058  0x76206120;
    mwr 0x8400005c  0x20797265;
    mwr 0x84000060  0x676e6f6c;
    mwr 0x84000064  0x6d616e20;
    mwr 0x84000068  0x62202c65;
    mwr 0x8400006c  0x01020304; # 0x1b: ext interconnect

    mwr 0x840000c0  0x05060708; # 0x30: Switch configuration
    mwr 0x840000c4  0x090a0b0c;
    mwr 0x840000c8  0x0d0e0f10;
    mwr 0x840000cc  0x11121314;
    mwr 0x840000d0  0x15161718;
    mwr 0x840000d4  0x191a1b1c;
    mwr 0x840000d8  0x1d1e1f20;
    mwr 0x840000dc  0x21222324;
    mwr 0x840000e0  0x25262728;
    mwr 0x840000e4  0x292a2b2c;
    mwr 0x840000e8  0x2d2e2f30;
    mwr 0x840000ec  0x31323334;
    mwr 0x840000f0  0x35363738;
    mwr 0x840000f4  0x393a3b3c;
    mwr 0x840000f8  0x3d3e3f40;
    mwr 0x840000fc  0x41424344;
    mwr 0x84000100  0x45464748;
    mwr 0x84000104  0x494a4b4c;
    mwr 0x84000108  0x4d4e4f50;
    mwr 0x8400010c  0x51525354;
    mwr 0x84000110  0x55565758;
    mwr 0x84000114  0x595a5b5c;
    mwr 0x84000118  0x5d5e5f60;
    mwr 0x8400011c  0x61626364;
    mwr 0x84000120  0x65666768; # 0x48: Cluster Element interconnect

    # Initialize and start the global timer
    mwr 0xF8F00208 0;
    mwr 0xF8F00200 0 2;
    mwr 0xF8F00208 1;
}
