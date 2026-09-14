@echo off
chcp 65001 >nul
title Sega Dreamcast - Ecco the Dolphin: Defender of the Future (Flycast x64)

echo ===============================================================================
echo     🌀 SEGA DREAMCAST 128-BIT — ECCO THE DOLPHIN: DEFENDER OF THE FUTURE
echo                     Emulador de Alta Fidelidad Flycast x64
echo ===============================================================================
echo.

set "FLYCAST_EXE=%~dp0flycast-win64\flycast.exe"
set "ECCO_ROM_PRIMARY=C:\Users\xiphos-pc1\Downloads\Ecco the Dolphin - Defender of the Future\Ecco the Dolphin - Defender of the Future.cdi"
set "ECCO_ROM_LOCAL=%~dp0Ecco the Dolphin - Defender of the Future.cdi"

if not exist "%FLYCAST_EXE%" (
    echo [ERROR] No se encontro el ejecutable de Flycast en:
    echo         "%FLYCAST_EXE%"
    echo.
    echo Por favor asegurese de que la carpeta flycast-win64 este presente.
    pause
    exit /b 1
)

set "TARGET_ROM="

if exist "%ECCO_ROM_PRIMARY%" (
    set "TARGET_ROM=%ECCO_ROM_PRIMARY%"
) else if exist "%ECCO_ROM_LOCAL%" (
    set "TARGET_ROM=%ECCO_ROM_LOCAL%"
)

if "%TARGET_ROM%"=="" (
    echo [AVISO] No se encontro la imagen CDI en la ruta esperada:
    echo         "%ECCO_ROM_PRIMARY%"
    echo.
    echo Buscando en carpetas de Descargas...
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
