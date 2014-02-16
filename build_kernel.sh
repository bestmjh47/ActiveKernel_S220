#!/bin/sh
#############################
# Take LTE SH Kernel Source #             
# http://theteamdev.com     #
# & bestmjh47               #
#############################
TOOLCHAINPATH=/home/moon/toolchain/arm-eabi-4.4.3/bin
export ARCH=arm
#export CROSS_COMPILE=/home/moon/toolchain/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-
export CROSS_COMPILE=$TOOLCHAINPATH/arm-aebi-
#export CROSS_COMPILE=/home/moon/toolchain/android-toolchain-eabi-4.8/bin/arm-eabi-
make msm8660_o4_es2_defconfig
echo #############################
echo #       Now Starting...     #
echo #############################
make -j15
echo Compiling Finished!
cp arch/arm/boot/zImage zImage
echo Striping Modules...
rm -rf modules/*
find -name '*.ko' -exec cp -av {} Modules \;
        for i in Modules/*; do $TOOLCHAINPATH/arm-eabi-strip --strip-unneeded $i;done;\

echo ""
echo Done! zImage and modules are READY!!!
