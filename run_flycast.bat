@echo off
chcp 65001 >nul
title Sega Dreamcast - Redream (Reemplazo Oficial de Flycast)

echo ===============================================================================
echo      🌀 SEGA DREAMCAST — REDIRIGIENDO AL MOTOR OFICIAL REDREAM x64
echo ===============================================================================
echo.
echo [INFO] Redream ha reemplazado oficialmente a Flycast en el proyecto.
echo [INFO] Mayor compatibilidad, BIOS integrada y soporte nativo inmediato para CHD.
echo.

call "%~dp0run_redream.bat" %*
exit /b %ERRORLEVEL%
