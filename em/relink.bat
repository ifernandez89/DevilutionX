@echo off
call ..\emsdk\emsdk_env.bat
echo Relinking without strip to generate devilutionx.js...
emmake make devilutionx VERBOSE=1