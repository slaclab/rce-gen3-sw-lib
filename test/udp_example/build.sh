#!/bin/bash
#
# Build the Pseudo ARP example 
#
# To run this, you need to be in the directory where you found this script. 


#set -x
#Compile the objects
rtems-g++  Socket.c            -I${RTEMS_SDK}/include/core \
                               -I${RTEMS_SDK}/include/rtems \
                               -I../arp_example \
                               -o Socket.o

#Link the .so image
rtems-ld   Socket.o            -L${RTEMS_SDK}/lib \
                               -L${RTEMS_SDK}/tgt/rtems \
                               -L../compiled \
                               -l:rtems.so \
                               -l:xaui.so \
                               -l:sas.so \
                               -l:dsl.so \
                               -l:pseudoarp.so \
                               -Wl,-soname,examples:socket.so \
                               -o socket.so
#Compile the .exe object
rtems-g++  Rx.c         -I${RTEMS_SDK}/include/core \
                               -I${RTEMS_SDK}/include/rtems \
                               -I../arp_example \
                               -o Rx.o

#Link the .exe image
rtems-task Rx.o          -L${RTEMS_SDK}/lib \
                               -L${RTEMS_SDK}/tgt/rtems \
                               -L${RTEMS_SDK}/tgt/rtems \
                               -l:rtems.so \
                               -l:xaui.so \
                               -l:sas.so \
                               -l:socket.so \
                               -Wl,-soname,examples:rx.exe \
                               -o rx.exe


#Compile the .exe object
rtems-g++  Tx.c         -I${RTEMS_SDK}/include/core \
                               -I${RTEMS_SDK}/include/rtems \
                               -I../arp_example \
                               -o Tx.o

#Link the .exe image
rtems-task Tx.o          -L${RTEMS_SDK}/lib \
                               -L${RTEMS_SDK}/tgt/rtems \
                               -L${RTEMS_SDK}/tgt/rtems \
                               -l:rtems.so \
                               -l:xaui.so \
                               -l:sas.so \
                               -l:socket.so \
                               -l:atca.so \
                               -l:dsl.so \
                               -l:rts.so \
                               -Wl,-soname,examples:tx.exe \
                               -o tx.exe


[ ! -e ../compiled ] && mkdir ../compiled

cp *.o *.so *.exe ../compiled
rm *.o *.so *.exe
