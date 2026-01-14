# 🧪 GUÍA: TEST REALISTA DE APOCALYPSE
## Simulación de Jugador Moderno con Fast-Clicking

---

## 📋 DESCRIPCIÓN

Este test simula comportamiento real de jugadores sin necesidad de jugar:

### ESCENARIOS TESTEADOS:

1. **Fast-Clicking SinglePlayer** (15 clicks/segundo)
   - Simula jugador moderno con clicking rápido
   - 3 segundos de gameplay a 60 FPS
   - Valida cooldown y límites

2. **Extreme Spam SinglePlayer** (60 clicks/segundo)
   - Caso extremo: spam cada frame
   - 2 segundos de abuse extremo
   - Valida protección contra abuse

3. **Multiplayer 2 Players**
   - 2 jugadores casteando simultáneamente
   - Valida límites por jugador
   - Valida límite global

4. **Multiplayer 4 Players Spam**
   - 4 jugadores spammeando
   - Valida límite global de 2 Apocalypse
   - Caso más estresante

5. **Worst Case - High Monster Density**
   - Área llena de monstruos (50 booms por spell)
   - Valida safety net de 50 booms
   - Caso extremo de densidad

---

## 🚀 OPCIÓN 1: COMPILAR STANDALONE (MÁS SIMPLE)

### Requisitos:
- Compilador C++ con soporte C++17
- MinGW, MSVC, o GCC

### Pasos:

#### Windows con MinGW:
```bash
g++ -std=c++17 -O2 test_apocalypse_realistic.cpp -o test_apocalypse_realistic.exe
test_apocalypse_realistic.exe
```

#### Windows con MSVC:
```bash
cl /EHsc /std:c++17 /O2 test_apocalypse_realistic.cpp
test_apocalypse_realistic.exe
```

#### Linux/Mac:
```bash
g++ -std=c++17 -O2 test_apocalypse_realistic.cpp -o test_apocalypse_realistic
./test_apocalypse_realistic
```

---

## 🚀 OPCIÓN 2: USAR SCRIPT AUTOMÁTICO

### Windows:
```bash
RUN_APOCALYPSE_TEST.bat
```

Este script:
1. Intenta compilar con g++
2. Si falla, intenta con cl (MSVC)
3. Ejecuta el test automáticamente
4. Muestra resultados

---

## 🚀 OPCIÓN 3: INTEGRAR CON DEVILUTIONX BUILD

### Agregar al CMakeLists.txt principal:

```cmake
# Test de Apocalypse (opcional)
option(BUILD_APOCALYPSE_TEST "Build Apocalypse realistic test" OFF)

if(BUILD_APOCALYPSE_TEST)
    add_executable(test_apocalypse_realistic test_apocalypse_realistic.cpp)
endif()
```

### Compilar:
```bash
cmake -B build_test -DBUILD_APOCALYPSE_TEST=ON
cmake --build build_test
build_test/test_apocalypse_realistic.exe
```

---

## 📊 INTERPRETACIÓN DE RESULTADOS

### EJEMPLO DE SALIDA EXITOSA:

```
========================================
TEST: Fast-Clicking SinglePlayer
========================================
Total cast attempts:      45
Successful casts:         12
Blocked by cooldown:      20
Blocked by player limit:  13
Blocked by global limit:  0
Max missiles reached:     0
Total booms created:      240
Max simultaneous missiles: 25
Result:                   ✅ PASSED
========================================
```

### MÉTRICAS CLAVE:

#### ✅ PASSED (Exitoso):
- `Max simultaneous missiles` < 200
- `Result: ✅ PASSED`
- No crashes durante el test

#### ❌ FAILED (Fallido):
- `Max simultaneous missiles` > 200
- `Result: ❌ CRASHED`
- Indica que la protección falló

### VALORES ESPERADOS:

| Métrica | Valor Esperado | Significado |
|---------|----------------|-------------|
| Successful casts | 10-20 por test | Casts que pasaron protección |
| Blocked by cooldown | 30-50% | Cooldown funcionando |
| Blocked by player limit | 20-40% | Límite por jugador funcionando |
| Blocked by global limit | 0-10% | Límite global funcionando |
| Max simultaneous missiles | 20-60 | Pico de missiles activos |
| Total booms created | 200-600 | Booms totales en el test |

### 🎯 CRITERIO DE ÉXITO:

```
✅ Max simultaneous missiles < 100  → EXCELENTE
✅ Max simultaneous missiles < 150  → BUENO
⚠️ Max simultaneous missiles < 200  → ACEPTABLE
❌ Max simultaneous missiles >= 200 → PELIGROSO
```

---

## 🔍 QUÉ VALIDA CADA TEST

### Test 1: Fast-Clicking SinglePlayer
**VALIDA**:
- ✅ Cooldown de 100ms funciona
- ✅ Límite de 1 Apocalypse por jugador funciona
- ✅ Clicking rápido no causa crash
- ✅ Feel responsivo mantenido

**ESPERADO**:
- ~12 casts exitosos en 3 segundos
- ~20-30 bloqueados por cooldown
- ~10-15 bloqueados por límite de jugador
- Max 30-40 missiles simultáneos

### Test 2: Extreme Spam SinglePlayer
**VALIDA**:
- ✅ Protección contra abuse extremo
- ✅ Sistema no colapsa con spam
- ✅ Límites se mantienen bajo presión

**ESPERADO**:
- ~8-10 casts exitosos en 2 segundos
- ~60-80 bloqueados por cooldown
- ~30-40 bloqueados por límite de jugador
- Max 40-50 missiles simultáneos

### Test 3: Multiplayer 2 Players
**VALIDA**:
- ✅ Cada jugador puede castear independientemente
- ✅ No se bloquean entre sí
- ✅ Límite global de 2 funciona

**ESPERADO**:
- ~30-36 casts exitosos (ambos jugadores)
- Ambos jugadores tienen casts exitosos
- Max 50-60 missiles simultáneos

### Test 4: Multiplayer 4 Players Spam
**VALIDA**:
- ✅ Límite global de 2 Apocalypse funciona
- ✅ Sistema estable con 4 jugadores
- ✅ No colapsa bajo carga multiplayer

**ESPERADO**:
- ~20-30 casts exitosos (distribuidos)
- ~100+ bloqueados por límite global
- Max 60-80 missiles simultáneos

### Test 5: Worst Case Monster Density
**VALIDA**:
- ✅ Safety net de 50 booms por spell funciona
- ✅ Densidad extrema de monstruos no causa crash
- ✅ Límite de booms se respeta

**ESPERADO**:
- ~15-20 casts exitosos
- Cada cast crea exactamente 50 booms (límite)
- Max 70-90 missiles simultáneos

---

## 🐛 TROUBLESHOOTING

### Error: "No se reconoce g++"
**SOLUCIÓN**: Instalar MinGW o usar MSVC
```bash
# Descargar MinGW: https://sourceforge.net/projects/mingw/
# O usar MSVC que viene con Visual Studio
```

### Error: "cannot find -lstdc++"
**SOLUCIÓN**: Instalar librerías de desarrollo
```bash
# Linux:
sudo apt-get install build-essential

# Mac:
xcode-select --install
```

### Error: Compilación exitosa pero no ejecuta
**SOLUCIÓN**: Verificar permisos
```bash
# Linux/Mac:
chmod +x test_apocalypse_realistic
./test_apocalypse_realistic
```

---

## 📈 ANÁLISIS DE RESULTADOS

### SI TODOS LOS TESTS PASAN (✅):

**CONCLUSIÓN**: 
- ✅ Protección de Apocalypse es sólida
- ✅ Fast-clicking no causa crashes
- ✅ Multiplayer funcional
- ✅ Casos extremos manejados correctamente
- ✅ **LISTO PARA JUGAR**

### SI ALGÚN TEST FALLA (❌):

**ACCIONES**:
1. Revisar qué test falló específicamente
2. Verificar `Max simultaneous missiles`
3. Si > 200: Ajustar límites en `CanSafelyCastApocalypse()`
4. Si crashed: Revisar `ProcessApocalypse()` por bugs
5. Re-ejecutar test después de ajustes

---

## 🎯 VALORES DE REFERENCIA

### CONFIGURACIÓN ACTUAL:

```cpp
Cooldown por jugador:     100ms
Límite por jugador:       1 Apocalypse activo
Límite global:            2 Apocalypse activos
Booms por Apocalypse:     50 máximo
Velocidad procesamiento:  Instantáneo (1 frame)
```

### MATEMÁTICA DE SEGURIDAD:

```
SINGLEPLAYER:
- 1 Apocalypse × 50 booms = 50 missiles
- Límite de crash: ~200 missiles
- Safety margin: 400% ✅

MULTIPLAYER (2 jugadores):
- 2 Apocalypse × 50 booms = 100 missiles
- Límite de crash: ~200 missiles
- Safety margin: 200% ✅

WORST CASE (4 jugadores intentando):
- 2 Apocalypse activos (límite global)
- 2 × 50 booms = 100 missiles
- Otros 2 jugadores bloqueados
- Safety margin: 200% ✅
```

---

## 🏆 CRITERIOS DE APROBACIÓN

### PARA CONSIDERAR EL FIX EXITOSO:

- [ ] Test 1 (Fast-Clicking) PASSED
- [ ] Test 2 (Extreme Spam) PASSED
- [ ] Test 3 (Multiplayer 2P) PASSED
- [ ] Test 4 (Multiplayer 4P) PASSED
- [ ] Test 5 (Worst Case) PASSED
- [ ] Max missiles < 150 en todos los tests
- [ ] 0 crashes en todos los tests
- [ ] Success rate = 100%

### SI TODOS LOS CRITERIOS SE CUMPLEN:

**🎉 APOCALYPSE FIX APROBADO - LISTO PARA PRODUCCIÓN 🎉**

---

## 📝 NOTAS ADICIONALES

### LIMITACIONES DEL TEST:

1. **Simulación simplificada**: No incluye toda la complejidad del engine real
2. **Timing aproximado**: Los sleeps pueden no ser exactos
3. **Sin rendering**: No simula carga de renderizado
4. **Sin pathfinding**: No simula AI de monstruos

### PERO ES SUFICIENTE PARA:

- ✅ Validar lógica de protección
- ✅ Detectar problemas de acumulación
- ✅ Verificar límites y cooldowns
- ✅ Simular casos extremos
- ✅ Dar confianza antes de jugar

---

## 🚀 PRÓXIMOS PASOS

### DESPUÉS DE EJECUTAR EL TEST:

1. **Si todos pasan**: 
   - ✅ Compilar DevilutionX completo
   - ✅ Jugar y validar feel
   - ✅ Confirmar 0% crash rate en juego real

2. **Si alguno falla**:
   - ⚠️ Revisar implementación
   - ⚠️ Ajustar límites si es necesario
   - ⚠️ Re-ejecutar test
   - ⚠️ NO jugar hasta que todos pasen

---

**CREADO POR**: Kiro AI  
**FECHA**: Enero 14, 2026  
**PROPÓSITO**: Validar fix de Apocalypse sin necesidad de jugar  
**ESTADO**: ✅ LISTO PARA USAR
