#include "datCode.hh"
#include DAT_PUBLIC(bootstrap, common, ppc-asm.h)

        .section .text,"ax"

        .global testUDI

testUDI:
        udi0fcm  r3,r4,r5
        udi1fcm  r3,r4,r5
        udi2fcm  r3,r4,r5
        udi3fcm  r3,r4,r5
        udi4fcm  r3,r4,r5
        udi5fcm  r3,r4,r5
        udi6fcm  r3,r4,r5
        udi7fcm  r3,r4,r5
        udi8fcm  r3,r4,r5
        udi9fcm  r3,r4,r5
        udi10fcm r3,r4,r5
        udi11fcm r3,r4,r5
        udi12fcm r3,r4,r5
        udi13fcm r3,r4,r5
        udi14fcm r3,r4,r5
        udi15fcm r3,r4,r5

        udi0fcm.  r3,r4,r5
        udi1fcm.  r3,r4,r5
        udi2fcm.  r3,r4,r5
        udi3fcm.  r3,r4,r5
        udi4fcm.  r3,r4,r5
        udi5fcm.  r3,r4,r5
        udi6fcm.  r3,r4,r5
        udi7fcm.  r3,r4,r5
        udi8fcm.  r3,r4,r5
        udi9fcm.  r3,r4,r5
        udi10fcm. r3,r4,r5
        udi11fcm. r3,r4,r5
        udi12fcm. r3,r4,r5
        udi13fcm. r3,r4,r5
        udi14fcm. r3,r4,r5
        udi15fcm. r3,r4,r5

ld_st: // See UG200, page 197: FCM Load/Store Instructions
//      .long   0x0111 11TT TTTA AAAA BBBB Bxxx xx00 1110  // U W LS

//      .long   0x0111 1100 0110 0100 0010 1000 0000 1110  // 0 0 0
//      .long   0x0111 1100 0110 0100 0010 1000 0100 1110  // 0 1 0
//      .long   0x0111 1100 0110 0100 0010 1000 1000 1110  // 0 2 0
//      .long   0x0111 1100 0110 0100 0010 1000 1100 1110  // 0 3 0
//      .long   0x0111 1100 0110 0100 0010 1010 0000 1110  // 0 4 0
//      .long   0x0111 1100 0110 0100 0010 1001 0000 1110  // 0 0 1
//      .long   0x0111 1100 0110 0100 0010 1001 0100 1110  // 0 1 1
//      .long   0x0111 1100 0110 0100 0010 1001 1000 1110  // 0 2 1
//      .long   0x0111 1100 0110 0100 0010 1001 1100 1110  // 0 3 1
//      .long   0x0111 1100 0110 0100 0010 1011 0000 1110  // 0 4 1
//      .long   0x0111 1100 0110 0100 0010 1100 0000 1110  // 1 0 0
//      .long   0x0111 1100 0110 0100 0010 1100 0100 1110  // 1 1 0
//      .long   0x0111 1100 0110 0100 0010 1100 1000 1110  // 1 2 0
//      .long   0x0111 1100 0110 0100 0010 1100 1100 1110  // 1 3 0
//      .long   0x0111 1100 0110 0100 0010 1110 0000 1110  // 1 4 0
//      .long   0x0111 1100 0110 0100 0010 1101 0000 1110  // 1 0 1
//      .long   0x0111 1100 0110 0100 0010 1101 0100 1110  // 1 1 1
//      .long   0x0111 1100 0110 0100 0010 1101 1000 1110  // 1 2 1
//      .long   0x0111 1100 0110 0100 0010 1101 1100 1110  // 1 3 1
//      .long   0x0111 1100 0110 0100 0010 1111 0000 1110  // 1 4 1

        //    opcode r3,r4,r5       // U W LS
        .long   0x7c64280e          // 0 0 0
        .long   0x7c64284e          // 0 1 0
        .long   0x7c64288e          // 0 2 0
        .long   0x7c6428ce          // 0 3 0
        .long   0x7c642a0e          // 0 4 0
        .long   0x7c64290e          // 0 0 1
        .long   0x7c64294e          // 0 1 1
        .long   0x7c64298e          // 0 2 1
        .long   0x7c6429ce          // 0 3 1
        .long   0x7c642b0e          // 0 4 1
        .long   0x7c642c0e          // 1 0 0
        .long   0x7c642c4e          // 1 1 0
        .long   0x7c642c8e          // 1 2 0
        .long   0x7c642cce          // 1 3 0
        .long   0x7c642e0e          // 1 4 0
        .long   0x7c642d0e          // 1 0 1
        .long   0x7c642d4e          // 1 1 1
        .long   0x7c642d8e          // 1 2 1
        .long   0x7c642dce          // 1 3 1
        .long   0x7c642f0e          // 1 4 1

        lbfcmx   r3,r4,r5
        lhfcmx   r3,r4,r5
        lwfcmx   r3,r4,r5
        lqfcmx   r3,r4,r5
        ldfcmx   r3,r4,r5
        stbfcmx  r3,r4,r5
        sthfcmx  r3,r4,r5
        stwfcmx  r3,r4,r5
        stqfcmx  r3,r4,r5
        stdfcmx  r3,r4,r5
        lbfcmux  r3,r4,r5
        lhfcmux  r3,r4,r5
        lwfcmux  r3,r4,r5
        lqfcmux  r3,r4,r5
        ldfcmux  r3,r4,r5
        stbfcmux r3,r4,r5
        sthfcmux r3,r4,r5
        stwfcmux r3,r4,r5
        stqfcmux r3,r4,r5
        stdfcmux r3,r4,r5

        blr
