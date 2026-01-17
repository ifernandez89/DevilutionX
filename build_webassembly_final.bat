@echo off
echo 🌐 Building Nightmare Browser Edition (WebAssembly) - Final Fix
echo ================================================================

REM Clean previous build
if exist build_webassembly_clean rmdir /s /q build_webassembly_clean
mkdir build_webassembly_clean
cd build_webassembly_clean

REM Set Emscripten environment
call ..\emsdk\emsdk_env.bat

REM Configure with consistent threading disabled and simplified features
cmake .. ^
    -G "MinGW Makefiles" ^
    -DCMAKE_TOOLCHAIN_FILE=../emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ^
    -DCMAKE_SYSTEM_NAME=Emscripten ^
    -DEMSCRIPTEN=1 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s DISABLE_EXCEPTION_CATCHING=1 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_C_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_EXE_LINKER_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=134217728" ^
    -DNOSOUND=ON ^
    -DDISABLE_ZERO_TIER=ON ^
    -DDISABLE_TCP=ON ^
    -DNONET=ON ^
    -DDEVILUTIONX_SCREENSHOT_FORMAT=DEVILUTIONX_SCREENSHOT_FORMAT_PCX ^
    -DDISCORD_INTEGRATION=OFF ^
    -DSCREEN_READER_INTEGRATION=OFF

if %ERRORLEVEL% neq 0 (
    echo ❌ CMake configuration failed
    pause
    exit /b 1
)

REM Build just the main target to test compilation
cmake --build . --target devilutionx --config Release

if %ERRORLEVEL% neq 0 (
    echo ❌ Build failed
    pause
    exit /b 1
)

echo ✅ WebAssembly build completed successfully!
echo 📁 Output files are in build_webassembly_clean/
pause