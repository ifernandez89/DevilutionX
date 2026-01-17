@echo off
REM ========================================
REM COMPILACION APOCALYPSE CRASH FIX
REM ========================================
REM Fecha: Enero 16, 2026
REM Fix: Deferred Loot Generation System
REM Build: build_COMPILE_FRESH
REM ========================================

echo.
echo ========================================
echo COMPILANDO APOCALYPSE CRASH FIX
echo ========================================
echo.
echo Build Directory: build_COMPILE_FRESH
echo Target: devilutionx.exe
echo Fix: Deferred Loot Generation
echo.

REM Matar procesos que puedan bloquear archivos
echo Cerrando procesos...
taskkill /F /IM devilutionx.exe 2>nul
taskkill /F /IM cc1plus.exe 2>nul
taskkill /F /IM ar.exe 2>nul
timeout /t 2 /nobreak >nul

echo.
echo Compilando...
cmake --build build_COMPILE_FRESH -j 4 --target devilutionx

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo ❌ ERROR EN COMPILACION
    echo ========================================
    echo.
    echo Revisa los errores arriba
    pause
    exit /b 1
)

echo.
echo ========================================
echo ✅ COMPILACION EXITOSA
echo ========================================
echo.

REM Copiar ejecutable
echo Copiando ejecutable...
copy /Y "build_COMPILE_FRESH\devilutionx.exe" "devilutionx.exe"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ⚠️ Error copiando ejecutable
    pause
    exit /b 1
)

echo.
echo ========================================
echo ✅ EJECUTABLE ACTUALIZADO
echo ========================================
echo.
echo Archivo: devilutionx.exe
echo Ubicacion: %CD%
echo.
echo Fix implementado:
echo - Deferred Loot Generation
echo - Logging de deferred loot
echo - Proteccion contra crash de Apocalypse
echo.
echo ========================================
echo LISTO PARA TESTING
echo ========================================
echo.
echo Ejecuta TEST_APOCALYPSE_FIX.bat para testear
echo.
pause
