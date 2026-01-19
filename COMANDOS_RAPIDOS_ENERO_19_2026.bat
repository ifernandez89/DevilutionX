@echo off
echo ========================================
echo DIABLO 1 WEBASSEMBLY COMPILATION SCRIPT
echo Date: January 19, 2026
echo ========================================

echo.
echo [1/5] Activating Emscripten environment...
cd emsdk
call emsdk_env.bat
cd ..

echo.
echo [2/5] Creating clean build directory...
if exist build_webassembly_real rmdir /s /q build_webassembly_real
mkdir build_webassembly_real
cd build_webassembly_real

echo.
echo [3/5] Configuring CMake with Emscripten...
cmake -G "MinGW Makefiles" ^
  -DCMAKE_TOOLCHAIN_FILE="../emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DBUILD_TESTING=OFF ^
  -DNONET=ON ^
  -DCMAKE_STRIP="" ^
  -DDEVILUTIONX_SYSTEM_LIBFMT=OFF ^
  -DDEVILUTIONX_SYSTEM_LIBSODIUM=OFF ^
  -DDEVILUTIONX_SYSTEM_SIMPLEINI=OFF ^
  -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=67108864 -s EXPORTED_FUNCTIONS=['_main'] -s EXPORTED_RUNTIME_METHODS=['callMain','FS'] -s MODULARIZE=1 -s EXPORT_NAME=DevilutionX" ^
  ..

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo [4/5] Compiling DevilutionX for WebAssembly...
mingw32-make -j4

if %ERRORLEVEL% neq 0 (
    echo ERROR: Compilation failed!
    pause
    exit /b 1
)

echo.
echo [5/5] Copying files to docs directory...
if exist devilutionx.wasm (
    copy devilutionx.wasm ..\docs\devilutionx_real.wasm
    echo ✅ WASM file copied
) else (
    echo ❌ WASM file not found
)

if exist devilutionx.js (
    copy devilutionx.js ..\docs\devilutionx_real.js
    echo ✅ JS file copied
) else (
    echo ❌ JS file not found
)

if exist devilutionx.data (
    copy devilutionx.data ..\docs\devilutionx_real.data
    echo ✅ DATA file copied
) else (
    echo ❌ DATA file not found
)

echo.
echo ========================================
echo COMPILATION COMPLETE!
echo ========================================
echo.
echo Next steps:
echo 1. Create diablo_final_working.html
echo 2. Test in browser
echo 3. Commit and push to GitHub
echo.
pause