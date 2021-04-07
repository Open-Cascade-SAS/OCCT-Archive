rem Environment configuration template for occ_build_wasm.bat (to be renamed as wasm_custom_env.bat)
set "aFreeType=%aSrcRoot%\..\3rdparty\freetype-2.7.1-wasm"
set "EMSDK_ROOT=%aSrcRoot%\..\occ-web3d\fips\fips-sdks\emsdk"

rem Uncomment to customize building steps
rem set "aBuildRoot=work"
rem set "toCMake=1"
rem set "toClean=0"
rem set "toMake=1"
rem set "toInstall=1"

set "BUILD_ModelingData=OFF"
set "BUILD_ModelingAlgorithms=OFF"
rem set "BUILD_Visualization=ON"
set "BUILD_ApplicationFramework=OFF"
set "BUILD_DataExchange=OFF"
