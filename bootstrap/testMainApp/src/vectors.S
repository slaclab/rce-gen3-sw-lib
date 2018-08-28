        .section .inproc,"ax"

        .global readPage,readPageNoECC,writePage,eraseBlock
        .global readFile

readPage:       blr

readPageNoECC:  blr

writePage:      blr

eraseBlock:     blr

readFile:       blr

restart:        blr



        .section .xfrvec,"ax"

        // XFRVEC - Transfer vector for flash memory functions
XFRVEC: b       readPage          // Join variable-location code
        b       readPageNoECC     // Join variable-location code
        b       writePage         // Join variable-location code
        b       eraseBlock        // Join variable-location code
        b       readFile          // Join variable-location code
        b       restart           // Join variable-location code

.Lxfrvec:
        .size   XFRVEC,.Lxfrvec-XFRVEC
