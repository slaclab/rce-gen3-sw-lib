#! /bin/sh
#This script creates an SDK dev rtems directory containing all required
#external libraries to support building of rtems.so.
#create_sdkdev_rtems.sh <path_to_rtems_installation> <bsp_variant> <path_to_output_dir>
#create_sdkdev_rtems.sh $RTEMS_ROOT xilinx_zynq_ml1_opt $SDKDEV

echo "Fixing up and copying $2 RTEMS includes and libraries to $3..."

cd $3

mkdir $3/$2

fixup_rtems.sh $1/bld/arm-rtems4.11/c/$2 $3/$2

cp -r $1/tgt/arm-rtems4.11/$2/lib/include $3
