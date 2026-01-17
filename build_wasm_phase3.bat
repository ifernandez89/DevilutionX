@echo off
echo 🚀 FASE 3: Input System Verification (Mouse & Keyboard Testing)
echo ================================================================

REM Clean build for FASE 3
if exist phase3 rmdir /s /q phase3
mkdir phase3
cd phase3

REM Set Emscripten environment
call ..\emsdk\emsdk_env.bat

REM FASE 3: Input system verification with enhanced event handling
echo 📋 Configurando build con verificación de input...
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
    -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -DPHASE3_INPUT_VERIFICATION=1" ^
    -DCMAKE_C_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_EXE_LINKER_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s EXPORTED_FUNCTIONS=['_main'] -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"

if %ERRORLEVEL% neq 0 (
    echo ❌ Configuración falló
    pause
    exit /b 1
)

echo 🔨 Compilando con verificación de input system...
emmake make devilutionx

if %ERRORLEVEL% neq 0 (
    echo ❌ Compilación falló
    pause
    exit /b 1
)

echo ✅ FASE 3 COMPLETADA!
echo 📁 Archivos generados:
dir *.js *.wasm 2>nul

echo.
echo 🎯 SIGUIENTE PASO: Testing input system verification
pause