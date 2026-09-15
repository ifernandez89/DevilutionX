@echo off
chcp 65001 >nul
title Sega Dreamcast - Redream x64

echo ===============================================================================
echo            🌀 REDREAM x64 — EMULADOR SEGA DREAMCAST NATIVO
echo ===============================================================================
echo.

set "REDREAM_EXE=%USERPROFILE%\Downloads\redream-win64\redream.exe"
if not exist "%REDREAM_EXE%" set "REDREAM_EXE=%~dp0redream-win64\redream.exe"
if not exist "%REDREAM_EXE%" set "REDREAM_EXE=%~dp0redream.exe"

if not exist "%REDREAM_EXE%" (
    echo [ERROR] No se encontró redream.exe en:
    echo         "%REDREAM_EXE%"
    pause
    exit /b 1
)

if "%~1"=="" (
    echo [INFO] Iniciando Redream...
    start "" "%REDREAM_EXE%"
) else (
    echo [INFO] Iniciando imagen: "%~1"
    start "" "%REDREAM_EXE%" "%~1"
)

exit /b 0
