#!/bin/bash

# Auxiliary script for semi-automated building of Draco for Linux platform.
# Script should be placed into root of Draco repository, edited with paths to CMake.
# https://github.com/google/draco

# go to the script directory
aScriptPath=${BASH_SOURCE%/*}
if [ -d "$aScriptPath" ]; then cd "$aScriptPath"; fi
aScriptPath="$PWD"
aProjName=${PWD##*/}

aDracoSrc=$aScriptPath
aNbJobs="$(getconf _NPROCESSORS_ONLN)"

# CMake should be up-to-date
#PATH=~/develop/local/bin:$PATH

# build stages to perform
toCMake=1
toClean=1
toMake=1
toInstall=1
toPack=1

aPlatformAndCompiler=lin64-gcc
aWorkDir=work/$aProjName-${aPlatformAndCompiler}-make
aDestDir=$aDracoSrc/work/$aProjName-$aPlatformAndCompiler
aLogFile=$aDracoSrc/build-${aPlatformAndCompiler}.log

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
  echo Configuring Draco for Linux...
  cmake -G "Unix Makefiles" \
  -D CMAKE_BUILD_TYPE:STRING="Release" \
  -D CMAKE_INSTALL_PREFIX:PATH="$aDestDir" \
  -D BUILD_DOCS:BOOL="OFF" \
  -D BUILD_LIBRARY_TYPE:STRING="Static" \
  "$aDracoSrc" 2>&1 | tee -a $aLogFile
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
  echo Installing into $aDracoSrc/work/$aPlatformAndCompiler...
  make install 2>&1 | tee -a $aLogFile
  gcc --version >> $aDestDir/config.log
  cp -f $aDracoSrc/LICENSE $aDestDir/LICENSE
fi
aTimeINSTALL=$SECONDS
logDuration $toInstall "Install" $aTimeBUILD $aTimeINSTALL

# create an archive
if [[ $toPack == 1 ]]; then
  anArchName=$aProjName-$aPlatformAndCompiler.tar.bz2
  echo Creating an archive $aDracoSrc/work/$anArchName...
  rm $aDracoSrc/work/$anArchName &>/dev/null
  pushd $aDestDir
  tar -jcf $aDracoSrc/work/$anArchName *
  popd
fi
aTimePACK=$SECONDS
logDuration $toPack "Packing archive" $aTimeINSTALL $aTimePACK

# finished
DURATION=$(($aTimePACK - $aTimeZERO))
echo Total time: $DURATION sec
logDuration 1 "Total" $aTimeZERO $aTimePACK

popd
