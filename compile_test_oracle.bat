@echo off
REM Compilar test del Oráculo
REM Requiere: MinGW-w64 con g++, SDL2

echo ========================================
echo Compilando Test del Oraculo
echo ========================================
echo.

REM Verificar que g++ está disponible
where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: g++ no encontrado
    echo Instala MinGW-w64 o usa MSYS2
    pause
    exit /b 1
)

echo Compilando...
g++ -std=c++17 ^
    test_oracle_complete.cpp ^
    Source/oracle/oracle_dormant_texts.cpp ^
    Source/oracle/oracle_prompt.cpp ^
    Source/oracle/oracle_ollama.cpp ^
    Source/oracle/oracle_validator.cpp ^
    Source/oracle/oracle_cache.cpp ^
    Source/utils/str_cat.cpp ^
    Source/utils/str_case.cpp ^
    Source/utils/paths.cpp ^
    Source/utils/file_util.cpp ^
    -I. -ISource ^
    -lwinhttp -lSDL2 -lSDL2main ^
    -o test_oracle_complete.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Compilacion exitosa!
    echo ========================================
    echo.
    echo Ejecuta: test_oracle_complete.exe
    echo.
) else (
    echo.
    echo ========================================
    echo ERROR en la compilacion
    echo ========================================
    echo.
    echo Verifica que tienes:
    echo - MinGW-w64 instalado
    echo - SDL2 en el PATH
    echo - Todas las dependencias
    echo.
)

pause
