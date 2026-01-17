@echo off
echo 🌐 Building WebAssembly - Simple Fix
echo ====================================

REM Use a very short path to avoid Windows path issues
if exist w rmdir /s /q w
mkdir w
cd w

REM Set Emscripten environment
call ..\emsdk\emsdk_env.bat

REM Configure with very simple settings to avoid path issues
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release -DNOSOUND=ON -DDISABLE_ZERO_TIER=ON -DDISABLE_TCP=ON -DNONET=ON -DDEVILUTIONX_SCREENSHOT_FORMAT=DEVILUTIONX_SCREENSHOT_FORMAT_PCX

if %ERRORLEVEL% neq 0 (
    echo ❌ CMake configuration failed
    pause
    exit /b 1
)

REM Build with emmake to ensure proper Emscripten environment
emmake make devilutionx

if %ERRORLEVEL% neq 0 (
    echo ❌ Build failed
    pause
    exit /b 1
)

echo ✅ WebAssembly build completed successfully!
echo 📁 Output files:
dir *.js *.wasm 2>nul
pause