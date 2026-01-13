@echo off
echo 🚫 Compilando DevilutionX SIN sistema de clima...

REM Crear backup del header original
copy Source\nightmare_weather.h Source\nightmare_weather.h.backup

REM Desactivar el sistema de clima
powershell -Command "(Get-Content Source\nightmare_weather.h) -replace '#define ENABLE_NIGHTMARE_WEATHER 1', '#define ENABLE_NIGHTMARE_WEATHER 0' | Set-Content Source\nightmare_weather.h"

echo ✅ Sistema de clima DESACTIVADO temporalmente

REM Intentar compilar usando cmake
echo 🔨 Intentando compilar...

cmake -S. -Bbuild_NO_WEATHER -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DNONET=ON -DDEVILUTIONX_SYSTEM_ZLIB=OFF

if %ERRORLEVEL% NEQ 0 (
    echo ❌ Error en configuración CMake
    goto restore
)

cmake --build build_NO_WEATHER -j 4

if %ERRORLEVEL% NEQ 0 (
    echo ❌ Error en compilación
    goto restore
)

REM Copiar ejecutable si la compilación fue exitosa
if exist build_NO_WEATHER\devilutionx.exe (
    copy build_NO_WEATHER\devilutionx.exe devilutionx_NO_WEATHER.exe
    echo ✅ ¡Ejecutable SIN clima compilado exitosamente!
    echo 📁 Archivo: devilutionx_NO_WEATHER.exe
) else (
    echo ❌ No se encontró el ejecutable compilado
)

:restore
REM Restaurar el header original
copy Source\nightmare_weather.h.backup Source\nightmare_weather.h
del Source\nightmare_weather.h.backup

echo 🔄 Header restaurado al estado original

if exist devilutionx_NO_WEATHER.exe (
    echo 🎯 COMPARACIÓN LISTA:
    echo    - CON clima: devilutionx_enhanced_rain.exe
    echo    - SIN clima: devilutionx_NO_WEATHER.exe
) else (
    echo ❌ Compilación falló - solo tienes la versión CON clima
)