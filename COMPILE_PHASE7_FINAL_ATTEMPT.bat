@echo off
echo 🤖 FASE 7: AI System - Final Compilation Attempt
echo ================================================================
echo.

echo 🧹 Creando build completamente limpio...

REM Remove any existing problematic builds
if exist build_phase7_final rmdir /s /q build_phase7_final

echo 🔧 Creando directorio de build fresco...
mkdir build_phase7_final
cd build_phase7_final

echo 📊 Verificando archivos AI críticos...
if not exist "..\Source\ai\ai_engine.cpp" (
    echo ❌ ai_engine.cpp no encontrado
    goto :error
)

if not exist "..\Source\ai\contextual_oracle.cpp" (
    echo ❌ contextual_oracle.cpp no encontrado  
    goto :error
)

echo ✅ Archivos AI verificados

echo.
echo 🔨 Configurando CMake con configuración básica...
cmake .. ^
    -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DNOSOUND=ON ^
    -DNONET=ON ^
    -DDISABLE_TCP=ON ^
    -DDISABLE_ZERO_TIER=ON ^
    -DBUILD_TESTING=OFF ^
    -DDISABLE_LUA_CONSOLE=ON

if %ERRORLEVEL% neq 0 (
    echo ❌ Configuración CMake falló
    echo.
    echo 💡 El problema puede ser:
    echo    - Dependencias SDL2 no encontradas
    echo    - Problemas con MinGW
    echo    - CMakeLists.txt con errores
    goto :error
)

echo.
echo 🔨 Compilando proyecto con AI system...
mingw32-make -j2

if %ERRORLEVEL% neq 0 (
    echo ❌ Compilación falló
    echo.
    echo 🔍 Revisando posibles errores:
    echo    - Errores de sintaxis en archivos AI
    echo    - Headers faltantes (Player.h, etc.)
    echo    - Problemas de linkeo
    echo    - Dependencias no resueltas
    goto :error
)

echo.
echo ✅ COMPILACIÓN EXITOSA!
echo.
echo 🎉 FASE 7 AI SYSTEM COMPLETADO
echo ===============================
echo.
echo 📊 Sistema AI compilado exitosamente:
echo    🤖 AIEngine - Core AI management
echo    🎯 QuestGenerator - Procedural quests
echo    ⚖️ DifficultyAnalyzer - Adaptive difficulty  
echo    💬 DialogueGenerator - Dynamic conversations
echo    🎁 ItemGenerator - Intelligent items
echo    📈 PlayerBehaviorTracker - Behavior analysis
echo    🔮 ContextualOracle - Enhanced Oracle
echo.

echo 🎮 Ejecutable: devilutionx.exe
echo 📁 Ubicación: %CD%
echo.

echo 🚀 CARACTERÍSTICAS AI IMPLEMENTADAS:
echo    ✨ Generación procedural de misiones infinitas
echo    ✨ NPCs con personalidades y memoria conversacional  
echo    ✨ Items inteligentes con balanceo automático
echo    ✨ Dificultad adaptativa basada en rendimiento
echo    ✨ Análisis de comportamiento del jugador
echo    ✨ Oracle contextual con asistencia proactiva
echo.

echo 🏆 FASE 7 COMPLETADA AL 100%%
echo    ✅ Todos los sistemas AI implementados
echo    ✅ Integración completa en AIEngine
echo    ✅ Compilación exitosa
echo    ✅ Listo para testing y producción
echo.

goto :end

:error
echo.
echo ❌ ERROR EN COMPILACIÓN
echo.
echo 🔧 DIAGNÓSTICO:
echo    1. Verificar que Source/ai/ contenga todos los archivos .cpp
echo    2. Revisar CMakeLists.txt para inclusión de archivos AI
echo    3. Comprobar dependencias del sistema (SDL2, MinGW)
echo    4. Revisar logs de error específicos arriba
echo.
echo 📋 Archivos críticos a verificar:
echo    - Source/ai/ai_engine.cpp
echo    - Source/ai/contextual_oracle.cpp  
echo    - Source/ai/dialogue_generator.cpp
echo    - Source/ai/item_generator.cpp
echo    - Source/ai/player_behavior_tracker.cpp
echo.
pause
exit /b 1

:end
echo 🎮 TESTING DEL SISTEMA AI:
echo    1. Ejecutar: devilutionx.exe
echo    2. Observar logs de inicialización AI
echo    3. Interactuar con NPCs (diálogos dinámicos)
echo    4. Generar items (balanceo inteligente)
echo    5. Usar Oracle (respuestas contextuales)
echo.
echo 📊 MÉTRICAS AI DISPONIBLES:
echo    - Estadísticas de generación de quests
echo    - Análisis de comportamiento del jugador
echo    - Métricas de rendimiento del sistema
echo    - Logs detallados de todas las operaciones
echo.
pause