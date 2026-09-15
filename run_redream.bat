@echo off
chcp 65001 >nul
title Sega Dreamcast - Redream x64

echo ===============================================================================
echo            🌀 REDREAM x64 — SEGA DREAMCAST EMULATOR
echo ===============================================================================
echo.

set "REDREAM_DIR=%~dp0redream-win64"
set "REDREAM_EXE=%REDREAM_DIR%\redream.exe"

if not exist "%REDREAM_EXE%" (
    if exist "%USERPROFILE%\Downloads\redream-win64\redream.exe" (
        set "REDREAM_EXE=%USERPROFILE%\Downloads\redream-win64\redream.exe"
    )
)

if not exist "%REDREAM_EXE%" (
    echo [INFO] Motor Redream x64 no detectado. Descargando version portable oficial...
    if not exist "%REDREAM_DIR%" mkdir "%REDREAM_DIR%"
    curl -L -A "Mozilla/5.0 (Windows NT 10.0; Win64; x64)" -o "%~dp0redream.zip" "https://redream.io/download/redream.x86_64-windows-v1.5.0.zip"
    powershell -Command "Expand-Archive -Path '%~dp0redream.zip' -DestinationPath '%REDREAM_DIR%' -Force"
    del /f /q "%~dp0redream.zip" >nul 2>&1
)

if not exist "%REDREAM_EXE%" (
    echo [ERROR] No se pudo descargar o extraer Redream en:
    echo         "%REDREAM_EXE%"
    pause
    exit /b 1
)

if "%~1"=="" (
    echo [INFO] Iniciando interfaz principal de Redream...
    start "" "%REDREAM_EXE%"
) else (
    echo [INFO] Iniciando imagen de disco seleccionada:
    echo        "%~1"
    start "" "%REDREAM_EXE%" "%~1"
)

exit /b 0
