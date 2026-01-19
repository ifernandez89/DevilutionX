@echo off
echo ========================================
echo 🔧 COMPILACION WEBASSEMBLY BASICA
echo ========================================

echo.
echo 🧹 Limpiando directorio anterior...
if exist build_webassembly_basic rmdir /s /q build_webassembly_basic
mkdir build_webassembly_basic
cd build_webassembly_basic

echo.
echo ⚙️ Configurando CMake con opciones BASICAS...
emcmake cmake .. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_TESTING=OFF ^
    -DNONET=ON ^
    -DDISABLE_ZERO_TIER=ON ^
    -DCMAKE_CXX_FLAGS="-O2" ^
    -DCMAKE_C_FLAGS="-O2"

if %ERRORLEVEL% neq 0 (
    echo ❌ ERROR: Configuracion de CMake fallida
    pause
    exit /b 1
)

echo.
echo 🔨 Compilando con configuracion basica...
emmake make -j 4

if %ERRORLEVEL% neq 0 (
    echo ❌ ERROR: Compilacion fallida
    pause
    exit /b 1
)

echo.
echo ✅ COMPILACION BASICA COMPLETADA
echo 📁 Archivos generados en: build_webassembly_basic/
echo.

dir *.wasm *.js *.data 2>nul
if %ERRORLEVEL% equ 0 (
    echo.
    echo 📊 Tamaños de archivos:
    for %%f in (*.wasm *.js *.data) do (
        echo   %%f: %%~zf bytes
    )
) else (
    echo ❌ No se encontraron archivos WebAssembly
)

echo.
echo 🎮 Para probar: Copiar archivos a docs/ y usar HTML original
pause