@echo off
REM ============================================================================
REM SCRIPT DE COMPILACION DEFINITIVO - NIGHTMARE EDITION
REM Fecha: Enero 16, 2026
REM Branch: compilacionExitosa
REM Status: METODO PROBADO Y FUNCIONAL
REM ============================================================================

echo.
echo ========================================
echo  NIGHTMARE EDITION - COMPILACION
echo ========================================
echo.

REM ============================================================================
REM PASO 1: VERIFICAR DEPENDENCIAS CRITICAS
REM ============================================================================

echo [1/7] Verificando dependencias criticas...

if not exist "deps\SDL2-dev\SDL2-2.32.10\x86_64-w64-mingw32\lib\cmake\SDL2" (
    echo.
    echo [ERROR] SDL2 no encontrado en deps\SDL2-dev\
    echo Por favor restaurar desde backup o descargar SDL2-devel-2.32.10-mingw.tar.gz
    pause
    exit /b 1
)

if not exist "deps\zlib-build\libzlibstatic.a" (
    echo.
    echo [ERROR] zlib compilado no encontrado en deps\zlib-build\
    echo Por favor restaurar desde backup o recompilar zlib
    pause
    exit /b 1
)

if not exist "deps\zlib-src\zlib-1.3.1" (
    echo.
    echo [ERROR] zlib source no encontrado en deps\zlib-src\
    echo Por favor restaurar desde backup
    pause
    exit /b 1
)

echo [OK] Todas las dependencias encontradas

REM ============================================================================
REM PASO 2: VERIFICAR HERRAMIENTAS
REM ============================================================================

echo [2/7] Verificando herramientas de compilacion...

where gcc >nul 2>&1
if errorlevel 1 (
    echo.
    echo [ERROR] GCC no encontrado en PATH
    echo Por favor instalar MinGW-w64 y agregarlo al PATH
    pause
    exit /b 1
)

where cmake >nul 2>&1
if errorlevel 1 (
    echo.
    echo [ERROR] CMake no encontrado en PATH
    echo Por favor instalar CMake y agregarlo al PATH
    pause
    exit /b 1
)

echo [OK] GCC y CMake encontrados

REM ============================================================================
REM PASO 3: BACKUP DEL EJECUTABLE ANTERIOR
REM ============================================================================

echo [3/7] Haciendo backup del ejecutable anterior...

if exist "nightmare.exe" (
    copy /Y "nightmare.exe" "nightmare.exe.backup" >nul
    echo [OK] Backup creado: nightmare.exe.backup
) else (
    echo [INFO] No hay ejecutable anterior para backup
)

REM ============================================================================
REM PASO 4: DETERMINAR BUILD DIRECTORY
REM ============================================================================

echo [4/7] Determinando build directory...

set BUILD_DIR=build_COMPILE_FRESH

if exist "%BUILD_DIR%\CMakeCache.txt" (
    echo [INFO] Usando build directory existente: %BUILD_DIR%
    set BUILD_TYPE=incremental
) else (
    echo [INFO] Creando nuevo build directory: %BUILD_DIR%
    set BUILD_TYPE=clean
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
)

REM ============================================================================
REM PASO 5: CONFIGURAR CMAKE (solo si es build limpio)
REM ============================================================================

if "%BUILD_TYPE%"=="clean" (
    echo [5/7] Configurando CMake...
    
    cmake -S. -B%BUILD_DIR% -G "MinGW Makefiles" ^
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
    
    if errorlevel 1 (
        echo.
        echo [ERROR] Configuracion de CMake fallo
        echo Verificar rutas de dependencias en el script
        pause
        exit /b 1
    )
    
    echo [OK] CMake configurado exitosamente
) else (
    echo [5/7] Saltando configuracion (build incremental)
)

REM ============================================================================
REM PASO 6: COMPILAR
REM ============================================================================

echo [6/7] Compilando (esto puede tomar varios minutos)...
echo.

cmake --build %BUILD_DIR% -j 4

if errorlevel 1 (
    echo.
    echo [ERROR] Compilacion fallo
    echo.
    echo Posibles causas:
    echo - Errores de sintaxis en codigo
    echo - Dependencias faltantes
    echo - Build directory corrupto
    echo.
    echo Soluciones:
    echo 1. Verificar errores de compilacion arriba
    echo 2. Crear nuevo build directory limpio
    echo 3. Restaurar codigo desde backup
    echo.
    pause
    exit /b 1
)

echo.
echo [OK] Compilacion exitosa

REM ============================================================================
REM PASO 7: COPIAR EJECUTABLE
REM ============================================================================

echo [7/7] Copiando ejecutable...

if not exist "%BUILD_DIR%\devilutionx.exe" (
    echo.
    echo [ERROR] Ejecutable no encontrado en %BUILD_DIR%\devilutionx.exe
    echo La compilacion puede haber fallado silenciosamente
    pause
    exit /b 1
)

copy /Y "%BUILD_DIR%\devilutionx.exe" "nightmare.exe" >nul

if errorlevel 1 (
    echo.
    echo [ERROR] No se pudo copiar el ejecutable
    pause
    exit /b 1
)

echo [OK] Ejecutable copiado a nightmare.exe

REM ============================================================================
REM RESUMEN FINAL
REM ============================================================================

echo.
echo ========================================
echo  COMPILACION COMPLETADA EXITOSAMENTE
echo ========================================
echo.
echo Ejecutable: nightmare.exe
echo Build type: %BUILD_TYPE%
echo Build dir:  %BUILD_DIR%
echo.

if exist "nightmare.exe.backup" (
    echo Backup anterior: nightmare.exe.backup
    echo.
)

echo Para ejecutar el juego:
echo   nightmare.exe
echo.
echo Para testear:
echo   QUICK_START_TESTING.bat
echo.

pause
