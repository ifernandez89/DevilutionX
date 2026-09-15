@echo off
chcp 65001 >nul
title Sega Dreamcast - Ecco the Dolphin: Defender of the Future (Redream)

echo ===============================================================================
echo     🌀 SEGA DREAMCAST 128-BIT — ECCO THE DOLPHIN (REDREAM v1.5.0)
echo                   Emulador Nativo de Alta Fidelidad a 60 FPS
echo ===============================================================================
echo.

set "REDREAM_EXE=%USERPROFILE%\Downloads\redream-win64\redream.exe"
if not exist "%REDREAM_EXE%" set "REDREAM_EXE=%~dp0redream-win64\redream.exe"
if not exist "%REDREAM_EXE%" set "REDREAM_EXE=%~dp0redream.exe"

set "ECCO_CHD=%USERPROFILE%\Downloads\Ecco the Dolphin - Defender of the Future (USA) (En,Fr,De,Es).chd"
if not exist "%ECCO_CHD%" set "ECCO_CHD=%~dp0Ecco the Dolphin - Defender of the Future (USA) (En,Fr,De,Es).chd"

if not exist "%REDREAM_EXE%" (
    echo [ERROR] No se encontró redream.exe en %USERPROFILE%\Downloads\redream-win64\
    echo Por favor verifica la ubicación de Redream.
    pause
    exit /b 1
)

echo [OK] Redream detectado: "%REDREAM_EXE%"

if exist "%ECCO_CHD%" (
    echo [OK] Imagen de Ecco the Dolphin detectada (.CHD 552 MB):
    echo      "%ECCO_CHD%"
    echo.
    echo ===============================================================================
    echo                       GUÍA RÁPIDA DE CONTROLES (TECLADO)
    echo ===============================================================================
    echo   • W / A / S / D     : Nadar / Mover a Ecco (Palanca analógica)
    echo   • K                 : Botón A (Nadar rápido / Acelerar / Interactuar)
    echo   • L                 : Botón B (Embestida / Ataque / Dash)
    echo   • J                 : Botón X (Sonar / Ecolocalización / Cantar)
    echo   • P o I             : Botón Y (Cámara / Vista libre)
    echo   • Q                 : Gatillo L (Giro cerrado izquierda)
    echo   • E                 : Gatillo R (Giro cerrado derecha)
    echo   • BARRA ESPACIADORA : START (Pausa / Menú)
    echo   • ESC               : Abrir menú de Redream (Ajustes, INPUT, salir)
    echo ===============================================================================
    echo.
    echo Iniciando Ecco the Dolphin en Redream...
    start "" "%REDREAM_EXE%" "%ECCO_CHD%"
) else (
    echo [AVISO] No se encontró automáticamente el archivo CHD de Ecco.
    echo Iniciando Redream para selección de juego...
    start "" "%REDREAM_EXE%"
)

timeout /t 3 >nul
exit /b 0
