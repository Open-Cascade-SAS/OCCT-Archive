#!/bin/bash

# This is helpful script to perform building of liblzma for macOS platform
# http://tukaani.org/xz/

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then
  cd "$aScriptPath"
fi

# define number of jobs from available CPU cores
aNbJobs="$(getconf _NPROCESSORS_ONLN)"
set -o pipefail

anXzRoot="$PWD"
export MACOSX_DEPLOYMENT_TARGET=10.10

OUTPUT_FOLDER="$anXzRoot/work/liblzma-macos"
rm -f -r "$OUTPUT_FOLDER"
mkdir -p "$OUTPUT_FOLDER/include"
mkdir -p "$OUTPUT_FOLDER/lib"
cp -f    "$anXzRoot/COPYING"                "$OUTPUT_FOLDER"
cp -f    "$anXzRoot/README"                 "$OUTPUT_FOLDER"
cp -f    "$anXzRoot/src/liblzma/api/lzma.h" "$OUTPUT_FOLDER/include"
cp -f -r "$anXzRoot/src/liblzma/api/lzma"   "$OUTPUT_FOLDER/include"
echo "Output directory: $OUTPUT_FOLDER"

# x86_64
./configure CFLAGS="-O2 -arch x86_64" CXXFLAGS="-O2 -arch x86_64" 2>&1 | tee $OUTPUT_FOLDER/config-x86_64.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
#cp -f "$anXzRoot/src/liblzma/.libs/liblzma.so" "$OUTPUT_FOLDER/libs/x86"
cp -f "$anXzRoot/src/liblzma/.libs/liblzma.a"  "$OUTPUT_FOLDER/lib/liblzma-x86_64.a"

# armv8a
./configure CFLAGS="-O2 -arch arm64" CXXFLAGS="-O2 -arch arm64" 2>&1 | tee $OUTPUT_FOLDER/config-arm64.log
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
make clean
make -j$aNbJobs
aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
#cp -f "$anXzRoot/src/liblzma/.libs/liblzma.so" "$OUTPUT_FOLDER/libs/arm64-v8a"
cp -f "$anXzRoot/src/liblzma/.libs/liblzma.a"  "$OUTPUT_FOLDER/lib/liblzma-arm64.a"

lipo -create -output "$OUTPUT_FOLDER/lib/liblzma.a" "$OUTPUT_FOLDER/lib/liblzma-x86_64.a" "$OUTPUT_FOLDER/lib/liblzma-arm64.a"
lipo -info "$OUTPUT_FOLDER/lib/liblzma.a"

#rm $OUTPUT_FOLDER/../liblzma-macos.7z &>/dev/null
#7za a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on $OUTPUT_FOLDER/../liblzma-macos.7z $OUTPUT_FOLDER
