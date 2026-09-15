@echo off
chcp 65001 >nul
title Sega Dreamcast - Ecco the Dolphin: Defender of the Future (Redream x64)

echo ===============================================================================
echo     🌀 SEGA DREAMCAST 128-BIT — ECCO THE DOLPHIN: DEFENDER OF THE FUTURE
echo                     Emulador de Alta Fidelidad Redream x64
echo ===============================================================================
echo.

set "REDREAM_DIR=%~dp0redream-win64"
set "REDREAM_EXE=%REDREAM_DIR%\redream.exe"

if not exist "%REDREAM_EXE%" (
    echo [INFO] Motor Redream x64 no detectado. Descargando versión portable oficial...
    if not exist "%REDREAM_DIR%" mkdir "%REDREAM_DIR%"
    curl -L -A "Mozilla/5.0 (Windows NT 10.0; Win64; x64)" -o "%~dp0redream.zip" "https://redream.io/download/redream.x86_64-windows-v1.5.0.zip"
    powershell -Command "Expand-Archive -Path '%~dp0redream.zip' -DestinationPath '%REDREAM_DIR%' -Force"
    del /f /q "%~dp0redream.zip" >nul 2>&1
)

set "TARGET_ROM="

if not "%~1"=="" (
    if exist "%~1" set "TARGET_ROM=%~1"
)

rem 1. Comprobacion de rutas prioritarias de CHD / CDI
if "%TARGET_ROM%"=="" (
    for %%e in (chd cdi gdi) do (
        if exist "D:\Hours of Fun\Roms\Sega Dreamcast\Ecco the Dolphin - Defender of the Future.%%e" (
            set "TARGET_ROM=D:\Hours of Fun\Roms\Sega Dreamcast\Ecco the Dolphin - Defender of the Future.%%e"
            goto :found_rom
        )
        if exist "%~dp0Ecco the Dolphin - Defender of the Future.%%e" (
            set "TARGET_ROM=%~dp0Ecco the Dolphin - Defender of the Future.%%e"
            goto :found_rom
        )
        if exist "%USERPROFILE%\Downloads\Ecco the Dolphin - Defender of the Future\Ecco the Dolphin - Defender of the Future.%%e" (
            set "TARGET_ROM=%USERPROFILE%\Downloads\Ecco the Dolphin - Defender of the Future\Ecco the Dolphin - Defender of the Future.%%e"
            goto :found_rom
        )
    )
)

rem 2. Busqueda recursiva de cualquier CHD o CDI de Ecco en Descargas
if "%TARGET_ROM%"=="" (
    echo [AVISO] Buscando imagen CHD / CDI de Ecco en carpeta de Descargas...
    for /r "%USERPROFILE%\Downloads" %%f in (*Ecco*.chd) do (
        set "TARGET_ROM=%%f"
        goto :found_rom
    )
    for /r "%USERPROFILE%\Downloads" %%f in (*Ecco*.cdi) do (
        set "TARGET_ROM=%%f"
        goto :found_rom
    )
)

:found_rom
if "%TARGET_ROM%"=="" (
    echo [AVISO] No se pudo localizar automáticamente el archivo CHD o CDI de Ecco the Dolphin.
    echo.
    echo Recuerda que puedes descargar el set completo Redump CHD desde:
    echo https://archive.org/download/sega-dreamcast-redump-collection
    echo.
    echo O puedes arrastrar y soltar el archivo .chd sobre este lanzador .bat.
    echo Abriendo la interfaz de Redream para seleccion directa...
    echo.
    start "" "%REDREAM_EXE%"
    pause
    exit /b 0
)

echo [OK] Imagen de disco Dreamcast detectada:
echo      "%TARGET_ROM%"
echo.
echo [INFO] Iniciando motor Redream a 60 FPS con BIOS integrada y mapeo automatico...
echo [INFO] Presione Alt+Enter para alternar Pantalla Completa.
echo [INFO] Presione Escape para abrir el menu rapido de Redream.
echo.

start "" "%REDREAM_EXE%" "%TARGET_ROM%"

echo [OK] Proceso lanzado en ventana independiente.
timeout /t 3 >nul
exit /b 0
