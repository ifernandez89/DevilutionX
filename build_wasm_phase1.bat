@echo off
echo 🚀 FASE 1: Regenerar JS wrapper limpio (sin strip, sin optimizaciones)
echo ================================================================

REM Clean build
if exist phase1 rmdir /s /q phase1
mkdir phase1
cd phase1

REM Set Emscripten environment
call ..\emsdk\emsdk_env.bat

REM FASE 1: Configuración MÍNIMA - solo que compile y genere JS
echo 📋 Configurando build mínimo...
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
    -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_C_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_EXE_LINKER_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1"

if %ERRORLEVEL% neq 0 (
    echo ❌ Configuración falló
    pause
    exit /b 1
)

echo 🔨 Compilando (sin optimizaciones, sin strip)...
emmake make devilutionx

if %ERRORLEVEL% neq 0 (
    echo ❌ Compilación falló
    pause
    exit /b 1
)

echo ✅ FASE 1 COMPLETADA!
echo 📁 Archivos generados:
dir *.js *.wasm 2>nul

echo.
echo 🎯 SIGUIENTE PASO: Crear index.html limpio
pause