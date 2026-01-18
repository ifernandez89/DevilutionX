@echo off
cd phase4
call ..\emsdk\emsdk_env.bat
emmake make devilutionx
echo Build completed with exit code: %ERRORLEVEL%
if %ERRORLEVEL% equ 0 (
    echo ✅ FASE 4 COMPLETADA!
    echo 📁 Archivos generados:
    dir *.js *.wasm *.data 2>nul
) else (
    echo ❌ Compilación falló
)
pause