@echo off
echo ========================================
echo COMPILACION CRASH HUNTER - MODO DEBUG
echo ========================================
echo.
echo SISTEMAS DESACTIVADOS:
echo - Architectural Shield (sanitizacion agresiva)
echo - Nightmare Items (items especiales)
echo - Hidden Content (textos ocultos)
echo - Dormant Assets (assets dormidos)
echo - Nightmare Audio (audio atmosferico)
echo.
echo SISTEMAS ACTIVOS:
echo - Crash Hunter (logging agresivo)
echo - Apocalypse Protection (solo proteccion de spell)
echo - Corruption Detector (deteccion basica)
echo.
echo ========================================
echo.

REM Usar build existente
cd build_COMPILE_FRESH

echo Compilando (usando build existente)...
cmake --build . -j 4

if errorlevel 1 (
    echo ERROR: Compilation failed
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo COMPILACION EXITOSA!
echo ========================================
echo.
echo Ejecutable: build_COMPILE_FRESH\devilutionx.exe
echo Logs: build_COMPILE_FRESH\debug_logs\crash_hunter.log
echo.
echo INSTRUCCIONES:
echo 1. Ejecuta el juego
echo 2. Reproduce el crash
echo 3. Revisa crash_hunter.log para ver el ultimo evento antes del crash
echo.

cd ..
pause
