#!/bin/sh

# Copy the extracted packages to folders named like these to make it easier
LIBEVENT_DIR=libevent           # libevent
LIBFLUID_BASE_DIR=libfluid_base # libfluid_base
LIBFLUID_MSG_DIR=libfluid_msg   # libfluid_base
NDK_ROOT=~/android/ndk          # Path to the Android NDK root
#LIBSSL_DIR=libssl               # OpenSSL

openlib() {
    if [ ! -d "$2" ]; then
        echo "$1 not found.\nPlace it at `pwd`/$2";
        exit
    fi
    cd $2
    make clean; make distclean
}

checkerror() {
    if [ "$?" -ne 0 ]; then
        echo "Error building $1. See log above.";
        exit
    fi
}

export PATH=$PATH:$NDK_ROOT/toolchains/arm-linux-androideabi-4.7/prebuilt/linux-x86_64/bin
export PATH=$PATH:$NDK_ROOT/toolchains/arm-linux-androideabi-4.7/prebuilt/linux-x86_64/include

#openlib "libssl" $LIBSSL_DIR
#export CC="arm-linux-androideabi-gcc --sysroot=$NDK_ROOT/platforms/android-8/arch-arm/"
#./Configure android-armv7 no-asm shared
#checkerror "libssl"
#make
#checkerror "libssl"
#cd -

openlib "libevent" $LIBEVENT_DIR
./configure --host=arm-linux-androideabi CC=arm-linux-androideabi-gcc LD=arm-linux-androideabi-ld CFLAGS="--sysroot=$NDK_ROOT/platforms/android-8/arch-arm/"
checkerror "libevent"
make
checkerror "libevent"
cd -

openlib "libfluid_base" $LIBFLUID_BASE_DIR
./configure --host=arm-linux-androideabi --disable-tls CXX=arm-linux-androideabi-g++ LD=arm-linux-androideabi-ld CXXFLAGS="--sysroot=$NDK_ROOT/platforms/android-8/arch-arm/ -I../$LIBEVENT_DIR/include -L`pwd`/../$LIBEVENT_DIR -I$NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.7/include -I$NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.7/include/backward -I$NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi-v7a/include"
checkerror "libfluid_base"
make
checkerror "libfluid_base"
cd -

openlib "libfluid_msg" $LIBFLUID_MSG_DIR
./configure --host=arm-linux-androideabi CC=arm-linux-androideabi-gcc CXX=arm-linux-androideabi-g++ LD=arm-linux-androideabi-ld CFLAGS="--sysroot=$NDK_ROOT/platforms/android-8/arch-arm/" CXXFLAGS="--sysroot=$NDK_ROOT/platforms/android-8/arch-arm/ -I$NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.7/include -I$NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.7/include/backward -I$NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.7/libs/armeabi-v7a/include"
checkerror "libfluid_msg"
make
checkerror "libfluid_msg"
cd -
