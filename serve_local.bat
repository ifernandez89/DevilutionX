@echo off
echo 🌐 Sirviendo DevilutionX localmente
echo ================================

REM Verificar si Python está disponible
python --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ❌ Python no encontrado. Instalá Python para servir localmente.
    echo 💡 Alternativamente, abrí index.html directamente en el navegador.
    pause
    exit /b 1
)

echo 🚀 Iniciando servidor local en http://localhost:8000
echo 📁 Sirviendo desde: %CD%
echo.
echo 💡 Para probar:
echo    1. Ejecutá build_wasm_phase1.bat primero
echo    2. Abrí http://localhost:8000 en el navegador
echo    3. Verificá la consola (F12) para logs
echo.
echo ⏹️  Presioná Ctrl+C para detener el servidor
echo.

python -m http.server 8000