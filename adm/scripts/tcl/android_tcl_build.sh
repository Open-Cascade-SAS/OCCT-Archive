#!/bin/bash

# This is helpful script to perform building of Tcl for Android platform
# https://www.tcl.tk/software/tcltk/download.html

#$HOME/develop/android-ndk-r12b/build/tools/make-standalone-toolchain.sh --platform=android-15 --install-dir=$HOME/develop/android15-armv7a  --ndk-dir=$HOME/develop/android-ndk-r12b --toolchain=arm-linux-androideabi-4.9
#$HOME/develop/android-ndk-r12b/build/tools/make-standalone-toolchain.sh --platform=android-21 --install-dir=$HOME/develop/android21-aarch64 --ndk-dir=$HOME/develop/android-ndk-r12b --toolchain=aarch64-linux-android-4.9
#$HOME/develop/android-ndk-r12b/build/tools/make-standalone-toolchain.sh --platform=android-15 --install-dir=$HOME/develop/android15-x86     --ndk-dir=$HOME/develop/android-ndk-r12b --toolchain=x86-4.9
ANDROID_ROOT="/home/develop/android"

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then
  cd "$aScriptPath"
fi

# define number of jobs from available CPU cores
aNbJobs="$(getconf _NPROCESSORS_ONLN)"
set -o pipefail

aPathBak="$PATH"
aTclRoot="$PWD"

OUTPUT_FOLDER="$aTclRoot/install/tcl-android"
rm -f -r "$OUTPUT_FOLDER"
mkdir -p "$OUTPUT_FOLDER"
cp -f    "$aTclRoot/license.terms" "$OUTPUT_FOLDER"
cp -f    "$aTclRoot/README.md"     "$OUTPUT_FOLDER"
echo "Output directory: $OUTPUT_FOLDER"

set -o pipefail

function buildArch {
  anAbi=$1
  anArch=$2
  aToolPath=$3
  aCFlags=$4
  export "PATH=$ANDROID_ROOT/$aToolPath/bin:$aPathBak"
  export "CC=$anArch-gcc"
  export "AR=$anArch-ar"
  export "RANLIB=$anArch-ranlib"
  export "CFLAGS=$aCFlags"
  pushd "$aTclRoot/unix"
  ./configure --build x86_64-linux --host $anArch --prefix=${OUTPUT_FOLDER} --libdir=${OUTPUT_FOLDER}/libs/$anAbi --bindir=${OUTPUT_FOLDER}/bins/$anAbi 2>&1 | tee $OUTPUT_FOLDER/config-$anAbi.log
  aResult=$?; if [[ $aResult != 0 ]]; then echo "FAILED configure $anAbi"; exit $aResult; fi
  make clean
  make -j$aNbJobs
  aResult=$?; if [[ $aResult != 0 ]]; then echo "FAILED make $anAbi"; exit $aResult; fi
  make install
  popd
  export "PATH=$aPathBak"
}

buildArch "arm64-v8a"   "aarch64-linux-android" "android21-aarch64" "-O2 -march=armv8-a"
buildArch "armeabi-v7a" "arm-linux-androideabi" "android15-armv7a"  "-O2 -march=armv7-a -mfloat-abi=softfp"
buildArch "x86"         "i686-linux-android"    "android15-x86"     "-O2 -march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"
buildArch "x86_64"      "x86_64-linux-android"  "android21-x86_64"  "-O2 -march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel"

rm $OUTPUT_FOLDER/../tcl-android.7z &>/dev/null
7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../tcl-android.7z $OUTPUT_FOLDER
