#!/bin/bash

# taken from https://github.com/floooh/sokol/blob/master/tests/test_common.sh

setup_android() {
	if [ ! -d "build/android_sdk" ] ; then
		mkdir -p build/android_sdk && cd build/android_sdk
		sdk_file="commandlinetools-linux-11076708_latest.zip"
		wget --no-verbose https://dl.google.com/android/repository/$sdk_file
		unzip -q $sdk_file
		cd cmdline-tools/bin
		echo "Install platform android"
		yes | ./sdkmanager --sdk_root=. "platforms;android-28" >/dev/null

		echo "Install build-tools"
		yes | ./sdkmanager --sdk_root=. "build-tools;29.0.3" >/dev/null

		echo "Install platform-tools"
		yes | ./sdkmanager --sdk_root=. "platform-tools" >/dev/null

		echo "Install ndk-bundle"
		yes | ./sdkmanager --sdk_root=. "ndk-bundle" >/dev/null
		cd ../../../..
	fi
}

setup_android
