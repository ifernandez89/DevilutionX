@echo off
REM ========================================
REM ORACLE UI SYSTEM - Testing Commands
REM Fecha: Enero 16, 2026
REM ========================================

echo.
echo ========================================
echo  ORACLE UI SYSTEM - TESTING
echo ========================================
echo.

REM Paso 1: Git Pull
echo [1/3] Git pull de master...
git pull origin master
if %errorlevel% neq 0 (
    echo ERROR: Git pull failed
    pause
    exit /b 1
)
echo OK: Git pull completado
echo.

REM Paso 2: Compilar
echo [2/3] Compilando...
cmake --build build_NOW -j 4
if %errorlevel% neq 0 (
    echo ERROR: Compilacion fallida
    echo.
    echo Revisar errores arriba y consultar:
    echo ORACLE_UI_IMPLEMENTATION_TESTING_ENERO_16_2026.md
    pause
    exit /b 1
)
echo OK: Compilacion exitosa
echo.

REM Paso 3: Instrucciones de testing
echo [3/3] TESTING MANUAL
echo.
echo Ejecutar el juego y realizar estos tests:
echo.
echo 1. Mensaje de bienvenida (30s)
echo    - Iniciar juego
echo    - Buscar: "The Oracle awakens..." en esquina superior izquierda
echo    - Duracion: 4 segundos con fade
echo.
echo 2. Canal Debug (1 min)
echo    - NO abrir chat
echo    - Hacer pregunta al Oraculo
echo    - Buscar: Mensaje en x=10, y=50
echo.
echo 3. Canal Chat (1 min)
echo    - Presionar Enter (abrir chat)
echo    - Hacer pregunta al Oraculo
echo    - Buscar: Mensaje centrado arriba del panel
echo.
echo 4. Fade Visual (1 min)
echo    - Observar aparicion/desaparicion gradual
echo    - Verificar: 1s fade in + 2-3s visible + 1s fade out
echo.
echo 5. Solo 1 Mensaje (30s)
echo    - Hacer multiples preguntas rapido
echo    - Verificar: Solo se ve el ultimo mensaje
echo.
echo ========================================
echo  LISTO PARA TESTEAR
echo ========================================
echo.
echo Si hay problemas, consultar:
echo - ORACLE_UI_IMPLEMENTATION_TESTING_ENERO_16_2026.md
echo - test_oracle_ui.md
echo.
pause
