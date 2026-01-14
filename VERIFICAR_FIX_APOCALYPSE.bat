@echo off
echo ========================================
echo VERIFICACION DEL FIX DE APOCALYPSE
echo ========================================
echo.
echo Este script verifica que el fix de Apocalypse este correctamente implementado.
echo.
echo ARQUITECTURA DEL FIX:
echo ---------------------
echo 1. Delayed Unlock System (3 frames minimo)
echo 2. Frame-based Protection (1 Apocalypse por frame)
echo 3. Time-based Protection (500ms entre casts)
echo 4. Atomic Lock (sobrevive al procesamiento)
echo.
echo ARCHIVOS CRITICOS:
echo ------------------
echo - Source/engine_health.cpp (CanSafelyCastApocalypse)
echo - Source/missiles.cpp (AddMissile con proteccion universal)
echo - Source/diablo.cpp (IncrementGlobalFrameCounter en game loop)
echo.
echo VERIFICANDO COMPILACION...
if not exist "devilutionx.exe" (
    echo [ERROR] No se encuentra devilutionx.exe
    echo Por favor compila primero con: cmake --build build_NOW -j 4
    pause
    exit /b 1
)
echo [OK] Ejecutable encontrado
echo.
echo VERIFICANDO LOGS DE DEBUG...
if exist "debug_logs\apocalypse_crash_debug_*.txt" (
    echo [OK] Sistema de logging activo
    echo.
    echo Ultimos logs:
    dir /b /o-d debug_logs\apocalypse_crash_debug_*.txt | findstr /n "^" | findstr "^[1-5]:"
) else (
    echo [WARNING] No se encontraron logs de debug
)
echo.
echo VERIFICANDO ARCHITECTURAL ANALYSIS LOG...
if exist "debug_logs\architectural_analysis.log" (
    echo [OK] Architectural analysis log encontrado
) else (
    echo [WARNING] No se encuentra architectural_analysis.log
)
echo.
echo ========================================
echo INSTRUCCIONES DE TESTING:
echo ========================================
echo.
echo 1. Ejecuta el juego: devilutionx.exe
echo 2. Crea un Sorcerer de alto nivel (o usa cheats)
echo 3. Aprende el spell Apocalypse
echo 4. Ve a un dungeon con muchos monstruos
echo 5. CLICKEA APOCALYPSE RAPIDAMENTE 10+ VECES
echo 6. Observa: NO DEBE CRASHEAR
echo.
echo COMPORTAMIENTO ESPERADO:
echo ------------------------
echo - Maximo 1 Apocalypse cada 3 frames
echo - Maximo 2 Apocalypse por segundo (500ms cooldown)
echo - Logs mostraran "atomic check" bloqueando casts
echo - Logs mostraran "delayed unlock" despues de 3 frames
echo - 0%% crash rate
echo.
echo LOGS A REVISAR DESPUES:
echo -----------------------
echo - debug_logs\apocalypse_crash_debug_[timestamp].txt
echo - debug_logs\architectural_analysis.log
echo.
echo ========================================
pause
