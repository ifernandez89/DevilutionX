# 🔧 VFS REFACTORING EXAMPLE - LIGHTING.CPP
**Fecha**: Enero 17, 2026  
**Objetivo**: Mostrar cómo refactorizar código existente para usar VFS  
**Archivo ejemplo**: `Source/lighting.cpp`  

---

## 📋 ANTES: Código Original

```cpp
// ❌ CÓDIGO ORIGINAL - Dependiente de filesystem nativo
tl::expected<void, std::string> LoadTrns()
{
    RETURN_IF_ERROR(LoadFileInMemWithStatus("plrgfx\\infra.trn", InfravisionTable));
    RETURN_IF_ERROR(LoadFileInMemWithStatus("plrgfx\\stone.trn", StoneTable));
    return LoadFileInMemWithStatus("gendata\\pause.trn", PauseTable);
}
```

## ✅ DESPUÉS: Código VFS-Compatible

```cpp
// ✅ CÓDIGO REFACTORIZADO - Compatible con WebAssembly
#include "abstractions/vfs_wrappers.h"

tl::expected<void, std::string> LoadTrns()
{
    RETURN_IF_ERROR(LoadFileInMemWithStatusVFS("plrgfx/infra.trn", InfravisionTable));
    RETURN_IF_ERROR(LoadFileInMemWithStatusVFS("plrgfx/stone.trn", StoneTable));
    return LoadFileInMemWithStatusVFS("gendata/pause.trn", PauseTable);
}
```

---

## 🔄 CAMBIOS REALIZADOS

### 1️⃣ Include Statement
```cpp
// Agregar al inicio del archivo
#include "abstractions/vfs_wrappers.h"
```

### 2️⃣ Function Calls
```cpp
// Cambiar de:
LoadFileInMemWithStatus("path\\file.ext", array)

// A:
LoadFileInMemWithStatusVFS("path/file.ext", array)
```

### 3️⃣ Path Separators
```cpp
// Cambiar de backslashes (Windows-specific):
"plrgfx\\infra.trn"

// A forward slashes (cross-platform):
"plrgfx/infra.trn"
```

---

## 🎯 BENEFICIOS DE LA REFACTORIZACIÓN

### ✅ Compatibilidad Multiplataforma
- **Desktop**: Usa filesystem nativo (comportamiento idéntico)
- **WebAssembly**: Usa assets embebidos (sin network requests)
- **Testing**: Puede usar filesystem en memoria

### ✅ Cero Cambios en Lógica
- La función `LoadTrns()` mantiene exactamente la misma interfaz
- Los arrays `InfravisionTable`, `StoneTable`, `PauseTable` se llenan igual
- El manejo de errores con `tl::expected` se mantiene idéntico

### ✅ Performance Mejorado
- **Desktop**: Sin cambios de performance
- **WebAssembly**: Carga instantánea (assets embebidos)
- **Caching**: VFS puede implementar caching transparente

### ✅ Debugging Mejorado
- Logs detallados de operaciones VFS
- Estadísticas de assets cargados
- Identificación clara del tipo de VFS en uso

---

## 📁 OTROS ARCHIVOS A REFACTORIZAR

### Archivos de Alta Prioridad:
1. `Source/monster.cpp` - LoadFileInMem para .trn y .dun files
2. `Source/quests.cpp` - LoadFileInMem para dungeon data  
3. `Source/loadsave.cpp` - Save/load operations (más complejo)

### Patrón de Refactorización:
```cpp
// ANTES:
auto data = LoadFileInMem<uint16_t>("levels\\l1data\\banner1.dun");

// DESPUÉS:
auto data = LoadFileInMemVFS<uint16_t>("levels/l1data/banner1.dun");
```

---

## 🚀 PRÓXIMOS PASOS

### 1. **Aplicar refactorización a lighting.cpp**:
```bash
# Backup del archivo original
cp Source/lighting.cpp Source/lighting.cpp.backup

# Aplicar cambios VFS
# (editar manualmente o usar script)
```

### 2. **Compilar y testear**:
```bash
# Compilar versión desktop
cmake --build build_COMPILE_FRESH -j 4

# Verificar que funciona idénticamente
./devilutionx.exe
```

### 3. **Refactorizar siguiente archivo**:
- Elegir `monster.cpp` como siguiente target
- Aplicar mismo patrón de refactorización
- Testear incrementalmente

### 4. **Crear herramienta de refactorización**:
```bash
# Script para automatizar cambios comunes
./tools/refactor_to_vfs.py Source/lighting.cpp
```

---

## 🎯 VALIDACIÓN DE ÉXITO

### ✅ Criterios de Aceptación:
- [ ] Código compila sin errores
- [ ] Juego funciona idénticamente en desktop
- [ ] Logs muestran "NativeVFS" en uso
- [ ] No hay regresiones en gameplay
- [ ] Performance se mantiene igual

### 🧪 Testing:
```cpp
// Verificar que VFS está funcionando
LogInfo("VFS Type: {}", GetVFS().GetImplementationType());
// Debe mostrar: "NativeVFS (Desktop Filesystem)"
```

---

## 💡 LECCIONES APRENDIDAS

### ✅ Lo que Funciona Bien:
- **Abstracción mínima**: Interfaces simples, implementaciones complejas
- **Drop-in replacement**: Cambios mínimos en código existente
- **Template-based**: Mantiene type safety del código original
- **Error handling**: Preserva `tl::expected` pattern

### ⚠️ Consideraciones:
- **Path separators**: Siempre usar forward slashes en VFS
- **Case sensitivity**: WebAssembly assets son case-sensitive
- **Memory management**: VFS maneja ownership de data loaded
- **Error messages**: Incluir path en mensajes de error

---

## 🏆 RESULTADO ESPERADO

Después de esta refactorización:
- ✅ **Desktop build**: Funciona idénticamente
- ✅ **WebAssembly ready**: Preparado para assets embebidos  
- ✅ **Maintainable**: Código más limpio y portable
- ✅ **Testable**: Puede usar mock VFS para testing
- ✅ **Debuggable**: Logs detallados de file operations

**PRÓXIMO ARCHIVO**: `monster.cpp` - Aplicar mismo patrón de refactorización.