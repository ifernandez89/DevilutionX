@echo off
cd phase4
call ..\emsdk\emsdk_env.bat
emmake make devilutionx
echo Build completed with exit code: %ERRORLEVEL%
pause