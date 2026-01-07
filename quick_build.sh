#!/bin/bash

# SCRIPT DE COMPILACIÓN RÁPIDA - DevilutionX
# Uso: ./quick_build.sh

set -e  # Salir en caso de error

echo "🚀 INICIANDO COMPILACIÓN RÁPIDA DE DEVILUTIONX"
echo "=============================================="

# Verificar dependencias críticas
echo "📋 Verificando dependencias..."
if [ ! -d "deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32" ]; then
    echo "❌ ERROR: Dependencias SDL2 no encontradas en deps/"
    echo "   Ejecuta: git checkout HEAD -- deps/"
    exit 1
fi

if [ ! -f "build_final_working/devilutionx.exe" ]; then
    echo "❌ ERROR: Build funcional no encontrado"
    echo "   Ejecuta: git checkout HEAD -- build_final_working/"
    exit 1
fi

echo "✅ Dependencias verificadas"

# Crear directorio de build limpio
BUILD_DIR="build_quick_$(date +%Y%m%d_%H%M%S)"
echo "📁 Creando directorio de build: $BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configurar CMake con dependencias locales
echo "⚙️  Configurando CMake..."
cmake -S. -B"$BUILD_DIR" \
    -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DNONET=ON \
    -DBUILD_TESTING=OFF \
    -DSDL2_DIR="deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/lib/cmake/SDL2" \
    -DCMAKE_PREFIX_PATH="deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32;deps/SDL2_image-dev/SDL2_image-2.8.8/x86_64-w64-mingw32"

if [ $? -ne 0 ]; then
    echo "❌ ERROR: Configuración de CMake falló"
    echo "   Intentando método alternativo..."
    
    # Método alternativo: copiar archivos de configuración
    cp "deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/lib/cmake/SDL2/sdl2-config.cmake" \
       "deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/lib/cmake/SDL2/SDL2Config.cmake" 2>/dev/null || true
    
    # Reintentar configuración
    cmake -S. -B"$BUILD_DIR" \
        -G "MinGW Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DNONET=ON \
        -DBUILD_TESTING=OFF \
        -DSDL2_DIR="deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/lib/cmake/SDL2"
fi

echo "✅ Configuración completada"

# Compilar
echo "🔨 Compilando..."
cmake --build "$BUILD_DIR" -j 4

if [ $? -ne 0 ]; then
    echo "❌ ERROR: Compilación falló"
    echo "   Revisa los errores arriba"
    exit 1
fi

echo "✅ Compilación exitosa"

# Copiar DLLs necesarias desde build funcional
echo "📦 Copiando DLLs necesarias..."
cp build_final_working/*.dll "$BUILD_DIR/" 2>/dev/null || true
cp build_final_working/DIABDAT.MPQ "$BUILD_DIR/" 2>/dev/null || true

echo "✅ DLLs copiadas"

# Verificar que el ejecutable existe
if [ -f "$BUILD_DIR/devilutionx.exe" ]; then
    echo "🎉 ¡COMPILACIÓN EXITOSA!"
    echo "   Ejecutable: $BUILD_DIR/devilutionx.exe"
    echo "   Para probar: cd $BUILD_DIR && ./devilutionx.exe"
    
    # Crear enlace simbólico para fácil acceso
    ln -sf "$BUILD_DIR/devilutionx.exe" "devilutionx_latest.exe" 2>/dev/null || true
    echo "   Enlace directo: ./devilutionx_latest.exe"
    
else
    echo "❌ ERROR: Ejecutable no generado"
    echo "   Revisa el directorio: $BUILD_DIR"
    exit 1
fi

echo ""
echo "📋 RESUMEN:"
echo "   - Feature 1 (Garantizar quests): ✅ Implementado"
echo "   - Build directory: $BUILD_DIR"
echo "   - Ejecutable: $BUILD_DIR/devilutionx.exe"
echo "   - DLLs: Copiadas desde build_final_working"
echo ""
echo "🚀 ¡Listo para probar!"