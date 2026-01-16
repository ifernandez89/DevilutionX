@echo off
REM ============================================================================
REM SCRIPT DE VERIFICACION DE DEPENDENCIAS CRITICAS
REM Fecha: Enero 16, 2026
REM Branch: compilacionExitosa
REM ============================================================================

echo.
echo ========================================
echo  VERIFICACION DE DEPENDENCIAS CRITICAS
echo ========================================
echo.

set ERROR_COUNT=0
set WARN_COUNT=0

REM ============================================================================
REM VERIFICAR SDL2
REM ============================================================================

echo [1/8] Verificando SDL2...

if exist "deps\SDL2-dev\SDL2-2.32.10\x86_64-w64-mingw32\lib\cmake\SDL2\SDL2Config.cmake" (
    echo [OK] SDL2 CMake config encontrado
) else (
    echo [ERROR] SDL2 CMake config NO encontrado
    echo        Ruta: deps\SDL2-dev\SDL2-2.32.10\x86_64-w64-mingw32\lib\cmake\SDL2\
    set /a ERROR_COUNT+=1
)

if exist "deps\SDL2-dev\SDL2-2.32.10\x86_64-w64-mingw32\bin\SDL2.dll" (
    echo [OK] SDL2.dll encontrado
) else (
    echo [ERROR] SDL2.dll NO encontrado
    set /a ERROR_COUNT+=1
)

REM ============================================================================
REM VERIFICAR ZLIB COMPILADO
REM ============================================================================

echo.
echo [2/8] Verificando zlib compilado...

if exist "deps\zlib-build\libzlibstatic.a" (
    echo [OK] libzlibstatic.a encontrado
    for %%A in ("deps\zlib-build\libzlibstatic.a") do (
        if %%~zA LSS 50000 (
            echo [WARN] libzlibstatic.a parece muy pequeno (%%~zA bytes)
            set /a WARN_COUNT+=1
        ) else (
            echo [OK] Tamano correcto (%%~zA bytes)
        )
    )
) else (
    echo [ERROR] libzlibstatic.a NO encontrado
    echo        Ruta: deps\zlib-build\
    echo        Solucion: Recompilar zlib (ver DEPENDENCIAS_CRITICAS_NO_BORRAR.md)
    set /a ERROR_COUNT+=1
)

REM ============================================================================
REM VERIFICAR ZLIB SOURCE
REM ============================================================================

echo.
echo [3/8] Verificando zlib source...

if exist "deps\zlib-src\zlib-1.3.1\zlib.h" (
    echo [OK] zlib.h encontrado
) else (
    echo [ERROR] zlib.h NO encontrado
    echo        Ruta: deps\zlib-src\zlib-1.3.1\
    set /a ERROR_COUNT+=1
)

if exist "deps\zlib-src\zlib-1.3.1\zconf.h" (
    echo [OK] zconf.h encontrado
) else (
    echo [ERROR] zconf.h NO encontrado
    set /a ERROR_COUNT+=1
)

REM ============================================================================
REM VERIFICAR DIABDAT.MPQ
REM ============================================================================

echo.
echo [4/8] Verificando DIABDAT.MPQ...

if exist "DIABDAT.MPQ" (
    echo [OK] DIABDAT.MPQ encontrado
    for %%A in ("DIABDAT.MPQ") do (
        if %%~zA LSS 1000000 (
            echo [WARN] DIABDAT.MPQ parece muy pequeno (%%~zA bytes)
            set /a WARN_COUNT+=1
        ) else (
            echo [OK] Tamano correcto (%%~zA bytes)
        )
    )
) else (
    echo [ERROR] DIABDAT.MPQ NO encontrado
    echo        Este archivo es necesario para ejecutar el juego
    echo        Copiar desde CD de Diablo 1 o backup
    set /a ERROR_COUNT+=1
)

REM ============================================================================
REM VERIFICAR BUILD DIRECTORIES
REM ============================================================================

echo.
echo [5/8] Verificando build directories...

set BUILD_FOUND=0

if exist "build_SAFE" (
    echo [OK] build_SAFE encontrado
    set BUILD_FOUND=1
)

if exist "build_SAFE2" (
    echo [OK] build_SAFE2 encontrado
    set BUILD_FOUND=1
)

if exist "build_COMPILE_FRESH" (
    echo [OK] build_COMPILE_FRESH encontrado
    set BUILD_FOUND=1
)

if %BUILD_FOUND%==0 (
    echo [WARN] No se encontro ningun build directory de backup
    echo        Recomendado: Mantener al menos build_SAFE o build_SAFE2
    set /a WARN_COUNT+=1
)

REM ============================================================================
REM VERIFICAR EJECUTABLE
REM ============================================================================

echo.
echo [6/8] Verificando ejecutable...

if exist "nightmare.exe" (
    echo [OK] nightmare.exe encontrado
    for %%A in ("nightmare.exe") do (
        if %%~zA LSS 1000000 (
            echo [WARN] nightmare.exe parece muy pequeno (%%~zA bytes)
            echo        Puede estar corrupto o incompleto
            set /a WARN_COUNT+=1
        ) else (
            echo [OK] Tamano correcto (%%~zA bytes)
        )
    )
) else (
    echo [WARN] nightmare.exe NO encontrado
    echo        Compilar con COMPILE_NIGHTMARE_DEFINITIVO.bat
    set /a WARN_COUNT+=1
)

REM ============================================================================
REM VERIFICAR HERRAMIENTAS
REM ============================================================================

echo.
echo [7/8] Verificando herramientas de compilacion...

where gcc >nul 2>&1
if errorlevel 1 (
    echo [ERROR] GCC no encontrado en PATH
    echo        Instalar MinGW-w64 y agregarlo al PATH
    set /a ERROR_COUNT+=1
) else (
    for /f "tokens=*" %%i in ('gcc --version ^| findstr /C:"gcc"') do (
        echo [OK] GCC encontrado: %%i
    )
)

where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake no encontrado en PATH
    echo        Instalar CMake y agregarlo al PATH
    set /a ERROR_COUNT+=1
) else (
    for /f "tokens=*" %%i in ('cmake --version ^| findstr /C:"cmake version"') do (
        echo [OK] CMake encontrado: %%i
    )
)

REM ============================================================================
REM VERIFICAR ESPACIO EN DISCO
REM ============================================================================

echo.
echo [8/8] Verificando espacio en disco...

for /f "tokens=3" %%a in ('dir /-c ^| findstr /C:"bytes free"') do set FREE_SPACE=%%a

REM Convertir a numero (remover comas)
set FREE_SPACE=%FREE_SPACE:,=%

REM Verificar si hay al menos 2GB libres (2147483648 bytes)
if %FREE_SPACE% LSS 2147483648 (
    echo [WARN] Espacio en disco bajo (menos de 2GB libres)
    echo        Recomendado: Al menos 2-3GB libres para compilar
    set /a WARN_COUNT+=1
) else (
    echo [OK] Espacio en disco suficiente
)

REM ============================================================================
REM RESUMEN FINAL
REM ============================================================================

echo.
echo ========================================
echo  RESUMEN DE VERIFICACION
echo ========================================
echo.

if %ERROR_COUNT%==0 (
    if %WARN_COUNT%==0 (
        echo [EXCELENTE] Todas las dependencias estan correctas
        echo.
        echo Puedes compilar con:
        echo   COMPILE_NIGHTMARE_DEFINITIVO.bat
        echo.
    ) else (
        echo [BIEN] Dependencias criticas OK, pero hay %WARN_COUNT% advertencia(s)
        echo.
        echo Las advertencias no impiden compilar, pero es recomendado resolverlas.
        echo.
    )
) else (
    echo [ERROR] Se encontraron %ERROR_COUNT% error(es) critico(s)
    echo.
    echo NO puedes compilar hasta resolver estos errores.
    echo.
    echo Soluciones:
    echo 1. Restaurar deps\ desde backup
    echo 2. Descargar dependencias faltantes
    echo 3. Ver DEPENDENCIAS_CRITICAS_NO_BORRAR.md para mas info
    echo.
)

echo Errores:       %ERROR_COUNT%
echo Advertencias:  %WARN_COUNT%
echo.

pause
