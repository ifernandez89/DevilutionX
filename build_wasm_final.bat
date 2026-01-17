@echo off
echo 🌐 Building WebAssembly - Final Fix
echo ===================================

REM Clean and create build directory
if exist wasm rmdir /s /q wasm
mkdir wasm
cd wasm

REM Set Emscripten environment
call ..\emsdk\emsdk_env.bat

REM Configure with minimal settings
cmake .. -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=../emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_SYSTEM_NAME=Emscripten -DEMSCRIPTEN=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" -DCMAKE_C_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" -DCMAKE_EXE_LINKER_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -s ALLOW_MEMORY_GROWTH=1" -DNOSOUND=ON -DDISABLE_ZERO_TIER=ON -DDISABLE_TCP=ON -DNONET=ON -DDEVILUTIONX_SCREENSHOT_FORMAT=DEVILUTIONX_SCREENSHOT_FORMAT_PCX

if %ERRORLEVEL% neq 0 (
    echo ❌ CMake configuration failed
    pause
    exit /b 1
)

REM Build
cmake --build . --target devilutionx

if %ERRORLEVEL% neq 0 (
    echo ❌ Build failed
    pause
    exit /b 1
)

echo ✅ WebAssembly build completed successfully!
echo 📁 Output files: devilutionx.js and devilutionx.wasm
dir *.js *.wasm
pause