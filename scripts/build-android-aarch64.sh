#!/bin/sh

git submodule init && git submodule update
wget https://dl.google.com/android/repository/android-ndk-r10e-linux-x86_64.zip -o /dev/null
unzip android-ndk-r10e-linux-x86_64.zip > /dev/null 2>&1
export ANDROID_NDK_HOME=$PWD/android-ndk-r10e/
export NDK_HOME=$PWD/android-ndk-r10e/
./waf configure -T release --android=aarch64,4.9,21 --togles --disable-warns --enable-speex --enable-opus --build-games=cstrike --prefix=./android_aarch64_build &&
./waf install
