#!/bin/sh

git submodule init && git submodule update

wget https://dl.google.com/android/repository/android-ndk-r27d-linux.zip -O ndk.zip > /dev/null 2>&1
unzip -q ndk.zip
export ANDROID_NDK_HOME=$PWD/android-ndk-r27d
export NDK_HOME=$PWD/android-ndk-r27d

mkdir -p $PWD/android-sdk
cd $PWD/android-sdk
wget https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip -O cmdline-tools.zip > /dev/null 2>&1
unzip -q cmdline-tools.zip
mkdir -p cmdline-tools/latest
mv cmdline-tools/* cmdline-tools/latest/ 2>/dev/null
export ANDROID_HOME=$PWD
export PATH=$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools:$PATH
cd ..

yes | sdkmanager --licenses > /dev/null 2>&1
sdkmanager "platform-tools" "platforms;android-21" "build-tools;34.0.0" > /dev/null 2>&1

./waf configure -T release --android=armeabi-v7a,clang,21 --togles --disable-warns --enable-speex --enable-opus --build-games=cstrike --prefix=./android_armv7a_build -vvv &&
./waf install