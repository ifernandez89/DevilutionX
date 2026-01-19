@echo off
echo 🔥 COMANDOS RAPIDOS - DIABLO WEBASSEMBLY 🔥
echo.

echo 🔧 COMPILAR WEBASSEMBLY:
echo cd emsdk ^&^& call emsdk_env.bat ^&^& cd .. ^&^& cd build_webassembly_simple ^&^& emcmake cmake .. -DCMAKE_BUILD_TYPE=Debug -DNONET=ON -DNOSOUND=OFF ^&^& emmake make -j 2
echo.

echo 🌐 SERVIDOR LOCAL:
echo cd docs ^&^& python -m http.server 8000
echo Luego abrir: http://localhost:8000/diablo_working_final.html
echo.

echo 📁 ARCHIVOS PRINCIPALES:
echo docs/diablo_working_final.html - Pagina principal
echo docs/devilutionx.wasm - Binario WebAssembly (78MB)
echo docs/devilutionx.js - Runtime JavaScript
echo docs/devilutionx.data - Datos del juego
echo.

echo 🚀 GIT COMMANDS:
echo git add . ^&^& git commit -m "Update" ^&^& git push origin sistemaWeb
echo.

echo ✅ ESTADO: PROYECTO COMPLETADO EXITOSAMENTE
pause