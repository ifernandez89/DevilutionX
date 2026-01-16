@echo off
echo ========================================
echo 🔮 ORACLE SYSTEM - OLLAMA STATUS TEST
echo ========================================
echo.

REM Test 1: Verificar si Ollama está corriendo
echo [TEST 1] Verificando si Ollama está corriendo...
curl -s http://localhost:11434/ >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ Ollama está CORRIENDO en localhost:11434
) else (
    echo ❌ Ollama NO está corriendo
    echo    Para iniciar Ollama: ollama serve
    goto :end
)
echo.

REM Test 2: Verificar si el modelo está disponible
echo [TEST 2] Verificando modelo qwen2.5:3b-instruct...
ollama list | findstr "qwen2.5:3b-instruct" >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ Modelo qwen2.5:3b-instruct está INSTALADO
) else (
    echo ❌ Modelo qwen2.5:3b-instruct NO está instalado
    echo    Para instalar: ollama pull qwen2.5:3b-instruct
    goto :end
)
echo.

REM Test 3: Hacer una consulta de prueba
echo [TEST 3] Haciendo consulta de prueba a Ollama...
echo {"model":"qwen2.5:3b-instruct","prompt":"The darkness grows. Say only: 'The Inferno awaits.'","stream":false,"options":{"temperature":0.2,"num_predict":20}} > temp_oracle_test.json

curl -s -X POST http://localhost:11434/api/generate ^
  -H "Content-Type: application/json" ^
  -d @temp_oracle_test.json > temp_oracle_response.json

if %errorlevel% equ 0 (
    echo ✅ Ollama respondió correctamente
    echo.
    echo Respuesta de Ollama:
    type temp_oracle_response.json | findstr "response"
) else (
    echo ❌ Error al consultar Ollama
)

del temp_oracle_test.json >nul 2>&1
del temp_oracle_response.json >nul 2>&1
echo.

:end
echo ========================================
echo.
echo CÓMO VERIFICAR EN EL JUEGO:
echo.
echo 1. MODO DEBUG (más fácil):
echo    - Compila en modo Debug: cmake -DCMAKE_BUILD_TYPE=Debug
echo    - Ejecuta el juego
echo    - Busca en la consola mensajes como:
echo      * "Oracle: Ollama query failed" = NO está usando Ollama
echo      * "Oracle: Ollama response: ..." = SÍ está usando Ollama
echo.
echo 2. MODO RELEASE (observación):
echo    - Textos PRESETEADOS: Siempre iguales, repetitivos
echo    - Textos de OLLAMA: Variaciones sutiles, nunca exactamente iguales
echo.
echo 3. VERIFICACIÓN VISUAL:
echo    - Muere varias veces en el mismo lugar
echo    - Si ves el MISMO texto exacto = Preseteados
echo    - Si ves VARIACIONES del texto = Ollama
echo.
echo 4. ARCHIVO DE LOG:
echo    - Busca en debug_logs/ archivos recientes
echo    - Grep por "Oracle:" para ver actividad
echo.
echo ========================================
pause
