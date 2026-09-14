@echo off
chcp 65001 >nul
title Sega Dreamcast / Naomi - Flycast x64

echo ===============================================================================
echo            🌀 FLYCAST x64 — SEGA DREAMCAST / NAOMI / ATOMISWAVE
echo ===============================================================================
echo.

set "FLYCAST_EXE=%~dp0flycast-win64\flycast.exe"

if not exist "%FLYCAST_EXE%" (
    echo [ERROR] No se encontro el ejecutable de Flycast en:
    echo         "%FLYCAST_EXE%"
    pause
    exit /b 1
)

if "%~1"=="" (
    echo [INFO] Iniciando menu principal de Flycast...
    start "" "%FLYCAST_EXE%"
) else (
    echo [INFO] Iniciando imagen de disco seleccionada:
    echo        "%~1"
    start "" "%FLYCAST_EXE%" "%~1"
)

exit /b 0
