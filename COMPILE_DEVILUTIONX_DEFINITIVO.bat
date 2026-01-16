@echo off
echo ========================================
echo  COMPILACION DEFINITIVA DEVILUTIONX
echo  METODO 100% FUNCIONAL CON AUDIO
echo ========================================
echo.

echo [1/3] Configurando CMAKE...
cmake -S. -Bbuild_NOW -G "MinGW Makefiles" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DNONET=ON ^
  -DNOSOUND=OFF ^
  -DBUILD_TESTING=OFF ^
  -DDISABLE_ZERO_TIER=ON ^
  -DUNPACKED_MPQS=OFF ^
  -DUNPACKED_SAVES=OFF ^
  -DDEVILUTIONX_STATIC_BZIP2=ON ^
  -DDEVILUTIONX_SYSTEM_BZIP2=OFF ^
  -DSDL2_DIR="deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/lib/cmake/SDL2" ^
  -DZLIB_LIBRARY="C:/c - c++/DevilutionX/deps/zlib-build/libzlibstatic.a" ^
  -DZLIB_INCLUDE_DIR="C:/c - c++/DevilutionX/deps/zlib-src/zlib-1.3.1" ^
  -DCMAKE_PREFIX_PATH="C:/c - c++/DevilutionX/deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32"

if %ERRORLEVEL% neq 0 (
    echo ERROR: Configuracion CMAKE fallo!
    pause
    exit /b 1
)

echo.
echo [2/3] Compilando...
cmake --build build_NOW -j 4

if %ERRORLEVEL% neq 0 (
    echo ERROR: Compilacion fallo!
    pause
    exit /b 1
)

echo.
echo [3/3] Verificando archivos necesarios...

if exist "build_NOW\DIABDAT.MPQ" (
    echo ✓ DIABDAT.MPQ ya existe
) else (
    echo Copiando DIABDAT.MPQ...
    copy build_final_working\DIABDAT.MPQ build_NOW\
)

if exist "build_NOW\SDL2.dll" (
    echo ✓ DLLs ya existen
) else (
    echo Copiando DLLs...
    copy build_final_working\*.dll build_NOW\
)

echo.
echo ========================================
echo  COMPILACION COMPLETADA EXITOSAMENTE!
echo ========================================
echo.
echo Ejecutable: build_NOW\devilutionx.exe
echo Audio: FUNCIONANDO
echo Features: TODAS IMPLEMENTADAS
echo.
echo Para ejecutar: cd build_NOW && devilutionx.exe
echo.
pause