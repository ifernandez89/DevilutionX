@echo off
echo ========================================
echo VERIFICACION NIGHTMARE EDITION
echo ========================================
echo.

echo Verificando archivos necesarios...
echo.

if exist "devilutionx.exe" (
    echo [OK] devilutionx.exe encontrado
) else (
    echo [ERROR] devilutionx.exe NO encontrado
    goto :error
)

if exist "DIABDAT.MPQ" (
    echo [OK] DIABDAT.MPQ encontrado
) else (
    echo [ERROR] DIABDAT.MPQ NO encontrado - NECESARIO PARA JUGAR
    goto :error
)

if exist "SDL2.dll" (
    echo [OK] SDL2.dll encontrado
) else (
    echo [ERROR] SDL2.dll NO encontrado
    goto :error
)

if exist "SDL2_image.dll" (
    echo [OK] SDL2_image.dll encontrado
) else (
    echo [ERROR] SDL2_image.dll NO encontrado
    goto :error
)

if exist "assets\ui_art\title.pcx" (
    echo [OK] Title screen personalizado encontrado
) else (
    echo [AVISO] Title screen personalizado no encontrado
)

echo.
echo ========================================
echo TODOS LOS ARCHIVOS NECESARIOS PRESENTES
echo ========================================
echo.
echo Puedes ejecutar el juego con: devilutionx.exe
echo.
pause
exit /b 0

:error
echo.
echo ========================================
echo ERROR: FALTAN ARCHIVOS NECESARIOS
echo ========================================
pause
exit /b 1
