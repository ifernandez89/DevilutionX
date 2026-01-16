@echo off
echo 🔥 Ejecutando DevilutionX Enhanced con Hellfire habilitado...
echo.
echo Esto permitirá que el libro Inferno y otros items de Hellfire aparezcan correctamente.
echo.

REM Buscar el ejecutable en diferentes ubicaciones posibles
if exist "devilutionx_enhanced.exe" (
    echo ✅ Encontrado: devilutionx_enhanced.exe
    start "" "devilutionx_enhanced.exe" --hellfire
) else if exist "build_NOW\devilutionx.exe" (
    echo ✅ Encontrado: build_NOW\devilutionx.exe
    start "" "build_NOW\devilutionx.exe" --hellfire
) else if exist "build_final\devilutionx.exe" (
    echo ✅ Encontrado: build_final\devilutionx.exe
    start "" "build_final\devilutionx.exe" --hellfire
) else if exist "build\devilutionx.exe" (
    echo ✅ Encontrado: build\devilutionx.exe
    start "" "build\devilutionx.exe" --hellfire
) else (
    echo ❌ No se encontró el ejecutable de DevilutionX Enhanced
    echo.
    echo Busca manualmente tu ejecutable y ejecútalo con: --hellfire
    echo Ejemplo: devilutionx.exe --hellfire
    pause
    exit /b 1
)

echo.
echo ✅ DevilutionX Enhanced iniciado con Hellfire habilitado
echo 📖 El libro Inferno debería aparecer ahora en tu inventario
echo.
pause