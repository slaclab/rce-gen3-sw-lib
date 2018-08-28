#!/bin/bash
#
# Build the Pseudo ARP example 
#
# To run this, you need to be in the directory where you found this script. 


#set -x
#Compile the objects
rtems-g++  PseudoARP.cc        -I${RTEMS_SDK}/include/core \
                               -I${RTEMS_SDK}/include/rtems \
                               -o PseudoARP.o

#Link the .so image
rtems-ld   PseudoARP.o         -L${RTEMS_SDK}/lib \
                               -L${RTEMS_SDK}/tgt/rtems \
                               -l:rtems.so \
                               -l:rts.so \
                               -l:dsl.so \
                               -l:atca.so \
                               -Wl,-soname,examples:pseudoarp.so \
                               -o pseudoarp.so

#Compile the .exe object
rtems-g++  arp_task.cc         -I${RTEMS_SDK}/include/core \
                               -I${RTEMS_SDK}/include/rtems \
                               -o arp_task.o

#Link the .exe image
rtems-task arp_task.o          -L${RTEMS_SDK}/lib \
                               -L${RTEMS_SDK}/tgt/rtems \
                               -l:rtems.so \
                               -l:rts.so \
                               -l:dsl.so \
                               -l:atca.so \
                               -l:pseudoarp.so \
                               -Wl,-soname,examples:arp_test.exe \
                               -o arp_test.exe


[ ! -e ../compiled ] && mkdir ../compiled

cp *.o *.so *.exe ../compiled
rm *.o *.so *.exe
