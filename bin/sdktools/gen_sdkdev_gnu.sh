#! /bin/sh
#This script creates an SDK dev gnu directory containing all required
#external libraries to support building of rtems.so.
#create_sdkdev_gnu.sh <path_to_libc_dir> <path_to_libgcc_dir> <path_to_gnu_output_dir>
#create_sdkdev_gnu.sh $RTEMS_TOOLS/arm-rtems4.11/lib/ $RTEMS_TOOLS/lib/gcc/arm-rtems4.11/4.8.1 $SDKDEV/gnu

echo "Fixing up and copying required GNU libraries to $3..."

fixup_libc.sh $1/libc.a $3

fixup_libgcc.sh $2/libgcc.a $3

cp $1/crt0.o $3
cp $1/libm.a $3

cp $2/crtbegin.o $3
cp $2/crtend.o $3
cp $2/crti.o $3
cp $2/crtn.o $3
cp $2/libstdc++.a $3
