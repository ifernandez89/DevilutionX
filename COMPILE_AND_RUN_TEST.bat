@echo off
echo ========================================
echo APOCALYPSE REALISTIC TEST - CMAKE BUILD
echo ========================================
echo.

REM Crear directorio de build para el test
if not exist "build_test" mkdir build_test
cd build_test

echo Configurando con CMake...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Intentando con generador por defecto...
    cmake .. -DCMAKE_BUILD_TYPE=Release
    
    if %ERRORLEVEL% NEQ 0 (
        echo.
        echo ❌ ERROR: CMake configuration failed
        cd ..
        pause
        exit /b 1
    )
)

echo.
echo Compilando test...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ ERROR: Build failed
    cd ..
    pause
    exit /b 1
)

echo.
echo ✅ Compilacion exitosa
echo.
echo ========================================
echo EJECUTANDO TESTS...
echo ========================================
echo.

REM Ejecutar el test
if exist "Release\test_apocalypse_realistic.exe" (
    Release\test_apocalypse_realistic.exe
) else if exist "test_apocalypse_realistic.exe" (
    test_apocalypse_realistic.exe
) else (
    echo ❌ ERROR: No se encontro el ejecutable
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ========================================
echo TEST COMPLETADO
echo ========================================
echo.

pause
