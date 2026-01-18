@echo off
echo 🔊 FASE 5: Audio System Verification (Sound Effects & Audio Playback)
echo ================================================================

REM Clean build for FASE 5
if exist phase5 rmdir /s /q phase5
mkdir phase5
cd phase5

REM Set Emscripten environment
call ..\emsdk\emsdk_env.bat

REM FASE 5: Audio system verification with enhanced audio output
echo 📋 Configurando build con verificación de audio system...
emcmake cmake .. ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_CXX_STANDARD=20 ^
    -DNOSOUND=OFF ^
    -DDISABLE_ZERO_TIER=ON ^
    -DDISABLE_TCP=ON ^
    -DNONET=ON ^
    -DPACKET_ENCRYPTION=OFF ^
    -DASAN=OFF ^
    -DENABLE_ASAN=OFF ^
    -DUBSAN=OFF ^
    -DTSAN=OFF ^
    -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -DPHASE5_AUDIO_VERIFICATION=1" ^
    -DCMAKE_C_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0" ^
    -DCMAKE_EXE_LINKER_FLAGS="-s USE_SDL=2 -s USE_PTHREADS=0 -s SHARED_MEMORY=0 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s EXPORTED_FUNCTIONS=['_main'] -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"

if %ERRORLEVEL% neq 0 (
    echo ❌ Configuración falló
    pause
    exit /b 1
)

echo 🔨 Compilando con verificación de audio system...
emmake make devilutionx

if %ERRORLEVEL% neq 0 (
    echo ❌ Compilación falló
    pause
    exit /b 1
)

echo ✅ FASE 5 COMPLETADA!
echo 📁 Archivos generados:
dir *.js *.wasm 2>nul

echo.
echo 🎯 SIGUIENTE PASO: Testing audio system verification
pause