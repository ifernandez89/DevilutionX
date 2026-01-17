#!/bin/bash
# 🌐 WebAssembly Build Script for Nightmare Browser Edition

set -e  # Exit on any error

echo "🌐 Nightmare Browser Edition - WebAssembly Build"
echo "=================================================="

# Check if Emscripten is available
if ! command -v emcc &> /dev/null; then
    echo "❌ Error: Emscripten not found!"
    echo "Please install Emscripten SDK:"
    echo "  git clone https://github.com/emscripten-core/emsdk.git"
    echo "  cd emsdk"
    echo "  ./emsdk install latest"
    echo "  ./emsdk activate latest"
    echo "  source ./emsdk_env.sh"
    exit 1
fi

echo "✅ Emscripten found: $(emcc --version | head -n1)"

# Generate asset manifest
echo "🔧 Generating asset manifest..."
if [ -f "tools/generate_asset_manifest.py" ]; then
    python3 tools/generate_asset_manifest.py
    echo "✅ Asset manifest generated"
else
    echo "⚠️ Warning: Asset manifest generator not found"
fi

# Create build directory
BUILD_DIR="build_webassembly"
echo "📁 Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure with Emscripten
echo "⚙️ Configuring WebAssembly build..."
cd "$BUILD_DIR"

emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake" \
    -DWEBASSEMBLY_BUILD=ON \
    -DNO_SAVE_SYSTEM=ON \
    -DPERMADEATH_MODE=ON \
    -DBROWSER_EDITION=ON

# Build
echo "🔨 Building WebAssembly..."
emmake make -j$(nproc)

echo ""
echo "🎯 Build complete!"
echo "📁 Output files:"
echo "  - devilutionx.js"
echo "  - devilutionx.wasm"
echo "  - devilutionx.data (if assets embedded)"
echo ""
echo "🌐 To test locally:"
echo "  python3 -m http.server 8000"
echo "  Open: http://localhost:8000"
echo ""
echo "🎮 Nightmare Browser Edition ready!"