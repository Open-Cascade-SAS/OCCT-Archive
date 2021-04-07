Instruction for building OCCT for ASRV
======================================

Build using regular native compiler
-----------------------------------

1. Make sure the directory 3rdparty is next to occt. It must contain the following products:
    freetype
    tbb
    tcltk
2. Change current directory to adm/scripts.
3. On Windows run the script cmake_gen.bat.
   On Linux run 'cmake_gen.sh' for release build and 'cmake_gen.sh -d' for debug build.
4. Go to build directory and build and install OCCT.
   On Windows run:
    build.bat
    build.bat vc14 64 d
    install.bat
    install.bat vc14 64 d
   On Linux run:
    make -j4 install

Build using Emscripten to generate libs for Web assembly
--------------------------------------------------------

1. Make sure you have installed Emscripten. For that you can make sure occ-web3d directory 
   is next to occt. It must contain fips\fips-sdks\emsdk.
2. Make sure the directory 3rdparty is next to occt. It must contain the following products:
    freetype wasm build
3. Change current directory to adm/scripts.
4. Run the script wasm_build.bat (wasm_build.sh on Linux).
5. Copy installed files from work/wasm to the 3rdparty under occt750-wasm.
