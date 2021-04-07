rem Environment configuration template for wasm_build.bat (to be renamed as wasm_custom.bat)
set "EMSDK_ROOT=%aCasSrc%\..\occ-web3d\fips\fips-sdks\emsdk"
set "aFreeType=%aCasSrc%\..\3rdparty\freetype-2.7.1-wasm"
rem set "aRapidJson=%aCasSrc%\..\3rdparty\rapidjson-1.1.0"
rem set "aDraco=%aCasSrc%\..\3rdparty\draco-1.4.1-wasm32"
rem set "aCmakeBin=%ProgramW6432%\CMake\bin"

rem Uncomment to customize building steps
rem set "aBuildRoot=work"
rem set "toCMake=1"
rem set "toClean=0"
rem set "toMake=1"
rem set "toInstall=1"
set "toPack=0"
set "toDebug=0"
set "toBuildSample=0"
rem Source map base (should point to server where C++ sources will be copied)
rem enables -g4 debug building option for WebGL sample and allows navigating C++ source code within JavaScript debugger.
rem set "sourceMapBase=http://localhost:9090/"

set "BUILD_ModelingData=OFF"
set "BUILD_ModelingAlgorithms=OFF"
rem set "BUILD_Visualization=ON"
set "BUILD_ApplicationFramework=OFF"
set "BUILD_DataExchange=OFF"

set "USE_RAPIDJSON=OFF"
set "USE_DRACO=OFF"
set "USE_PTHREADS=OFF"
