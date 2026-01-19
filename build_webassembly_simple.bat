@echo off
echo ========================================
echo 🔧 COMPILACION WEBASSEMBLY SIMPLE
echo ========================================

echo.
echo 🔧 Activando Emscripten SDK...
cd emsdk
call emsdk_env.bat
cd ..

echo.
echo 🧹 Limpiando directorio anterior...
if exist build_webassembly_simple rmdir /s /q build_webassembly_simple
mkdir build_webassembly_simple
cd build_webassembly_simple

echo.
echo ⚙️ Configurando CMake con opciones SIMPLES...
emcmake cmake .. ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DBUILD_TESTING=OFF ^
    -DNONET=OFF ^
    -DDISABLE_ZERO_TIER=OFF ^
    -DCMAKE_CXX_FLAGS="-O1 -g" ^
    -DCMAKE_C_FLAGS="-O1 -g"

if %ERRORLEVEL% neq 0 (
    echo ❌ ERROR: Configuracion de CMake fallida
    pause
    exit /b 1
)

echo.
echo 🔨 Compilando con configuracion simple...
emmake make -j 2

if %ERRORLEVEL% neq 0 (
    echo ❌ ERROR: Compilacion fallida
    pause
    exit /b 1
)

echo.
echo ✅ COMPILACION SIMPLE COMPLETADA
echo 📁 Archivos generados en: build_webassembly_simple/
echo.

dir *.wasm *.js *.data 2>nul
if %ERRORLEVEL% equ 0 (
    echo.
    echo 📊 Tamaños de archivos:
    for %%f in (*.wasm *.js *.data) do (
        echo   %%f: %%~zf bytes
    )
    
    echo.
    echo 📋 Copiando archivos a docs para prueba...
    copy devilutionx.wasm ..\docs\devilutionx_simple.wasm
    copy devilutionx.js ..\docs\devilutionx_simple.js
    copy devilutionx.data ..\docs\devilutionx_simple.data
    copy index.html ..\docs\diablo_simple_compiled.html
    
    echo ✅ Archivos copiados a docs/
    echo 🌐 Prueba en: http://localhost:8000/diablo_simple_compiled.html
    
) else (
    echo ❌ No se encontraron archivos WebAssembly
)

echo.
pause