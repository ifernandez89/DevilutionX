@echo off
echo Compilando sistema avanzado de lluvia...

REM Compilar nightmare_weather.cpp
g++ -c Source/nightmare_weather.cpp -o nightmare_weather.o ^
    -I Source ^
    -I 3rdParty ^
    -I C:/mingw64/include ^
    -I C:/mingw64/include/SDL2 ^
    -std=c++17 ^
    -DENABLE_NIGHTMARE_WEATHER=1 ^
    -DUSE_SDL1=ON ^
    -O2

if %ERRORLEVEL% NEQ 0 (
    echo Error compilando nightmare_weather.cpp
    exit /b 1
)

echo ✅ Compilación exitosa!
echo Archivo objeto generado: nightmare_weather.o

REM Ahora necesitamos recompilar el ejecutable completo
echo Recompilando ejecutable completo...

g++ -o devilutionx_enhanced_rain_NEW.exe ^
    nightmare_weather.o ^
    -L C:/mingw64/lib ^
    -lSDL2 -lSDL2main -lSDL2_image ^
    -lmingw32 -lwinmm -limm32 -lole32 -loleaut32 -lversion -luuid ^
    -static-libgcc -static-libstdc++ ^
    -Wl,--subsystem,windows

if %ERRORLEVEL% NEQ 0 (
    echo Error enlazando ejecutable
    exit /b 1
)

echo ✅ ¡EJECUTABLE COMPILADO EXITOSAMENTE!
echo Archivo: devilutionx_enhanced_rain_NEW.exe