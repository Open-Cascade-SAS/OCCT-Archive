@echo OFF

rem Auxiliary script for semi-automated building of Draco library for Android platform.
rem https://github.com/google/draco

set "aDracoSrc=%~dp0"
set "aBuildRoot=%aDracoSrc%\work"

set aNbJobs=%NUMBER_OF_PROCESSORS%

rem Paths to 3rd-party tools and libraries
call c:\TDM-GCC-64\mingwvars.bat
set "aCmakeBin=c:\CMake\bin"
set "anNdkPath=c:/android-ndk-r12"

set "PATH=%aCmakeBin%;%PATH%"
set "aCompiler=gcc"
set "aCppLib=gnustl_shared"
if not exist "%anNdkPath%/sources/cxx-stl/gnu-libstdc++" (
  if exist "%anNdkPath%/sources/cxx-stl/llvm-libc++" (
    set "aCompiler=clang"
    set "aCppLib=c++_shared"
  )
)

set "aDestDir=%aBuildRoot%\draco-android-%aCompiler%"
rmdir /S /Q "%aDestDir%"
if not exist "%aDestDir%" ( mkdir "%aDestDir%" )
if not exist "%aDestDir%\include" ( mkdir "%aDestDir%\include" )
if not exist "%aDestDir%\libs"    ( mkdir "%aDestDir%\libs" )
xcopy /Y "%aDracoSrc%\LICENSE"   "%aDestDir%\"
xcopy /Y "%aDracoSrc%\README.md" "%aDestDir%\"

call :cmakeGenerate "15" "armeabi-v7a"
call :cmakeGenerate "15" "x86"
call :cmakeGenerate "21" "arm64-v8a"
call :cmakeGenerate "21" "x86_64"

set "anArchName=draco-android"
echo Creating archive %anArchName%.7z
if exist "%aBuildRoot%/%anArchName%.7z" del "%aBuildRoot%/%anArchName%.7z"
set "THE_7Z_PARAMS=-t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on"
set "THE_7Z_PATH=%ProgramW6432%\7-Zip\7z.exe"
"%THE_7Z_PATH%" a -r %THE_7Z_PARAMS% "%aBuildRoot%/%anArchName%.7z" "%aDestDir%"

pause

goto :eof

:cmakeGenerate
set "anApi=%~1"
set "anAbi=%~2"
set "aPlatformAndCompiler=android-%anAbi%-%aCompiler%"
set "aWorkDir=%aBuildRoot%\draco-%aPlatformAndCompiler%-make"
set "aLogFile=%aBuildRoot%\build-%aPlatformAndCompiler%.log"
if not exist "%aWorkDir%" ( mkdir "%aWorkDir%" )
if     exist "%aLogFile%" ( del   "%aLogFile%" )

pushd "%aWorkDir%"

echo Configuring Draco for Android %anAbi%, API level %anApi%...
cmake -G "MinGW Makefiles" ^
 -D CMAKE_SYSTEM_NAME:STRING="Android" ^
 -D CMAKE_ANDROID_NDK="%anNdkPath%" ^
 -D CMAKE_BUILD_TYPE:STRING="Release" ^
 -D CMAKE_ANDROID_ARCH_ABI:STRING="%anAbi%" ^
 -D CMAKE_SYSTEM_VERSION:STRING="%anApi%" ^
 -D CMAKE_ANDROID_STL_TYPE="%aCppLib%" ^
 -D BUILD_LIBRARY_TYPE:STRING="Static" ^
 -D CMAKE_INSTALL_PREFIX:PATH="%aBuildRoot%/draco-%aPlatformAndCompiler%" ^
 -D LIBRARY_OUTPUT_PATH:PATH="%aBuildRoot%/draco-%aPlatformAndCompiler%/libs/%anAbi%" ^
 "%aDracoSrc%"

if errorlevel 1 (
  popd
  pause
  exit /B
  goto :eof
)

mingw32-make clean

echo Building...
mingw32-make -j %aNbJobs% 2>> "%aLogFile%"
if errorlevel 1 (
  type %aLogFile%
  popd
  pause
  exit /B
  goto :eof
)
type %aLogFile%

echo Installing Draco into %aBuildRoot%/draco-%aPlatformAndCompiler%...
mingw32-make install 2>> %aLogFile%

xcopy /S /Y "%aBuildRoot%\draco-%aPlatformAndCompiler%\include\*" "%aDestDir%\include\"
xcopy /S /Y "%aBuildRoot%\draco-%aPlatformAndCompiler%\libs\*"    "%aDestDir%\libs\"

popd
goto :eof
