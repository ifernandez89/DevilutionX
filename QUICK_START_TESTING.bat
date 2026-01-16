@echo off
REM ========================================
REM QUICK START - Testing Completo
REM Fecha: Enero 16, 2026
REM ========================================

echo.
echo ========================================
echo  NIGHTMARE EDITION - TESTING COMPLETO
echo ========================================
echo.
echo Features implementadas hoy:
echo 1. Oracle UI System
echo 2. Fix boton friendly en single player
echo 3. Oracle fallback a textos dormidos
echo.
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
    echo Revisar errores y consultar documentacion:
    echo - ORACLE_UI_IMPLEMENTATION_TESTING_ENERO_16_2026.md
    echo - FIX_FRIENDLY_BUTTON_SINGLE_PLAYER_ENERO_16_2026.md
    echo - ORACLE_FALLBACK_DORMANT_TEXTS_ENERO_16_2026.md
    pause
    exit /b 1
)
echo OK: Compilacion exitosa
echo.

REM Paso 3: Instrucciones de testing
echo [3/3] TESTING MANUAL (17 minutos total)
echo.
echo ========================================
echo  TEST 1: ORACLE UI (5 minutos)
echo ========================================
echo.
echo 1. Mensaje de bienvenida (30s)
echo    - Iniciar juego
echo    - Buscar: "The Oracle awakens..." en esquina superior izquierda
echo.
echo 2. Canal Debug (1 min)
echo    - NO abrir chat
echo    - Hacer pregunta al Oraculo (morir, libro, altar)
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
echo  TEST 2: FRIENDLY BUTTON (2 minutos)
echo ========================================
echo.
echo 1. Single Player (1 min)
echo    - Iniciar juego single player
echo    - Verificar: Boton friendly NO visible
echo    - Solo se ve boton de chat
echo.
echo 2. Multiplayer (1 min)
echo    - Iniciar juego multiplayer
echo    - Verificar: Boton friendly SI visible
echo    - Toggle funciona correctamente
echo.
echo ========================================
echo  TEST 3: ORACLE FALLBACK (10 minutos)
echo ========================================
echo.
echo 1. Sin Ollama (3 min)
echo    - NO iniciar Ollama
echo    - Hacer preguntas al Oraculo
echo    - Verificar: Textos dormidos originales
echo.
echo 2. Con Ollama (3 min)
echo    - Iniciar: ollama serve
echo    - Hacer preguntas al Oraculo
echo    - Verificar: Variaciones IA unicas
echo.
echo 3. Ollama se cae (4 min)
echo    - Iniciar con Ollama
echo    - Hacer pregunta (ver respuesta IA)
echo    - Matar Ollama: killall ollama
echo    - Hacer pregunta (ver texto dormido)
echo    - Verificar: Transicion suave sin crash
echo.
echo ========================================
echo  LISTO PARA TESTEAR
echo ========================================
echo.
echo Si todo funciona:
echo   git add .
echo   git commit -m "feat: Oracle UI + Friendly button fix + Oracle fallback"
echo   git push origin master
echo.
echo Si hay problemas, consultar:
echo - ORACLE_UI_IMPLEMENTATION_TESTING_ENERO_16_2026.md
echo - FIX_FRIENDLY_BUTTON_SINGLE_PLAYER_ENERO_16_2026.md
echo - ORACLE_FALLBACK_DORMANT_TEXTS_ENERO_16_2026.md
echo - RESUMEN_FINAL_SESION_COMPLETA_ENERO_16_2026.md
echo.
pause
