@echo off
title PC Vivaz 8-Bit Educational Computer
echo ==========================================================
echo  Iniciando Emulador de PC Vivaz / Educational Computer 2000
echo ==========================================================
cd /d "%~dp0"
if exist python.exe (
    python server.py
) else (
    where python >nul 2>nul
    if %errorlevel% equ 0 (
        python server.py
    ) else (
        echo Abriendo index.html en el navegador...
        start index.html
    )
)
pause
