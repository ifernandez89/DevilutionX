@echo off
set "PATH=C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
call "C:\Projects\DevilutionX\deps\emsdk\emsdk_env.bat"

echo === Pre-building Emscripten ports ===
call embuilder.bat build zlib bzip2 libpng sdl2 sdl2_image sdl2_image_png

echo === Configuring CMake with emcmake ===
call emcmake.bat cmake -S . -B build-web -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DASAN=OFF -DUBSAN=OFF -DTSAN=OFF -DDISABLE_ZERO_TIER=ON -DDISABLE_TCP=ON -DDEVILUTIONX_SYSTEM_SDL_AUDIOLIB=OFF -DDEVILUTIONX_SYSTEM_LIBSODIUM=OFF -DDEVILUTIONX_SYSTEM_LIBFMT=OFF -DDEVILUTIONX_SYSTEM_BZIP2=OFF

echo === Building WebAssembly binary ===
cmake --build build-web --target devilutionx -j 4
