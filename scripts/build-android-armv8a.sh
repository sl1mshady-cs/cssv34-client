#!/bin/sh

git submodule init && git submodule update
wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip -o /dev/null
unzip android-ndk-r25c-linux.zip
export ANDROID_NDK_HOME=$PWD/android-ndk-r25c/
export NDK_HOME=$PWD/android-ndk-r25c/
./waf configure -T release --android=aarch64,4.9,32 --togles --disable-warns --enable-speex --enable-opus --build-games=cstrike --prefix=./android_armv8a_build &&
./waf install
