@echo off
echo ========================================
echo COMPILACION FOCUS FIX - METODO DEFINITIVO
echo ========================================

REM Limpiar build anterior
if exist build_FOCUS_FIX rmdir /s /q build_FOCUS_FIX

REM Configurar CMAKE con parametros que funcionan
cmake -S. -Bbuild_FOCUS_FIX -G "MinGW Makefiles" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DNONET=ON ^
  -DNOSOUND=ON ^
  -DBUILD_TESTING=OFF ^
  -DDISABLE_ZERO_TIER=ON ^
  -DUNPACKED_MPQS=OFF ^
  -DUNPACKED_SAVES=OFF ^
  -DDEVILUTIONX_STATIC_BZIP2=ON ^
  -DDEVILUTIONX_SYSTEM_BZIP2=OFF ^
  -DSDL2_DIR="deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/lib/cmake/SDL2" ^
  -DZLIB_LIBRARY="C:/c - c++/DevilutionX/deps/zlib-build/libzlibstatic.a"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMAKE configuration failed
    pause
    exit /b 1
)

REM Compilar
cmake --build build_FOCUS_FIX --config Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Compilation failed
    pause
    exit /b 1
)

echo ========================================
echo COMPILACION EXITOSA!
echo Ejecutable: build_FOCUS_FIX/devilutionx.exe
echo ========================================
pause