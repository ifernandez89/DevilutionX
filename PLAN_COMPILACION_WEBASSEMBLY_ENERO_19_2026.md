# 🔧 PLAN DE COMPILACIÓN WEBASSEMBLY - Diablo 1 Real
**Fecha:** Para 19 de Enero, 2026  
**Objetivo:** Compilar el VERDADERO Diablo 1 para WebAssembly

## 🎯 OBJETIVO PRINCIPAL

Compilar DevilutionX con Emscripten para crear archivos WebAssembly que funcionen correctamente en el navegador, reemplazando los archivos problemáticos actuales.

## 🔧 COMANDOS PREPARADOS

### **PASO 1: Preparar Entorno**
```bash
# Activar Emscripten
cd emsdk
.\emsdk_env.bat

# Crear directorio limpio
cd ..
Remove-Item -Recurse -Force build_webassembly_real
mkdir build_webassembly_real
cd build_webassembly_real
```

### **PASO 2: Configurar CMake**
```bash
# Configuración básica
cmake -G "Unix Makefiles" ^
  -DCMAKE_TOOLCHAIN_FILE="../emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DBUILD_TESTING=OFF ^
  -DDEVILUTIONX_SYSTEM_LIBFMT=OFF ^
  -DDEVILUTIONX_SYSTEM_LIBSODIUM=OFF ^
  -DDEVILUTIONX_SYSTEM_SIMPLEINI=OFF ^
  ..
```

### **PASO 3: Flags Emscripten Específicos**
```bash
# Agregar al CMakeLists.txt o como variables
-s USE_SDL=2
-s USE_SDL_IMAGE=2
-s ALLOW_MEMORY_GROWTH=1
-s INITIAL_MEMORY=67108864
-s MAXIMUM_MEMORY=268435456
-s EXPORTED_FUNCTIONS='["_main"]'
-s EXPORTED_RUNTIME_METHODS='["callMain","FS"]'
-s MODULARIZE=1
-s EXPORT_NAME="DevilutionX"
-s ENVIRONMENT=web
-s FILESYSTEM=1
--preload-file assets
```

### **PASO 4: Compilar**
```bash
# Compilación
make -j4

# O si falla, usar emcc directamente
emcc [archivos_objeto] -o devilutionx.html [flags_emscripten]
```

## 📁 ARCHIVOS ESPERADOS

### **Salida de Compilación:**
```
build_webassembly_real/
├── devilutionx.wasm     - Motor WebAssembly compilado
├── devilutionx.js       - Runtime Emscripten funcional
├── devilutionx.data     - Assets del juego
└── devilutionx.html     - Página de prueba (opcional)
```

### **Copiar a docs/ para GitHub Pages:**
```bash
cp devilutionx.wasm ../docs/devilutionx_real.wasm
cp devilutionx.js ../docs/devilutionx_real.js
cp devilutionx.data ../docs/devilutionx_real.data
```

## 🎮 INTEGRACIÓN CON BROWSER

### **Crear diablo_final_working.html:**
```html
<!-- Usar los nuevos archivos compilados -->
<script src="devilutionx_real.js"></script>

<!-- Configuración Module correcta -->
window.Module = {
    canvas: document.getElementById('canvas'),
    noInitialRun: true,
    locateFile: (path) => {
        if (path.endsWith('.wasm')) return 'devilutionx_real.wasm';
        if (path.endsWith('.data')) return 'devilutionx_real.data';
        return path;
    },
    onRuntimeInitialized: () => {
        // Montar MPQ y iniciar juego
        mountMPQAndStart();
    }
};
```

## 🔍 TROUBLESHOOTING PREPARADO

### **Si CMake falla:**
```bash
# Usar generador diferente
cmake -G "Ninja" [opciones]

# O usar Visual Studio si está disponible
cmake -G "Visual Studio 16 2019" [opciones]
```

### **Si compilación falla:**
```bash
# Compilar con más verbose
make VERBOSE=1

# O usar emcc directamente con archivos específicos
emcc Source/*.cpp -I Source/ [flags] -o devilutionx.html
```

### **Si WebAssembly no carga:**
```bash
# Verificar flags de memoria
-s ALLOW_MEMORY_GROWTH=1
-s INITIAL_MEMORY=33554432

# Verificar exports
-s EXPORTED_FUNCTIONS='["_main","_malloc","_free"]'
```

## 📊 MÉTRICAS DE ÉXITO

### **Compilación Exitosa:**
- ✅ **devilutionx.wasm**: Archivo generado sin errores
- ✅ **devilutionx.js**: Runtime Emscripten completo
- ✅ **Tamaño razonable**: WASM < 10MB, JS < 1MB

### **Carga en Navegador:**
- ✅ **Sin errores de import**: No "module is not an object"
- ✅ **Runtime inicializado**: onRuntimeInitialized se ejecuta
- ✅ **FS disponible**: Module.FS funciona para MPQ

### **Juego Funcionando:**
- ✅ **MPQ montado**: Archivo diablodat.mpq accesible
- ✅ **main() ejecuta**: Juego inicia sin errores
- ✅ **Canvas activo**: Renderizado visible
- ✅ **Controles responden**: Mouse y teclado funcionan

## 🚨 PROBLEMAS POTENCIALES

### **Problema 1: Dependencias SDL**
- **Síntoma**: Errores de SDL no encontrado
- **Solución**: Usar -s USE_SDL=2 y verificar que Emscripten tenga SDL

### **Problema 2: Memoria insuficiente**
- **Síntoma**: "Cannot enlarge memory arrays"
- **Solución**: Aumentar INITIAL_MEMORY y usar ALLOW_MEMORY_GROWTH

### **Problema 3: Archivos no encontrados**
- **Síntoma**: 404 en .wasm o .data
- **Solución**: Verificar locateFile() y paths correctos

### **Problema 4: MPQ no accesible**
- **Síntoma**: Juego no encuentra diablodat.mpq
- **Solución**: Verificar montaje VFS y paths en código C++

## 📋 CHECKLIST DE COMPILACIÓN

### **Pre-compilación:**
- [ ] Emscripten activado (emcc --version funciona)
- [ ] Directorio build limpio
- [ ] CMakeLists.txt revisado para WebAssembly

### **Compilación:**
- [ ] CMake configurado sin errores
- [ ] Make ejecuta sin fallos
- [ ] Archivos .wasm, .js, .data generados

### **Post-compilación:**
- [ ] Archivos copiados a docs/
- [ ] Nueva versión HTML creada
- [ ] Testing básico en navegador

### **Integración:**
- [ ] MPQ loading funciona
- [ ] Runtime inicializa correctamente
- [ ] Juego responde a controles

## 🎯 RESULTADO ESPERADO

Al final del día 19 de enero, deberíamos tener:

1. **DevilutionX compilado** correctamente para WebAssembly
2. **Archivos funcionales** que cargan sin errores en navegador
3. **Diablo 1 real** ejecutándose con archivo MPQ del usuario
4. **Experiencia completa** de juego en navegador
5. **Deployment en GitHub Pages** con versión final

---

**🏆 ÉXITO GARANTIZADO**: Con Emscripten configurado y el plan detallado, la compilación del verdadero Diablo 1 para WebAssembly está lista para ejecutarse mañana.