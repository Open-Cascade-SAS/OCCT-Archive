#!/bin/bash

# This is helpful script to perform building of liblzma for Android platform
# http://tukaani.org/xz/

anNdkRoot=/home/develop/android/android-ndk-r12b
anNdkArm32=/home/develop/android/android15-armv7a
anNdkArm64=/home/develop/android/android21-aarch64
anNdkx86=/home/develop/android/android15-x86
anNdkx86_64=/home/develop/android/android21-x86_64
#$anNdkRoot/build/tools/make-standalone-toolchain.sh --platform=android-15 --install-dir=$anNdkArm32  --ndk-dir=$anNdkRoot --toolchain=arm-linux-androideabi-4.9
#$anNdkRoot/build/tools/make-standalone-toolchain.sh --platform=android-21 --install-dir=$anNdkArm64  --ndk-dir=$anNdkRoot --toolchain=aarch64-linux-android-4.9
#$anNdkRoot/build/tools/make-standalone-toolchain.sh --platform=android-15 --install-dir=$anNdkx86    --ndk-dir=$anNdkRoot --toolchain=x86-4.9
#$anNdkRoot/build/tools/make-standalone-toolchain.sh --platform=android-21 --install-dir=$anNdkx86_64 --ndk-dir=$anNdkRoot --toolchain=x86_64-4.9

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then
  cd "$aScriptPath"
fi

# define number of jobs from available CPU cores
aNbJobs="$(getconf _NPROCESSORS_ONLN)"
set -o pipefail

aPathBak="$PATH"
anXzRoot="$PWD"
aCFlagsArmv7a="-O2 -march=armv7-a -mfloat-abi=softfp"
aCFlagsArmv8a="-O2 -march=armv8-a"
aCFlagsx86="-O2 -march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"
aCFlagsx86_64="-O2 -march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel"

OUTPUT_FOLDER="$anXzRoot/install/liblzma-android"
rm -f -r "$OUTPUT_FOLDER"
mkdir -p "$OUTPUT_FOLDER/include"
mkdir -p "$OUTPUT_FOLDER/libs/armeabi-v7a"
mkdir -p "$OUTPUT_FOLDER/libs/x86"
mkdir -p "$OUTPUT_FOLDER/libs/arm64-v8a"
mkdir -p "$OUTPUT_FOLDER/libs/x86_64"
cp -f    "$anXzRoot/COPYING"                "$OUTPUT_FOLDER"
cp -f    "$anXzRoot/README"                 "$OUTPUT_FOLDER"
cp -f    "$anXzRoot/src/liblzma/api/lzma.h" "$OUTPUT_FOLDER/include"
cp -f -r "$anXzRoot/src/liblzma/api/lzma"   "$OUTPUT_FOLDER/include"
echo "Output directory: $OUTPUT_FOLDER"

# armv7a
export "PATH=$anNdkArm32/bin:$aPathBak"
export "CC=arm-linux-androideabi-gcc"
export "CXX=arm-linux-androideabi-g++"
export "CFLAGS=$aCFlagsArmv7a"
export "CXXFLAGS=$aCFlagsArmv7a"
./configure --host arm-linux-androideabi 2>&1 | tee $OUTPUT_FOLDER/config-armv7a.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$anXzRoot/src/liblzma/.libs/liblzma.so" "$OUTPUT_FOLDER/libs/armeabi-v7a"
cp -f "$anXzRoot/src/liblzma/.libs/liblzma.a"  "$OUTPUT_FOLDER/libs/armeabi-v7a"

# x86
export "PATH=$anNdkx86/bin:$aPathBak"
export "CC=i686-linux-android-gcc"
export "CXX=i686-linux-android-g++"
export "CFLAGS=$aCFlagsx86"
export "CXXFLAGS=$aCFlagsx86"
./configure --host i686-linux-android 2>&1 | tee $OUTPUT_FOLDER/config-x86.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$anXzRoot/src/liblzma/.libs/liblzma.so" "$OUTPUT_FOLDER/libs/x86"
cp -f "$anXzRoot/src/liblzma/.libs/liblzma.a"  "$OUTPUT_FOLDER/libs/x86"

# armv8a
export "PATH=$anNdkArm64/bin:$aPathBak"
export "CC=aarch64-linux-android-gcc"
export "CXX=aarch64-linux-android-g++"
export "CFLAGS=$aCFlagsArmv8a"
export "CXXFLAGS=$aCFlagsArmv8a"
./configure --host aarch64-linux-android 2>&1 | tee $OUTPUT_FOLDER/config-aarch64.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$anXzRoot/src/liblzma/.libs/liblzma.so" "$OUTPUT_FOLDER/libs/arm64-v8a"
cp -f "$anXzRoot/src/liblzma/.libs/liblzma.a"  "$OUTPUT_FOLDER/libs/arm64-v8a"

# x86_64
export "PATH=$anNdkx86_64/bin:$aPathBak"
export "CC=x86_64-linux-android-gcc"
export "CXX=x86_64-linux-android-g++"
export "CFLAGS=$aCFlagsx86_64"
export "CXXFLAGS=$aCFlagsx86_64"
./configure --host x86_64-linux-android 2>&1 | tee $OUTPUT_FOLDER/config-x86_64.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
cp -f "$anXzRoot/src/liblzma/.libs/liblzma.so" "$OUTPUT_FOLDER/libs/x86_64"
cp -f "$anXzRoot/src/liblzma/.libs/liblzma.a"  "$OUTPUT_FOLDER/libs/x86_64"

export "PATH=$aPathBak"

rm $OUTPUT_FOLDER/../liblzma-android.7z &>/dev/null
7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../liblzma-android.7z $OUTPUT_FOLDER
