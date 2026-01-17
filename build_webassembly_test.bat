@echo off
echo 🌐 Building Nightmare Browser Edition (WebAssembly) - Test Build
echo ================================================================

REM Clean previous build
if exist build_webassembly_new rmdir /s /q build_webassembly_new
mkdir build_webassembly_new
cd build_webassembly_new

REM Set Emscripten environment
call ..\emsdk\emsdk_env.bat

REM Configure with explicit Emscripten detection and MinGW generator
cmake .. ^
    -G "MinGW Makefiles" ^
    -DCMAKE_TOOLCHAIN_FILE=../emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ^
    -DCMAKE_SYSTEM_NAME=Emscripten ^
    -DEMSCRIPTEN=1 ^
    -DWEBASSEMBLY_BUILD=1 ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s DISABLE_EXCEPTION_CATCHING=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_C_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_EXE_LINKER_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -s ALLOW_MEMORY_GROWTH=1 --preload-file assets@/" ^
    -DNOSOUND=ON

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
echo 📁 Output files are in build_webassembly_new/
pause