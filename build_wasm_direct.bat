@echo off
echo 🌐 Building WebAssembly - Direct Emscripten
echo ============================================

REM Clean build directory
if exist em rmdir /s /q em
mkdir em
cd em

REM Set Emscripten environment properly
call ..\emsdk\emsdk_env.bat

REM Use emcmake with explicit toolchain
emcmake cmake .. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_CXX_STANDARD=20 ^
    -DCMAKE_CXX_STANDARD_REQUIRED=ON ^
    -DNOSOUND=ON ^
    -DDISABLE_ZERO_TIER=ON ^
    -DDISABLE_TCP=ON ^
    -DNONET=ON ^
    -DDEVILUTIONX_SCREENSHOT_FORMAT=DEVILUTIONX_SCREENSHOT_FORMAT_PCX ^
    -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -stdlib=libc++" ^
    -DCMAKE_C_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_EXE_LINKER_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -s ALLOW_MEMORY_GROWTH=1"

if %ERRORLEVEL% neq 0 (
    echo ❌ CMake configuration failed
    pause
    exit /b 1
)

REM Build with emmake
emmake make devilutionx -j1

if %ERRORLEVEL% neq 0 (
    echo ❌ Build failed
    pause
    exit /b 1
)

echo ✅ WebAssembly build completed successfully!
echo 📁 Output files:
dir *.js *.wasm 2>nul
pause