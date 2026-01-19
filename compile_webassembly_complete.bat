@echo off
echo 🔧 Activando Emscripten...
cd emsdk
call emsdk_env.bat
cd ..

echo 🔨 Compilando WebAssembly con NONET=ON...
cd build_webassembly_simple
emmake make -j 2

echo ✅ Compilación completada
echo 📁 Archivos generados:
dir *.wasm *.js *.data 2>nul

pause