@echo off
echo 🔧 Activando Emscripten...
cd emsdk
call emsdk_env.bat
cd ..

echo 🔨 Compilando...
cd build_webassembly_simple
emmake make -j 2

echo ✅ Compilación completada
pause