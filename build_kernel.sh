#!/bin/sh
############################
# Take Tachy Kernel Source #             
#                          #
#         bestmjh47        #
############################
TOOLCHAINPATH=/home/moon/toolchain/linaro-4.8.3/bin
export ARCH=arm
#export CROSS_COMPILE=/home/moon/toolchain/arm-linux-androideabi-4.6/bin/arm-linux-androideabi-
export CROSS_COMPILE=$TOOLCHAINPATH/arm-gnueabi-
#export CROSS_COMPILE=/home/moon/toolchain/android-toolchain-eabi-4.8/bin/arm-eabi-
make bestmjh47_defconfig
echo #############################
echo #       Now Starting...     #
echo #############################
make -j15
echo Compiling Finished!
cp arch/arm/boot/zImage zImage
echo Striping Modules...
rm -rf modules/*
find -name '*.ko' -exec cp -av {} modules \;
        for i in modules/*; do $TOOLCHAINPATH/arm-gnueabi-strip --strip-unneeded $i;done;\
echo ""
echo Done! zImage and modules are READY!!!
echo ""
echo Making bootimage...
mkbootimg --cmdline "androidboot.hardware=qcom no_console_suspend=1" --base 0x40200000 --ramdiskaddr 0x41500000 --pagesize 2048 --kernel zImage --ramdisk ramdisk/bestmjh47-ramdisk.gz -o boot-bestmjh47.img
