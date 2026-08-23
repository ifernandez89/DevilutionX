# DOCUMENTACIÓN COMPLETA DEL PROCESO DE BUILD - DevilutionX

## 🚨 INFORMACIÓN CRÍTICA - NO PERDER

### ESTADO ACTUAL DEL PROYECTO
- **Feature 1 (Garantizar todas las quests)**: ✅ IMPLEMENTADO y TESTEADO
- **Implementación perdida de muerte de Diablo + NPC swap**: ✅ RECUPERADA del commit `ff9521e66`
- **Dependencias SDL2**: ✅ COMPLETAS en `deps/` folder
- **Build funcional**: ✅ Disponible en `build_final_working/` con `devilutionx.exe` funcional

## 📁 ESTRUCTURA DE ARCHIVOS CRÍTICOS

### Dependencias Completas (NUNCA BORRAR)
```
deps/
├── SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/    # Headers y libs SDL2 64-bit
├── SDL2_image-dev/SDL2_image-2.8.8/             # SDL2_image development
├── zlib-src/zlib-1.3.1/                         # Zlib source completo
└── *.zip                                         # Backups de todas las deps
```

### Build Funcional (PRESERVAR)
```
build_final_working/
├── devilutionx.exe                               # ✅ EJECUTABLE FUNCIONAL
├── SDL2.dll, SDL2_image.dll                     # ✅ DLLs necesarias
├── DIABDAT.MPQ                                   # ✅ Assets del juego
└── [todas las demás DLLs]                        # ✅ Dependencias runtime
```

### Scripts de Build (DOCUMENTADOS)
```
Packaging/windows/
├── mingw-prep.sh                                 # Script preparación MinGW
├── mingw-prep64.sh -> mingw-prep.sh              # Enlace simbólico 64-bit
└── dos-prep.sh                                   # Preparación DOS (referencia)
```

## 🔧 MÉTODOS DE COMPILACIÓN (ORDEN DE PREFERENCIA)

### MÉTODO 1: Build Directo con Dependencias Locales ⭐ RECOMENDADO
```bash
# Configurar con dependencias locales
cmake -S. -Bbuild_quick \
  -G "MinGW Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DNONET=ON \
  -DSDL2_DIR="deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/lib/cmake/SDL2" \
  -DCMAKE_PREFIX_PATH="deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32;deps/SDL2_image-dev/SDL2_image-2.8.8/x86_64-w64-mingw32"

# Compilar
cmake --build build_quick -j 4
```

### MÉTODO 2: Usar Build Funcional Existente
```bash
# Copiar ejecutable funcional y aplicar cambios específicos
cp build_final_working/devilutionx.exe ./
# Recompilar solo archivos modificados si es necesario
```

### MÉTODO 3: Script MinGW (Si falla método 1)
```bash
# Ejecutar script de preparación MinGW
./Packaging/windows/mingw-prep64.sh

# Configurar con toolchain
cmake -S. -Bbuild_mingw \
  -DCMAKE_TOOLCHAIN_FILE=CMake/platforms/mingwcc64.toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DNONET=ON

# Compilar
cmake --build build_mingw -j 4 --target package
```

## 🎯 FEATURES IMPLEMENTADAS

### Feature 1: Garantizar Todas las Quests ✅
**Archivo**: `Source/quests.cpp` líneas 242-250
```cpp
// FEATURE 1: Garantizar que todas las quests estén disponibles
// Solo hacer disponibles, NO activar automáticamente
if (!gbIsSpawn) {
    for (auto &quest : Quests) {
        if (quest._qactive == QUEST_NOTAVAIL) {
            quest._qactive = QUEST_INIT;
        }
    }
}
```

### Implementación Perdida: Muerte de Diablo + NPC Swap ✅ RECUPERADA
**Commit**: `ff9521e66e6ba2f22aab9a53d96ea2e792000e11`

**Archivos modificados**:
1. **Source/monster.cpp**:
   - Línea 877: `if (gbIsMultiplayer) gbProcessPlayers = false;`
   - Línea 1516: `if (monster.var1 == 140 && gbIsMultiplayer) PrepDoEnding();`

2. **Source/diablo.cpp**:
   - Línea 3484: `return true;` (en lugar de `return false;`)

3. **assets/txtdata/towners/towners.tsv**:
   - Farnham: (71, 84) → (80, 20)
   - Adria: (80, 20) → (71, 84)

## 🚀 PROCESO DE COMPILACIÓN RÁPIDA

### Pasos Inmediatos (< 5 minutos)
1. **Verificar dependencias**: `ls deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/`
2. **Configurar build**: Usar MÉTODO 1 arriba
3. **Compilar**: `cmake --build build_quick -j 4`
4. **Copiar DLLs**: `cp build_final_working/*.dll build_quick/`
5. **Probar**: `./build_quick/devilutionx.exe`

### Solución de Problemas Comunes
- **SDL2 no encontrado**: Verificar que `SDL2Config.cmake` existe en `deps/SDL2-dev/.../cmake/SDL2/`
- **DLLs faltantes**: Copiar todas las DLLs de `build_final_working/`
- **Errores de linking**: Usar `-DDEVILUTIONX_STATIC_LIBSODIUM=ON`

## 📋 CHECKLIST PRE-COMPILACIÓN

- [ ] ✅ Dependencias en `deps/` están completas
- [ ] ✅ `build_final_working/devilutionx.exe` funciona
- [ ] ✅ Feature 1 implementado en `Source/quests.cpp`
- [ ] ✅ Commit de backup realizado
- [ ] ✅ Scripts MinGW disponibles en `Packaging/windows/`

## 🔄 PROCESO DE BACKUP AUTOMÁTICO

### Antes de cada compilación:
```bash
# Backup completo
git add -A
git commit -m "BACKUP: Pre-compilation state $(date)"

# Backup específico de build funcional
tar -czf "backup_build_$(date +%Y%m%d_%H%M%S).tar.gz" build_final_working/
```

## 📞 COMANDOS DE EMERGENCIA

### Si se corrompe el build:
```bash
# Restaurar desde backup
git reset --hard HEAD~1

# Restaurar dependencias
git checkout HEAD -- deps/

# Restaurar build funcional
git checkout HEAD -- build_final_working/
```

### Verificación rápida:
```bash
# Probar ejecutable existente
./build_final_working/devilutionx.exe

# Verificar Feature 1
grep -n "FEATURE 1" Source/quests.cpp

# Verificar dependencias SDL2
ls deps/SDL2-dev/SDL2-2.32.10/x86_64-w64-mingw32/lib/cmake/SDL2/
```

---

## 🎯 PRÓXIMAS FEATURES A IMPLEMENTAR

1. **Feature 2**: Mejora de iluminación (soft)
2. **Feature 3**: Densidad decorativa leve
3. **Restaurar**: Muerte de Diablo + NPC swap (código recuperado)

**IMPORTANTE**: NUNCA hacer push sin compilar y probar primero. Siempre hacer backup antes de cambios mayores.