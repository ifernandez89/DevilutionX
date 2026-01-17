@echo off
REM 🌐 WebAssembly Build Script for Nightmare Browser Edition (Windows)

echo 🌐 Nightmare Browser Edition - WebAssembly Build
echo ==================================================

REM Check if Emscripten is available
where emcc >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ Error: Emscripten not found!
    echo Please install Emscripten SDK:
    echo   git clone https://github.com/emscripten-core/emsdk.git
    echo   cd emsdk
    echo   emsdk install latest
    echo   emsdk activate latest
    echo   emsdk_env.bat
    exit /b 1
)

echo ✅ Emscripten found
emcc --version | findstr /C:"emcc"

REM Generate asset manifest
echo 🔧 Generating asset manifest...
if exist "tools\generate_asset_manifest.py" (
    python tools\generate_asset_manifest.py
    echo ✅ Asset manifest generated
) else (
    echo ⚠️ Warning: Asset manifest generator not found
)

REM Create build directory
set BUILD_DIR=build_webassembly
echo 📁 Creating build directory: %BUILD_DIR%
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Configure with Emscripten
echo ⚙️ Configuring WebAssembly build...
cd "%BUILD_DIR%"

emcmake cmake .. ^
    -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DWEBASSEMBLY_BUILD=ON ^
    -DNO_SAVE_SYSTEM=ON ^
    -DPERMADEATH_MODE=ON ^
    -DBROWSER_EDITION=ON

if %errorlevel% neq 0 (
    echo ❌ Configuration failed
    exit /b 1
)

REM Build
echo 🔨 Building WebAssembly...
emmake mingw32-make -j4

if %errorlevel% neq 0 (
    echo ❌ Build failed
    exit /b 1
)

echo.
echo 🎯 Build complete!
echo 📁 Output files:
echo   - devilutionx.js
echo   - devilutionx.wasm
echo   - devilutionx.data (if assets embedded)
echo.
echo 🌐 To test locally:
echo   python -m http.server 8000
echo   Open: http://localhost:8000
echo.
echo 🎮 Nightmare Browser Edition ready!

cd ..