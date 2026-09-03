@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   DEVILUTIONX - COMPILADOR WEBASSEMBLY (WASM)
echo ===================================================

cd /d "%~dp0\.."

if not exist "deps\emsdk\emsdk_env.bat" (
    echo [ERROR] No se encontro Emscripten en deps\emsdk.
    echo Por favor ejecuta primero: scripts\setup_emsdk.bat
    pause
    exit /b 1
)

echo [1/4] Cargando variables de entorno de Emscripten...
call deps\emsdk\emsdk_env.bat

echo [2/4] Pre-compilando librerias del sistema (ports)...
call embuilder build zlib bzip2 libpng sdl2 sdl2_image sdl2_image_png

echo [3/4] Configurando CMake para WebAssembly con Ninja...
call emcmake cmake -S . -B build-web -G Ninja ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DBUILD_TESTING=OFF ^
  -DASAN=OFF ^
  -DUBSAN=OFF ^
  -DTSAN=OFF ^
  -DDISABLE_ZERO_TIER=ON ^
  -DDISABLE_TCP=ON ^
  -DDEVILUTIONX_SYSTEM_SDL_AUDIOLIB=OFF ^
  -DDEVILUTIONX_SYSTEM_LIBSODIUM=OFF ^
  -DDEVILUTIONX_SYSTEM_LIBFMT=OFF ^
  -DDEVILUTIONX_SYSTEM_BZIP2=OFF

echo [4/4] Compilando target devilutionx (WASM)...
cmake --build build-web --target devilutionx -j 4

if %ERRORLEVEL% equ 0 (
    echo.
    echo ===================================================
    echo   COMPILACION WASM EXITOSA!
    echo   Sincronizando binarios con Packaging\emscripten...
    echo ===================================================
    copy /y "build-web\devilutionx.wasm" "Packaging\emscripten\devilutionx.wasm"
    copy /y "build-web\devilutionx.js"   "Packaging\emscripten\devilutionx.js"
    if exist "build-web\devilutionx.data" (
        copy /y "build-web\devilutionx.data" "Packaging\emscripten\devilutionx.data"
    )
    echo.
    echo [LISTO] Los nuevos binarios ya estan activos en tu servidor local!
) else (
    echo.
    echo [ERROR] La compilacion de WebAssembly fallo. Revisa los mensajes arriba.
)

if "%~1" neq "--non-interactive" pause
