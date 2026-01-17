@echo off
REM 🌐 Full Nightmare Browser Edition Build Script

echo 🌐 NIGHTMARE BROWSER EDITION - FULL BUILD
echo ==========================================

REM Set Emscripten environment
call emsdk\emsdk_env.bat

REM Check if Emscripten is available
where emcc >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ Error: Emscripten not found! Run emsdk_env.bat first
    exit /b 1
)

echo ✅ Emscripten found
emcc --version | findstr /C:"emcc"

REM Generate asset manifest
echo 🔧 Generating asset manifest...
python tools\generate_asset_manifest.py

REM Create build directory
set BUILD_DIR=build_nightmare_full
echo 📁 Creating build directory: %BUILD_DIR%
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Configure with Emscripten
echo ⚙️ Configuring Full WebAssembly build...
cd "%BUILD_DIR%"

emcmake cmake .. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DWEBASSEMBLY_BUILD=ON ^
    -DNO_SAVE_SYSTEM=ON ^
    -DPERMADEATH_MODE=ON ^
    -DBROWSER_EDITION=ON ^
    -DNONET=ON ^
    -DBUILD_TESTING=OFF ^
    -DUSE_SDL1=OFF ^
    -DUSE_SDL3=OFF

if %errorlevel% neq 0 (
    echo ❌ Configuration failed
    cd ..
    exit /b 1
)

REM Build
echo 🔨 Building Full WebAssembly...
emmake make -j4

if %errorlevel% neq 0 (
    echo ❌ Build failed
    cd ..
    exit /b 1
)

echo.
echo 🎯 Full build complete!
echo 📁 Output files in %BUILD_DIR%:
dir *.js *.wasm *.html 2>nul

echo.
echo 🌐 To test:
echo   python -m http.server 8000
echo   Open: http://localhost:8000/%BUILD_DIR%/
echo.
echo 🎮 Full Nightmare Browser Edition ready!

cd ..