@echo off
echo ========================================
echo APOCALYPSE REALISTIC TEST
echo ========================================
echo.
echo Compilando test realista...
echo.

REM Compilar el test (standalone, no necesita DevilutionX)
g++ -std=c++17 -O2 test_apocalypse_realistic.cpp -o test_apocalypse_realistic.exe

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ ERROR: Compilacion fallida
    echo.
    echo Intentando con compilador alternativo...
    cl /EHsc /std:c++17 /O2 test_apocalypse_realistic.cpp /Fe:test_apocalypse_realistic.exe
    
    if %ERRORLEVEL% NEQ 0 (
        echo.
        echo ❌ ERROR: No se pudo compilar el test
        echo.
        echo Asegurate de tener g++ o MSVC instalado
        pause
        exit /b 1
    )
)

echo.
echo ✅ Compilacion exitosa
echo.
echo ========================================
echo EJECUTANDO TESTS...
echo ========================================
echo.

REM Ejecutar el test
test_apocalypse_realistic.exe

echo.
echo ========================================
echo TEST COMPLETADO
echo ========================================
echo.

pause
