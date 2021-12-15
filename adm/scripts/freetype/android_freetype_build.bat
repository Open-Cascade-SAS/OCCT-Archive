@echo OFF

rem Auxiliary script for semi-automated building of FreeType for Android platform.
rem Script should be placed into root of FreeType repository, edited with paths
rem to CMake, 3rd-parties, Android NDK and MinGW64 make tool.

rem CMake toolchain definition should be cloned from the following git repository:
rem https://github.com/taka-no-me/android-cmake

rem CMake can be downloaded from official site:
rem https://cmake.org/download/

rem Android NDK can be downloaded from official site:
rem https://developer.android.com/ndk/downloads

set "aFreeTypeSrc=%~dp0"
set aNbJobs=%NUMBER_OF_PROCESSORS%

call c:\TDM-GCC-64\mingwvars.bat
set "PATH=c:\CMake\bin;%PATH%"
set "anNdkPath=c:/android-ndk-r12"
set "aToolchain=c:/android-cmake-master/android.toolchain.cmake"

set "toPack=1"

set "FREETYPE_MAJOR=0"
set "FREETYPE_MINOR=0"
set "FREETYPE_PATCH=0"
for /f "tokens=3" %%i in ('findstr /b /c:"#define FREETYPE_MAJOR" "%aFreeTypeSrc%\include\freetype\freetype.h"') do ( set "FREETYPE_MAJOR=%%i" )
for /f "tokens=3" %%i in ('findstr /b /c:"#define FREETYPE_MINOR" "%aFreeTypeSrc%\include\freetype\freetype.h"') do ( set "FREETYPE_MINOR=%%i" )
for /f "tokens=3" %%i in ('findstr /b /c:"#define FREETYPE_PATCH" "%aFreeTypeSrc%\include\freetype\freetype.h"') do ( set "FREETYPE_PATCH=%%i" )
set "aFreeTypeVersion=%FREETYPE_MAJOR%.%FREETYPE_MINOR%.%FREETYPE_PATCH%"
echo "aFreeTypeVersion=%aFreeTypeVersion%"

call :cmakeGenerate "15" "armeabi-v7a"
call :cmakeGenerate "15" "x86"
call :cmakeGenerate "21" "arm64-v8a"
call :cmakeGenerate "21" "x86_64"

set "THE_7Z_PARAMS=-t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on"
if ["%THE_7Z_PATH%"] == [""] set "THE_7Z_PATH=%ProgramW6432%\7-Zip\7z.exe"
if not exist "%THE_7Z_PATH%" set "THE_7Z_PATH=%ProgramW6432%\7-Zip\7z.exe"
if not exist "%THE_7Z_PATH%" set "THE_7Z_PATH=%ProgramFiles%\7-Zip\7z.exe"

set "anArchName=freetype-%aFreeTypeVersion%-android"
set "aTarget=%~dp0work\%anArchName%"

if "%toPack%"=="1" (
  echo Creating archive %anArchName%.7z
  rmdir /S /Q "%aTarget%"
  if not exist "%aTarget%\libs" ( mkdir "%aTarget%\libs" )
  if exist "%~dp0work/%anArchName%.7z" del "%~dp0work/%anArchName%.7z"
  xcopy /S /Y /I "%~dp0work\android-armeabi-v7a-gcc\include\freetype2"  "%aTarget%\include"
  xcopy /Y "%~dp0work\android-armeabi-v7a-gcc\lib\*.so" "%aTarget%\libs\armeabi-v7a\"
  xcopy /Y "%~dp0work\android-arm64-v8a-gcc\lib\*.so"   "%aTarget%\libs\arm64-v8a\"
  xcopy /Y "%~dp0work\android-x86-gcc\lib\*.so"         "%aTarget%\libs\x86\"
  xcopy /Y "%~dp0work\android-x86_64-gcc\lib\*.so"      "%aTarget%\libs\x86_64\"
  xcopy /Y "%aFreeTypeSrc%\README"                      "%aTarget%\"
  xcopy /Y "%aFreeTypeSrc%\docs\FTL.TXT"                "%aTarget%\"
  xcopy /Y "%aFreeTypeSrc%\docs\GPLv2.TXT"              "%aTarget%\"
  xcopy /Y "%aFreeTypeSrc%\docs\LICENSE.TXT"            "%aTarget%\"

  "%THE_7Z_PATH%" a -r %THE_7Z_PARAMS% "%~dp0work/%anArchName%.7z" "%aTarget%"
)

if not ["%1"] == ["-nopause"] (
  pause
)

goto :eof

:cmakeGenerate
set "anApi=%1"
set "anAbi=%2"
set "aPlatformAndCompiler=android-%anAbi%-gcc"
set "aWorkDir=work\%aPlatformAndCompiler%-make"
set "aLogFile=%~dp0build-%anAbi%.log"
if not exist "%aWorkDir%" ( mkdir "%aWorkDir%" )
if     exist "%aLogFile%" ( del   "%aLogFile%" )

pushd "%aWorkDir%"

set STARTTIME=%TIME%
echo Configuring OCCT for Android %anAbi% (API level %anApi%)...
cmake -G "MinGW Makefiles" ^
 -D CMAKE_TOOLCHAIN_FILE:FILEPATH="%aToolchain%" ^
 -D ANDROID_NDK:FILEPATH="%anNdkPath%" ^
 -D ANDROID_ABI:STRING="%anAbi%" ^
 -D ANDROID_NATIVE_API_LEVEL:STRING="%anApi%" ^
 -D ANDROID_STL:STRING="gnustl_shared" ^
 -D CMAKE_BUILD_TYPE:STRING="Release" ^
 -D BUILD_SHARED_LIBS:BOOL="TRUE" ^
 -D CMAKE_INSTALL_PREFIX:PATH="%~dp0work/%aPlatformAndCompiler%" ^
 "%aFreeTypeSrc%"

if errorlevel 1 (
  popd
  pause
  exit /B
  goto :eof
)

mingw32-make clean

echo Building FreeType...
mingw32-make -j %aNbJobs% 2>> %aLogFile%
type %aLogFile%
if errorlevel 1 (
  popd
  pause
  exit /B
  goto :eof
)
echo Installing FreeType into %~dp0work/%aPlatformAndCompiler%...
mingw32-make install 2>> %aLogFile%

set ENDTIME=%TIME%
rem handle time before 10AM (win10 - remove empty space at the beginning)
if "%STARTTIME:~0,1%"==" " set "STARTTIME=%STARTTIME:~1%"
if   "%ENDTIME:~0,1%"==" " set   "ENDTIME=%ENDTIME:~1%"
rem handle time before 10AM (win7 - add 0 at the beginning)
if "%STARTTIME:~1,1%"==":" set "STARTTIME=0%STARTTIME%"
if   "%ENDTIME:~1,1%"==":" set   "ENDTIME=0%ENDTIME%"
rem convert hours:minutes:seconds:ms into duration
set /A STARTTIME=(1%STARTTIME:~0,2%-100)*360000 + (1%STARTTIME:~3,2%-100)*6000 + (1%STARTTIME:~6,2%-100)*100 + (1%STARTTIME:~9,2%-100)
set /A   ENDTIME=  (1%ENDTIME:~0,2%-100)*360000 +   (1%ENDTIME:~3,2%-100)*6000 +   (1%ENDTIME:~6,2%-100)*100 +   (1%ENDTIME:~9,2%-100)
set /A DURATION=%ENDTIME%-%STARTTIME%
if %ENDTIME% LSS %STARTTIME% set set /A DURATION=%STARTTIME%-%ENDTIME%
set /A DURATIONH=%DURATION% / 360000
set /A DURATIONM=(%DURATION% - %DURATIONH%*360000) / 6000
set /A DURATIONS=(%DURATION% - %DURATIONH%*360000 - %DURATIONM%*6000) / 100
if %DURATIONH% LSS 10 set DURATIONH=0%DURATIONH%
if %DURATIONM% LSS 10 set DURATIONM=0%DURATIONM%
if %DURATIONS% LSS 10 set DURATIONS=0%DURATIONS%
echo Building time: %DURATIONH%:%DURATIONM%:%DURATIONS% for %anAbi%
echo Building time: %DURATIONH%:%DURATIONM%:%DURATIONS% >> %aLogFile%

popd
goto :eof
