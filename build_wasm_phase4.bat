@echo off
echo 🎨 FASE 4: Render System Verification (Canvas Display & Visual Output)
echo ================================================================

REM Clean build for FASE 4
if exist phase4 rmdir /s /q phase4
mkdir phase4
cd phase4

REM Set Emscripten environment
call ..\emsdk\emsdk_env.bat

REM FASE 4: Render system verification with enhanced visual output
echo 📋 Configurando build con verificación de render system...
emcmake cmake .. ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_CXX_STANDARD=20 ^
    -DNOSOUND=ON ^
    -DDISABLE_ZERO_TIER=ON ^
    -DDISABLE_TCP=ON ^
    -DNONET=ON ^
    -DPACKET_ENCRYPTION=OFF ^
    -DASAN=OFF ^
    -DENABLE_ASAN=OFF ^
    -DUBSAN=OFF ^
    -DTSAN=OFF ^
    -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -DPHASE4_RENDER_VERIFICATION=1" ^
    -DCMAKE_C_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_EXE_LINKER_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s EXPORTED_FUNCTIONS=['_main'] -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"

if %ERRORLEVEL% neq 0 (
    echo ❌ Configuración falló
    pause
    exit /b 1
)

echo 🔨 Compilando con verificación de render system...
emmake make devilutionx

if %ERRORLEVEL% neq 0 (
    echo ❌ Compilación falló
    pause
    exit /b 1
)

echo ✅ FASE 4 COMPLETADA!
echo 📁 Archivos generados:
dir *.js *.wasm 2>nul

echo.
echo 🎯 SIGUIENTE PASO: Testing render system verification
pause