#!/bin/bash

# Auxiliary script for semi-automated building building of Bullet for macOS platform.
# Script should be placed into root of bullet repository, edited with paths to CMake.
# https://github.com/bulletphysics/bullet3/releases

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then cd "$aScriptPath"; fi
aScriptPath="$PWD"
aProjName=${PWD##*/}

aBulletSrc=$aScriptPath
aNbJobs="$(getconf _NPROCESSORS_ONLN)"

PATH=/Applications/CMake.app/Contents/bin:$PATH

# build stages to perform
toCMake=1
toClean=1
toMake=1
toInstall=1
toPack=1

export MACOSX_DEPLOYMENT_TARGET=10.10
anAbi=arm64
#anAbi=x86_64
aPlatformAndCompiler=macos-$anAbi
aWorkDir=work/$aProjName-${aPlatformAndCompiler}-make
aDestDir=$aBulletSrc/work/$aProjName-$aPlatformAndCompiler
aLogFile=$aBulletSrc/build-${aPlatformAndCompiler}.log

mkdir -p $aWorkDir
rm    -f $aLogFile

pushd $aWorkDir

aTimeZERO=$SECONDS
set -o pipefail

function logDuration {
  if [[ $1 == 1 ]]; then
    aDur=$(($4 - $3))
    echo $2 time: $aDur sec>> $aLogFile
  fi
}

# (re)generate Make files
if [[ $toCMake == 1 ]]; then
  echo Configuring Bullet for OS X...
  cmake -G "Unix Makefiles" \
  -D CMAKE_BUILD_TYPE:STRING="Release" \
  -D BUILD_LIBRARY_TYPE:STRING="Static" \
  -D CMAKE_OSX_ARCHITECTURES:STRING="$anAbi" \
  -D CMAKE_INSTALL_PREFIX:PATH="$aDestDir" \
  -D LIBRARY_OUTPUT_PATH:PATH="$aDestDir/lib" \
  -D LIB_DESTINATION:STRING="lib" \
  -D INCLUDE_INSTALL_DIR:STRING="$aDestDir/include/bullet" \
  -D BUILD_BULLET3:BOOL="ON" \
  -D BUILD_CPU_DEMOS:BOOL="OFF" \
  -D BUILD_EXTRAS:BOOL="OFF" \
  -D BUILD_BULLET2_DEMOS:BOOL="OFF" \
  -D BUILD_OPENGL3_DEMOS:BOOL="OFF" \
  -D BUILD_PYBULLET:BOOL="OFF" \
  -D BUILD_SHARED_LIBS:BOOL="OFF" \
  -D BUILD_UNIT_TESTS:BOOL="OFF" \
  -D BULLET2_USE_THREAD_LOCKS:BOOL="OFF" \
  -D USE_GLUT:BOOL="OFF" \
  -D USE_DOUBLE_PRECISION:BOOL="OFF" \
  -D USE_CUSTOM_VECTOR_MATH:BOOL="OFF" \
  -D USE_GRAPHICAL_BENCHMARK:BOOL="OFF" \
  -D USE_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD:BOOL="OFF" \
 "$aBulletSrc" 2>&1 | tee -a $aLogFile
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
fi
aTimeGEN=$SECONDS
logDuration $toCMake "Generation" $aTimeZERO $aTimeGEN

# clean up from previous build
if [[ $toClean == 1 ]]; then
  make clean
fi

# build the project
if [[ $toMake == 1 ]]; then
  echo Building...
  make -j $aNbJobs 2>&1 | tee -a $aLogFile
  aResult=$?; if [[ $aResult != 0 ]]; then exit $aResult; fi
fi
aTimeBUILD=$SECONDS
logDuration $toMake "Building"       $aTimeGEN  $aTimeBUILD
logDuration $toMake "Total building" $aTimeZERO $aTimeBUILD

# install the project
if [[ $toInstall == 1 ]]; then
  echo Installing into $aBulletSrc/work/$aPlatformAndCompiler...
  make install 2>&1 | tee -a $aLogFile
  xcodebuild -version > $aDestDir/config.log
  clang --version >> $aDestDir/config.log
  echo MACOSX_DEPLOYMENT_TARGET=$MACOSX_DEPLOYMENT_TARGET>> $aDestDir/config.log
  cp -f $aBulletSrc/VERSION $aDestDir/VERSION
  cp -f $aBulletSrc/LICENSE.txt $aDestDir/LICENSE.txt
fi
aTimeINSTALL=$SECONDS
logDuration $toInstall "Install" $aTimeBUILD $aTimeINSTALL

# create an archive
if [[ $toPack == 1 ]]; then
  anArchName=$aProjName-$aPlatformAndCompiler.tar.bz2
  echo Creating an archive $aBulletSrc/work/$anArchName...
  rm $aBulletSrc/work/$anArchName &>/dev/null
  pushd $aDestDir
  tar -jcf $aBulletSrc/work/$anArchName *
  popd
fi
aTimePACK=$SECONDS
logDuration $toPack "Packing archive" $aTimeINSTALL $aTimePACK

# finished
DURATION=$(($aTimePACK - $aTimeZERO))
echo Total time: $DURATION sec
logDuration 1 "Total" $aTimeZERO $aTimePACK

popd
