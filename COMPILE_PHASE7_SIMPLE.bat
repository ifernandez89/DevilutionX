@echo off
echo 🤖 COMPILANDO FASE 7: AI System Integration (Método Simplificado)
echo ================================================================
echo.

echo 📊 Usando build directory existente para evitar problemas de configuración...

REM Use existing build directory that works
if exist build_NOW (
    echo ✅ Usando build_NOW existente
    cd build_NOW
) else if exist build_final_working (
    echo ✅ Usando build_final_working existente  
    cd build_final_working
) else if exist build_current (
    echo ✅ Usando build_current existente
    cd build_current
) else (
    echo ❌ No se encontró directorio de build funcional
    echo 🔧 Creando nuevo build con configuración mínima...
    mkdir build_ai_simple
    cd build_ai_simple
    
    echo 🔨 Configuración mínima...
    cmake .. -DCMAKE_BUILD_TYPE=Release
    
    if %ERRORLEVEL% neq 0 (
        echo ❌ Configuración falló
        pause
        exit /b 1
    )
)

echo.
echo 🔨 Compilando sistema AI integrado...
cmake --build . --config Release -j4

if %ERRORLEVEL% neq 0 (
    echo ❌ Compilación falló
    echo.
    echo 💡 Revisando errores de compilación...
    echo    - Verificar que todos los archivos AI estén presentes
    echo    - Comprobar sintaxis en archivos .cpp
    echo    - Revisar includes y dependencias
    echo.
    pause
    exit /b 1
)

echo.
echo ✅ COMPILACIÓN EXITOSA!
echo.
echo 🎉 FASE 7 AI SYSTEM - COMPILADO
echo ================================
echo.
echo 📊 Sistemas AI integrados:
echo    🤖 AIEngine: Core system
echo    🎯 QuestGenerator: Procedural quests  
echo    ⚖️ DifficultyAnalyzer: Adaptive difficulty
echo    💬 DialogueGenerator: Dynamic conversations
echo    🎁 ItemGenerator: Intelligent items
echo    📈 PlayerBehaviorTracker: Behavior analysis
echo    🔮 ContextualOracle: Enhanced Oracle
echo.

echo 🎮 Ejecutable: devilutionx.exe
echo 📁 Directorio: %CD%
echo.

echo 🚀 FASE 7 LISTA PARA TESTING
pause