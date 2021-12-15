#!/bin/bash

# Auxiliary script for building FreeType library for iOS platform.

aScriptDir=${BASH_SOURCE%/*}
if [ -d "$aScriptDir" ]; then cd "$aScriptDir"; fi
aScriptDir="$PWD"

aFreeType=freetype-2.10.4
aFreeTypeSrc=${aScriptDir}/${aFreeType}-src

PATH=/Applications/CMake.app/Contents/bin:$PATH
PLATFORMPATH="/Applications/Xcode.app/Contents/Developer/Platforms"
export IPHONEOS_DEPLOYMENT_TARGET=8.0

rm -r -f ${aFreeType}-ios
mkdir -p ${aFreeType}-ios/lib
rm -r -f ${aFreeType}-iPhoneSimulator
mkdir -p ${aFreeType}-iPhoneSimulator/lib

function buildArchCmake {
  anAbi=$1
  aPlatformSdk=$2
  aSysRoot=$PLATFORMPATH/${aPlatformSdk}.platform/Developer/SDKs/${aPlatformSdk}.sdk

  aPlatformAndCompiler=${aPlatformSdk}-${anAbi}-clang
  
  aWorkDir="${aScriptDir}/${aFreeType}-${aPlatformAndCompiler}-make"
  aDestDir="${aScriptDir}/${aFreeType}-${aPlatformAndCompiler}"
  aLogFile="${aScriptDir}/${aFreeType}-build-${aPlatformAndCompiler}.log"

  rm -r -f "$aWorkDir"
  rm -r -f "$aDestDir"
  mkdir -p "$aWorkDir"
  mkdir -p "$aDestDir"
  rm -f "$aLogFile"

  pushd ${aWorkDir}
  cmake -G "Unix Makefiles" \
  -D CMAKE_SYSTEM_NAME="iOS" \
  -D CMAKE_OSX_ARCHITECTURES:STRING="$anAbi" \
  -D CMAKE_OSX_DEPLOYMENT_TARGET:STRING="$IPHONEOS_DEPLOYMENT_TARGET" \
  -D CMAKE_OSX_SYSROOT:PATH="$aSysRoot" \
  -D CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS:BOOL="OFF" \
  -D CMAKE_BUILD_TYPE:STRING="Release" \
  -D CMAKE_INSTALL_PREFIX:PATH="$aDestDir" \
  -D BUILD_SHARED_LIBS:BOOL="OFF" \
  -D FT_WITH_ZLIB:BOOL="OFF" \
  -D FT_WITH_BZIP2:BOOL="OFF" \
  -D FT_WITH_PNG:BOOL="OFF" \
  -D FT_WITH_HARFBUZZ:BOOL="OFF" \
  -D FT_WITH_BROTLI:BOOL="OFF" \
  "$aFreeTypeSrc" 2>&1 | tee -a "$aLogFile"
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
  
  make clean

  echo Building...
  make 2>&1 | tee -a "$aLogFile"
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi

  make install
  echo Installing into $aDestDir...
  make install 2>&1 | tee -a "$aLogFile"

  popd
}

buildArchCmake arm64  iPhoneOS
buildArchCmake arm64  iPhoneSimulator
buildArchCmake x86_64 iPhoneSimulator

aTargName=ios
cp -rf ./${aFreeType}-src/include      ${aFreeType}-${aTargName}/
cp ./${aFreeType}-src/docs/FTL.TXT     ${aFreeType}-${aTargName}/
cp ./${aFreeType}-src/docs/GPLv2.TXT   ${aFreeType}-${aTargName}/
cp ./${aFreeType}-src/docs/LICENSE.TXT ${aFreeType}-${aTargName}/
cp ./${aFreeType}-src/docs/CHANGES     ${aFreeType}-${aTargName}/
cp ./${aFreeType}-src/docs/README      ${aFreeType}-${aTargName}/
lipo -create -output ${aFreeType}-${aTargName}/lib/libfreetype.a ${aFreeType}-iPhoneOS-arm64-clang/lib/libfreetype.a 
lipo -info ${aFreeType}-${aTargName}/lib/libfreetype.a

anArchName=${aFreeType}-${aTargName}.tar.bz2
rm ${aDestDir}/../${anArchName} &>/dev/null
pushd "./${aFreeType}-${aTargName}"
tar -jcf ${aScriptDir}/${anArchName} *
popd

aTargName=iPhoneSimulator
cp -rf ./${aFreeType}-src/include      ${aFreeType}-${aTargName}/
cp ./${aFreeType}-src/docs/FTL.TXT     ${aFreeType}-${aTargName}/
cp ./${aFreeType}-src/docs/GPLv2.TXT   ${aFreeType}-${aTargName}/
cp ./${aFreeType}-src/docs/LICENSE.TXT ${aFreeType}-${aTargName}/
cp ./${aFreeType}-src/docs/CHANGES     ${aFreeType}-${aTargName}/
cp ./${aFreeType}-src/docs/README      ${aFreeType}-${aTargName}/
lipo -create -output ${aFreeType}-${aTargName}/lib/libfreetype.a ${aFreeType}-iPhoneSimulator-arm64-clang/lib/libfreetype.a ${aFreeType}-iPhoneSimulator-x86_64-clang/lib/libfreetype.a
lipo -info ${aFreeType}-${aTargName}/lib/libfreetype.a

anArchName=${aFreeType}-${aTargName}.tar.bz2
rm ${aDestDir}/../${anArchName} &>/dev/null
pushd "./${aFreeType}-${aTargName}"
tar -jcf ${aScriptDir}/${anArchName} *
popd
