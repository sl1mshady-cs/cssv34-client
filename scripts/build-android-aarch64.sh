#!/bin/sh

git submodule init && git submodule update
wget https://dl.google.com/android/repository/android-ndk-r20b-linux-x86_64.zip -o /dev/null
unzip android-ndk-r20b-linux-x86_64.zip
export ANDROID_NDK_HOME=$PWD/android-ndk-r20b/
export NDK_HOME=$PWD/android-ndk-r20b/
./waf configure -T release --android=aarch64,4.9,32 --togles --disable-warns --enable-speex --enable-opus --build-games=cstrike --prefix=./android_aarch64_build &&
./waf install
