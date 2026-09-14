@echo off
chcp 65001 >nul
title Sega Dreamcast - Ecco the Dolphin: Defender of the Future (Flycast x64)

echo ===============================================================================
echo     🌀 SEGA DREAMCAST 128-BIT — ECCO THE DOLPHIN: DEFENDER OF THE FUTURE
echo                     Emulador de Alta Fidelidad Flycast x64
echo ===============================================================================
echo.

set "FLYCAST_EXE=%~dp0flycast-win64\flycast.exe"
set "ECCO_ROM_ROMS=D:\Hours of Fun\Roms\Sega Dreamcast\Ecco the Dolphin - Defender of the Future.cdi"
set "ECCO_ROM_DOWNLOADS=C:\Users\xiphos-pc1\Downloads\Ecco the Dolphin - Defender of the Future\Ecco the Dolphin - Defender of the Future.cdi"
set "ECCO_ROM_LOCAL=%~dp0Ecco the Dolphin - Defender of the Future.cdi"

if not exist "%FLYCAST_EXE%" (
    echo [INFO] Motor Flycast x64 no detectado. Descargando versión portable oficial...
    curl -L -o "%~dp0flycast.zip" "https://github.com/flyinghead/flycast/releases/download/v2.7/flycast-win64-2.7.zip"
    powershell -Command "Expand-Archive -Path '%~dp0flycast.zip' -DestinationPath '%~dp0flycast-win64' -Force"
    del /f /q "%~dp0flycast.zip" >nul 2>&1
)

set "TARGET_ROM="

if exist "%ECCO_ROM_ROMS%" (
    set "TARGET_ROM=%ECCO_ROM_ROMS%"
) else if exist "%ECCO_ROM_LOCAL%" (
    set "TARGET_ROM=%ECCO_ROM_LOCAL%"
) else if exist "%ECCO_ROM_DOWNLOADS%" (
    set "TARGET_ROM=%ECCO_ROM_DOWNLOADS%"
)

if "%TARGET_ROM%"=="" (
    echo [AVISO] Buscando imagen CDI en discos locales...
    for /r "%USERPROFILE%\Downloads" %%f in (*Ecco*.cdi) do (
        set "TARGET_ROM=%%f"
        goto :found_rom
    )
)

:found_rom
if "%TARGET_ROM%"=="" (
    echo [ERROR] No se pudo localizar el archivo CDI de Ecco the Dolphin.
    echo Arrastre y suelte la imagen .cdi sobre este archivo .bat para iniciarla directamente.
    echo Abriendo la interfaz de Flycast para seleccion manual...
    start "" "%FLYCAST_EXE%"
    pause
    exit /b 0
)

echo [OK] Imagen de disco Dreamcast detectada:
echo      "%TARGET_ROM%"
echo.
echo [INFO] Iniciando motor Flycast a 60 FPS con aceleracion 3D por hardware...
echo [INFO] Presione Alt+Enter para alternar Pantalla Completa.
echo [INFO] Para configurar mandos o teclas, use la pestana 'Controls' en el menu (Escape / Tab).
echo.

start "" "%FLYCAST_EXE%" "%TARGET_ROM%"

echo [OK] Proceso lanzado en ventana independiente.
timeout /t 3 >nul
exit /b 0
