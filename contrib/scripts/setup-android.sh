#!/bin/bash

# taken from https://github.com/floooh/sokol/blob/master/tests/test_common.sh

setup_android() {
    if [ ! -d "build/android_sdk" ] ; then
        mkdir -p build/android_sdk && cd build/android_sdk
        sdk_file="sdk-tools-linux-3859397.zip"
        wget --no-verbose https://dl.google.com/android/repository/$sdk_file
        unzip -q $sdk_file
        cd tools/bin
        yes | ./sdkmanager "platforms;android-28" >/dev/null
        yes | ./sdkmanager "build-tools;29.0.3" >/dev/null
        yes | ./sdkmanager "platform-tools" >/dev/null
        yes | ./sdkmanager "ndk-bundle" >/dev/null
        cd ../../../..
    fi
}

setup_android
