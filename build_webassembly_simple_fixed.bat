@echo off
echo 🔧 Activando Emscripten...
cd emsdk
call emsdk_env.bat
cd ..

echo 🔧 Configurando build con NONET=ON...
cd build_webassembly_simple
emcmake cmake .. -DCMAKE_BUILD_TYPE=Debug -DNONET=ON -DNOSOUND=OFF

echo 🔨 Compilando...
emmake make -j 2

echo ✅ Compilación completada
pause