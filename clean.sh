#bin/sh
export BUILD_TOPDIR=$PWD
export STAGING_DIR=$BUILD_TOPDIR/tmp
export MAKECMD="make --silent ARCH=mips CROSS_COMPILE=mips-linux-gnu-"
export CROSS_COMPILE=mips-linux-gnu-

make clean
