@echo off
REM ========================================
REM TEST FASE 1 - DESACTIVACIONES
REM ========================================
REM Fecha: Enero 16, 2026
REM Objetivo: Testear si el crash desaparece
REM Sistemas desactivados:
REM - Light Mutations (3%)
REM - Depth Variants
REM - Elite Monsters (0.5%)
REM ========================================

echo.
echo ========================================
echo TESTING FASE 1 - DESACTIVACIONES
echo ========================================
echo.
echo Sistemas desactivados:
echo - 🚫 Light Mutations (3%%)
echo - 🚫 Depth Variants  
echo - 🚫 Elite Monsters (0.5%%)
echo.
echo ========================================
echo INICIANDO JUEGO...
echo ========================================
echo.

REM Limpiar logs anteriores
if exist "debug_logs\crash_hunter.log" del "debug_logs\crash_hunter.log"

REM Ejecutar el juego
echo Ejecutando devilutionx.exe...
echo.
echo INSTRUCCIONES:
echo 1. Carga tu save file
echo 2. Ve al nivel donde ocurre el crash
echo 3. Usa Apocalypse varias veces
echo 4. Si crashea, presiona Ctrl+C y reporta
echo 5. Si NO crashea, deja correr 5+ minutos
echo.
echo ========================================

start /wait devilutionx.exe

echo.
echo ========================================
echo TESTING COMPLETADO
echo ========================================
echo.

REM Revisar si hay logs
if exist "debug_logs\crash_hunter.log" (
    echo ✅ Log encontrado - revisando...
    echo.
    type "debug_logs\crash_hunter.log"
) else (
    echo ⚠️ No se encontró log de crash hunter
    echo Esto puede significar que el sistema no está compilado
)

echo.
echo ========================================
echo RESULTADO DEL TEST:
echo ========================================
echo.
echo ¿El juego crasheó? (S/N)
set /p crashed="Respuesta: "

if /i "%crashed%"=="S" (
    echo.
    echo ❌ CRASH PERSISTENTE
    echo → El culpable NO está en estos 3 sistemas
    echo → Proceder a FASE 2: Desactivar sistemas de generación
    echo.
) else (
    echo.
    echo ✅ NO CRASH DETECTADO
    echo → Uno de estos 3 sistemas ES el culpable
    echo → Proceder a reactivar uno por uno para identificar cuál
    echo.
)

echo ========================================
pause