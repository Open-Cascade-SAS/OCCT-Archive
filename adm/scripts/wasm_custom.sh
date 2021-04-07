# environment configuration template for occ_build_wasm.sh (to be renamed as wasm_custom_env.sh)
export aFreeType="$aSrcRoot/../3rdparty/freetype-2.7.1-wasm"
export EMSDK_ROOT="$aSrcRoot/../occ-web3d/fips/fips-sdks/emsdk"

# Uncomment to customize building steps
#export aBuildRoot=work
#export toCMake=1
#export toClean=0
#export toMake=1
#export toInstall=1

export BUILD_ModelingData=OFF
export BUILD_ModelingAlgorithms=OFF
#export BUILD_Visualization=ON
export BUILD_ApplicationFramework=OFF
export BUILD_DataExchange=OFF

export aNbJobs=10
