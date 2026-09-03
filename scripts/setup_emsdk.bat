@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   DEVILUTIONX - INSTALADOR DE EMSCRIPTEN (EMSDK)
echo ===================================================

cd /d "%~dp0\.."

if not exist "deps" mkdir deps

if not exist "deps\emsdk" (
    echo [1/3] Clonando repositorio de Emscripten SDK en deps\emsdk...
    git clone https://github.com/emscripten-core/emsdk.git deps\emsdk
) else (
    echo [1/3] Repositorio deps\emsdk ya existe.
)

cd deps\emsdk

echo [2/3] Instalando Emscripten 3.1.53 (version oficial de DevilutionX)...
call emsdk.bat install 3.1.53

echo [3/3] Activando Emscripten 3.1.53...
call emsdk.bat activate 3.1.53

echo ===================================================
echo   EMSCRIPTEN INSTALADO Y CONFIGURADO CON EXITO!
echo   Ya puedes compilar con: scripts\build_wasm.bat
echo ===================================================
if "%~1" neq "--non-interactive" pause
