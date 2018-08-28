#!/bin/bash
#
# Build the hello world example for RTEMS
#
# To run this, you need to be in the directory where you found this script. 


#set -x
#Compile the objects
rtems-gcc  hello_so_1.c        -I${RTEMS_SDK}/include/core \
                               -I${RTEMS_SDK}/include/rtems \
                               -o hello_so_1.o

rtems-gcc  hello_so_2.c        -I${RTEMS_SDK}/include/core \
                               -I${RTEMS_SDK}/include/rtems \
                               -o hello_so_2.o

#Link the .so images
rtems-ld   hello_so_1.o        -L${RTEMS_SDK}/lib \
                               -L${RTEMS_SDK}/tgt/rtems \
                               -l:rtems.so \
                               -Wl,-soname,examples:hello_1.so \
                               -o hello_1.so

rtems-ld   hello_so_2.o        -L${RTEMS_SDK}/lib \
                               -L${RTEMS_SDK}/tgt/rtems \
                               -l:rtems.so \
                               -Wl,-soname,examples:hello_2.so \
                               -o hello_2.so

#Compile the .exe object
rtems-gcc  hello_task.c        -I${RTEMS_SDK}/include/core \
                               -I${RTEMS_SDK}/include/rtems \
                               -o hello_task.o

#Compile the .svt object
rtems-gcc  hello_svt.c         -I${RTEMS_SDK}/include/core \
                               -o hello_svt.o

#Link the .svt image
rtems-svt  hello_svt.o         -L${RTEMS_SDK}/lib \
                               -l:rtems.so \
                               -Wl,-soname,examples:hello.svt \
                               -o hello.svt 

#Link the .exe images
rtems-task hello_task.o        -L${RTEMS_SDK}/lib \
                               -l:rtems.so -l:hello_1.so \
                               -Wl,-soname,examples:hello_1.exe \
                               -o hello_1.exe

rtems-task hello_task.o        -L${RTEMS_SDK}/lib \
                               -l:rtems.so -l:hello_2.so \
                               -Wl,-soname,examples:hello_2.exe \
                               -o hello_2.exe

[ ! -e ../compiled ] && mkdir ../compiled

cp *.o *.so *.svt *.exe ../compiled

rm *.o *.so *.svt *.exe 
