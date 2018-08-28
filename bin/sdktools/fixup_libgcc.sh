#! /bin/sh
#This script fixes up libgcc.a for use in a shared library.
#fixup_libgcc.sh <path_to_libgcc_file> <path_to_output_dir>
#fixup_libgcc.sh $RTEMS_TOOLS/lib/gcc/arm-rtems4.11/4.8.1/libgcc.a $RTEMS_TOOLS/lib/gcc/arm-rtems4.11/4.8.1/slac_modified

cd $2
cp $1 ./

echo "Extracting archive libgcc.a..."
mkdir out
arm-rtems4.11-ar -x libgcc.a
echo "Changing symbols from HIDDEN to DEFAULT..."
elfmodify -d __aeabi_uidiv,__aeabi_uidivmod _udivsi3.o out/_udivsi3.o
elfmodify -d __aeabi_idiv,__aeabi_idivmod _divsi3.o out/_divsi3.o
elfmodify -d __aeabi_llsr _lshrdi3.o out/_lshrdi3.o
elfmodify -d __aeabi_lasr _ashrdi3.o out/_ashrdi3.o
elfmodify -d __aeabi_llsl _ashldi3.o out/_ashldi3.o
elfmodify -d __aeabi_dneg _arm_negdf2.o out/_arm_negdf2.o
elfmodify -d __aeabi_drsub,__aeabi_dsub,__aeabi_dadd,__aeabi_ui2d,__aeabi_i2d,__aeabi_f2d,__aeabi_ul2d,__aeabi_l2d _arm_addsubdf3.o out/_arm_addsubdf3.o
elfmodify -d __aeabi_dmul,__aeabi_ddiv _arm_muldivdf3.o out/_arm_muldivdf3.o
elfmodify -d __aeabi_cdrcmple,__aeabi_cdcmpeq,__aeabi_cdcmple,__aeabi_dcmpeq,__aeabi_dcmplt,__aeabi_dcmple,__aeabi_dcmpge,__aeabi_dcmpgt _arm_cmpdf2.o out/_arm_cmpdf2.o
elfmodify -d __aeabi_dcmpun _arm_unorddf2.o out/_arm_unorddf2.o
elfmodify -d __aeabi_d2iz _arm_fixdfsi.o out/_arm_fixdfsi.o 
elfmodify -d __aeabi_d2uiz _arm_fixunsdfsi.o out/_arm_fixunsdfsi.o 
elfmodify -d __aeabi_d2f _arm_truncdfsf2.o out/_arm_truncdfsf2.o 
elfmodify -d __aeabi_fneg _arm_negsf2.o out/_arm_negsf2.o 
elfmodify -d __aeabi_frsub,__aeabi_fsub,__aeabi_fadd,__aeabi_ui2f,__aeabi_i2f,__aeabi_ul2f,__aeabi_l2f _arm_addsubsf3.o out/_arm_addsubsf3.o
elfmodify -d __aeabi_fmul,__aeabi_fdiv _arm_muldivsf3.o out/_arm_muldivsf3.o
elfmodify -d __aeabi_cfrcmple,__aeabi_cfcmpeq,__aeabi_cfcmple,__aeabi_fcmpeq,__aeabi_fcmplt,__aeabi_fcmple,__aeabi_fcmpge,__aeabi_fcmpgt _arm_cmpsf2.o out/_arm_cmpsf2.o 
elfmodify -d __aeabi_fcmpun _arm_unordsf2.o out/_arm_unordsf2.o 
elfmodify -d __aeabi_f2iz _arm_fixsfsi.o out/_arm_fixsfsi.o 
elfmodify -d __aeabi_f2uiz _arm_fixunssfsi.o out/_arm_fixunssfsi.o 
elfmodify -d __aeabi_lcmp _aeabi_lcmp.o out/_aeabi_lcmp.o 
elfmodify -d __aeabi_ulcmp _aeabi_ulcmp.o out/_aeabi_ulcmp.o 
elfmodify -d __aeabi_ldivmod _aeabi_ldivmod.o out/_aeabi_ldivmod.o 
elfmodify -d __aeabi_uldivmod _aeabi_uldivmod.o out/_aeabi_uldivmod.o 
elfmodify -d __aeabi_lmul _muldi3.o out/_muldi3.o
elfmodify -d __aeabi_f2lz _fixsfdi.o out/_fixsfdi.o 
elfmodify -d __aeabi_d2lz _fixdfdi.o out/_fixdfdi.o 
elfmodify -d __aeabi_f2ulz _fixunssfdi.o out/_fixunssfdi.o 
elfmodify -d __aeabi_d2ulz _fixunsdfdi.o out/_fixunsdfdi.o 
elfmodify -d __aeabi_uread4,__aeabi_uwrite4,__aeabi_uread8,__aeabi_uwrite8 unaligned-funcs.o out/unaligned-funcs.o 
elfmodify -d __aeabi_unwind_cpp_pr0,__aeabi_unwind_cpp_pr1,__aeabi_unwind_cpp_pr2 unwind-arm.o out/unwind-arm.o

rm *.o
mv out/*.o ./
rm -rf out

echo "Updating archive libgcc.a..."

#Delete unused thread local storage emulation. Contains unresolved symbols.
arm-rtems4.11-ar -v -d libgcc.a emutls.o

#replace fixed up objects in archive
arm-rtems4.11-ar -v -r libgcc.a \
_aeabi_lcmp.o \
_arm_fixdfsi.o \
_arm_truncdfsf2.o \
_fixunsdfdi.o \
_aeabi_ldivmod.o \
_arm_fixsfsi.o \
_arm_unorddf2.o \
_fixunssfdi.o \
_aeabi_ulcmp.o \
_arm_fixunsdfsi.o \
_arm_unordsf2.o \
_lshrdi3.o \
_aeabi_uldivmod.o \
_arm_fixunssfsi.o \
_ashldi3.o \
_muldi3.o \
_arm_addsubdf3.o \
_arm_muldivdf3.o \
_ashrdi3.o \
_udivsi3.o \
_arm_addsubsf3.o \
_arm_muldivsf3.o \
_divsi3.o \
unaligned-funcs.o \
_arm_cmpdf2.o \
_arm_negdf2.o \
_fixdfdi.o \
unwind-arm.o \
_arm_cmpsf2.o \
_arm_negsf2.o \
_fixsfdi.o \

arm-rtems4.11-ranlib libgcc.a

rm *.o
