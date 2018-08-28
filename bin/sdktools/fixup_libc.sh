#! /bin/sh
#This script fixes up libc.a for use in a shared library.
#fixup_libc.sh <path_to_libc_file> <path_to_output_dir>
#fixup_libc.sh $RTEMS_TOOLS/arm-rtems4.11/lib/libc.a $RTEMS_TOOLS/arm-rtems4.11/lib/slac_modified
cd $2
cp $1 ./

#The following objects are overidden by rtems and must be removed from the archive.
echo "Updating archive libc.a..."
arm-rtems4.11-ar -v -d libc.a \
lib_a-access.o \
lib_a-envlock.o \
lib_a-calloc.o \
lib_a-linkr.o \
lib_a-malloptr.o \
lib_a-reallocr.o \
lib_a-callocr.o  \
lib_a-malignr.o \
lib_a-mallstatsr.o \
lib_a-signalr.o \
lib_a-cfreer.o \
lib_a-mallinfor.o \
lib_a-msizer.o \
lib_a-vallocr.o \
lib_a-freer.o \
lib_a-mallocr.o \
lib_a-pvallocr.o \
lib_a-getreent.o
