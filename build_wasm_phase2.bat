@echo off
echo 🚀 FASE 2: Engine Loop Verification (Enhanced Logging)
echo ========================================================

REM Clean build for FASE 2
if exist phase2 rmdir /s /q phase2
mkdir phase2
cd phase2

REM Set Emscripten environment
call ..\emsdk\emsdk_env.bat

REM FASE 2: Enhanced logging to verify engine loop execution
echo 📋 Configurando build con logging detallado...
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
    -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -DPHASE2_ENGINE_VERIFICATION=1" ^
    -DCMAKE_C_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_EXE_LINKER_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s EXPORTED_FUNCTIONS=['_main'] -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"

if %ERRORLEVEL% neq 0 (
    echo ❌ Configuración falló
    pause
    exit /b 1
)

echo 🔨 Compilando con verificación de engine loop...
emmake make devilutionx

if %ERRORLEVEL% neq 0 (
    echo ❌ Compilación falló
    pause
    exit /b 1
)

echo ✅ FASE 2 COMPLETADA!
echo 📁 Archivos generados:
dir *.js *.wasm 2>nul

echo.
echo 🎯 SIGUIENTE PASO: Testing engine loop verification
pause