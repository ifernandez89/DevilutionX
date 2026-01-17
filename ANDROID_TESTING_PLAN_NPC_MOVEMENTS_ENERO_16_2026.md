# 📱 ANDROID TESTING PLAN - NPC Micro-Movements
**Fecha**: Enero 16, 2026  
**Branch**: `npcWalk`  
**Prioridad**: CRÍTICA antes de merge a master

---

## 🎯 RIESGO REAL: ANDROID, NO CÓDIGO

### Por qué Android es diferente:

**Input / Timing / Visibilidad**:
- ✋ El jugador se mueve distinto (tap vs mouse)
- 📱 El viewport es más chico
- 👆 El tap puede quedar cerca de NPCs más tiempo
- ⏱️ El framerate puede variar (30-60fps vs 60fps estable)

**Consecuencia**:
```
Puede exponer comportamientos visuales que en PC no notás.
Nada grave, pero hay que mirarlo.
```

---

## 🧪 QUÉ TESTEAR SÍ O SÍ ANTES DE LLAMAR "FINAL"

### ⚠️ IMPORTANTE: Hacé esto en APK, NO en PC

---

## 1️⃣ TEST DE CERCANÍA CONSTANTE

### Objetivo
Verificar que la cancelación por proximidad funciona correctamente en Android.

### Procedimiento
```
1. Compilar APK debug
2. Instalar en emulador/teléfono
3. Iniciar juego
4. Ir al pueblo
5. Pararse cerca de Farnham (<5 tiles)
6. SIN hablar
7. SIN moverse
8. Esperar 30 segundos
```

### Esperado
```
✅ El NPC se queda quieto SIEMPRE
✅ No intenta moverse
✅ No cancela en loop
✅ Sin logs de "Started walk" (si debug activo)
✅ Solo logs de "Canceled: player nearby"
```

### Si falla
```
❌ NPC intenta moverse → Problema con MIN_PLAYER_DISTANCE
❌ Cancela en loop → Problema con timer
❌ Se mueve igual → Problema con IsPlayerNear()
```

**Acción**: Aumentar `MIN_PLAYER_DISTANCE` de 5 a 7 tiles en Android

---

## 2️⃣ TEST DE TAP + MOVIMIENTO

### Objetivo
Verificar que el tap para hablar cancela el movimiento correctamente.

### Procedimiento
```
1. Pararse lejos de Adria (>5 tiles)
2. Esperar a que Adria inicie WALK_SHORT
3. Observar que empieza a moverse
4. Tap rápido en Adria para hablar
5. Verificar comportamiento
```

### Esperado
```
✅ Movimiento cancelado inmediatamente
✅ NPC estable (no oscila)
✅ Diálogo se abre normal
✅ NPC no se mueve durante diálogo
✅ Al cerrar diálogo, NPC vuelve a IDLE
```

### Si falla
```
❌ NPC sigue moviéndose → Problema con qtextflag check
❌ NPC oscila → Problema con cancelación
❌ Diálogo no abre → Problema con TalkToTowner
❌ NPC se mueve durante diálogo → Problema crítico
```

**Acción**: Verificar que `qtextflag` se chequea ANTES de cualquier movimiento

---

## 3️⃣ TEST DE CARGA DE MAPA

### Objetivo
Verificar que los NPCs se resetean correctamente al entrar/salir del pueblo.

### Procedimiento
```
1. Entrar al pueblo
2. Esperar micro-movimientos (15-20 segundos)
3. Observar posiciones de Farnham, Adria, Gillian
4. Salir del pueblo (ir a dungeon)
5. Volver al pueblo
6. Verificar posiciones
7. Repetir 5 veces
```

### Esperado
```
✅ Todos los NPCs en home al entrar
✅ Sin desfasajes de posición
✅ Sin teleports visibles
✅ microState = IDLE al cargar
✅ returnAttempts = 0 al cargar
✅ Comportamiento consistente en cada carga
```

### Si falla
```
❌ NPC fuera de home → Problema con InitTowners
❌ Desfasaje de posición → Problema con dMonster
❌ Teleport visible → Problema con timing
❌ Estado inconsistente → Problema con inicialización
```

**Acción**: Verificar que `InitNPCMicroMovement()` se llama correctamente

---

## 📱 RECOMENDACIÓN PRÁCTICA (MUY CONCRETA)

### Setup de Testing Android

#### 1. Instalar Android Studio
```bash
# Descargar de: https://developer.android.com/studio
# Instalar Android SDK
# Configurar emulador o conectar teléfono real
```

#### 2. Compilar APK Debug
```bash
cd "C:\c - c++\DevilutionX"
git checkout npcWalk

# Compilar para Android (ajustar según tu setup)
# Ejemplo con CMake + Android NDK:
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 \
      -B build_android

cmake --build build_android
```

#### 3. Probar en:
```
✅ Emulador Android (API 21+)
✅ Si podés, teléfono real (mejor para timing)
```

#### 4. Si esos 3 tests pasan:
```
✅ Podés etiquetar esta build como "release candidate"
✅ Merge a master
✅ Celebrar 🎉
```

---

## 🛡️ PROTECCIONES PREVENTIVAS (OPCIONALES)

### Si los tests fallan en Android, considerar:

#### A. Aumentar MIN_PLAYER_DISTANCE
```cpp
// Source/towners.cpp
#ifdef __ANDROID__
constexpr int MIN_PLAYER_DISTANCE = 7;  // Más conservador en Android
#else
constexpr int MIN_PLAYER_DISTANCE = 5;  // PC
#endif
```

**Razón**: Viewport más chico, tap menos preciso

#### B. Aumentar timers en Android
```cpp
// Source/towners.cpp
#ifdef __ANDROID__
constexpr uint32_t MIN_IDLE_TIME = 10000;  // 10s en Android
constexpr uint32_t MAX_IDLE_TIME = 20000;  // 20s en Android
#else
constexpr uint32_t MIN_IDLE_TIME = 8000;   // 8s en PC
constexpr uint32_t MAX_IDLE_TIME = 15000;  // 15s en PC
#endif
```

**Razón**: Framerate variable, más tiempo para que player se aleje

#### C. Desactivar en Android (último recurso)
```cpp
// Source/towners.cpp
#ifdef __ANDROID__
#define ENABLE_NPC_MICRO_MOVEMENT 0  // Desactivado en Android
#else
#define ENABLE_NPC_MICRO_MOVEMENT 1  // Activo en PC
#endif
```

**Razón**: Si hay problemas críticos, desactivar solo en Android

---

## 🎯 CRITERIOS DE ÉXITO

### Para declarar "RELEASE CANDIDATE":

#### Tests obligatorios:
- [x] Compilación exitosa (PC) ✅
- [ ] Test 1: Cercanía constante (Android) ⏳
- [ ] Test 2: Tap + movimiento (Android) ⏳
- [ ] Test 3: Carga de mapa (Android) ⏳

#### Comportamiento esperado:
- [ ] NPCs se quedan quietos si player cerca ⏳
- [ ] Tap cancela movimiento correctamente ⏳
- [ ] NPCs se resetean al cargar mapa ⏳
- [ ] Sin oscilaciones visibles ⏳
- [ ] Sin teleports visibles ⏳
- [ ] Sin crashes ⏳

#### Si TODO pasa:
```
✅ Sistema aprobado para Android
✅ Merge a master
✅ Tag como "v1.0-npc-movements"
```

#### Si algo falla:
```
1. Identificar problema específico
2. Aplicar protección preventiva (A, B, o C)
3. Re-testear
4. Si sigue fallando → Desactivar en Android (opción C)
```

---

## 📊 MATRIZ DE DECISIÓN

### Escenario 1: TODO pasa en Android
```
Acción: Merge a master
Confianza: 100%
Riesgo: Cero
```

### Escenario 2: Test 1 falla (cercanía)
```
Acción: Aumentar MIN_PLAYER_DISTANCE a 7
Re-testear
Si pasa → Merge
```

### Escenario 3: Test 2 falla (tap)
```
Acción: Verificar qtextflag check
Añadir delay adicional si necesario
Re-testear
Si pasa → Merge
```

### Escenario 4: Test 3 falla (carga)
```
Acción: Verificar InitNPCMicroMovement
Verificar que InitTowners se llama
Re-testear
Si pasa → Merge
```

### Escenario 5: Múltiples tests fallan
```
Acción: Desactivar en Android (#ifdef __ANDROID__)
Merge a master (activo solo en PC)
Investigar más a fondo
```

---

## 🔍 DEBUGGING EN ANDROID

### Si hay problemas, verificar:

#### 1. Logs de debug
```bash
adb logcat | grep "NPC Micro-Movement"
```

**Buscar**:
- "Started walk" cuando no debería
- "Canceled" en loop
- "Teleported" visible

#### 2. Framerate
```bash
adb shell dumpsys gfxinfo <package_name>
```

**Verificar**:
- FPS estable (>30fps)
- Frame drops durante movimiento

#### 3. Touch input
```bash
adb shell getevent
```

**Verificar**:
- Tap se registra correctamente
- No hay doble-tap accidental

---

## 💡 NOTAS FINALES

### NO necesitás:
- ❌ Refactors
- ❌ Más protecciones (a menos que tests fallen)
- ❌ Más estados
- ❌ Más lógica

### SÍ necesitás:
- ✅ Compilar APK debug
- ✅ Testear en emulador/teléfono
- ✅ Verificar los 3 tests
- ✅ Decidir: merge o ajustar

### Filosofía:
> **"Test first, optimize only if needed."**

No añadas protecciones preventivas hasta que los tests demuestren que son necesarias.

---

## ✅ CHECKLIST FINAL

### Antes de merge a master:
- [ ] APK compilado
- [ ] Test 1 ejecutado (cercanía)
- [ ] Test 2 ejecutado (tap)
- [ ] Test 3 ejecutado (carga)
- [ ] Todos los tests pasan
- [ ] Sin crashes
- [ ] Sin comportamientos extraños

### Si TODO pasa:
```bash
git checkout master
git merge npcWalk
git push origin master
git tag v1.0-npc-movements
git push origin v1.0-npc-movements
```

---

**Creado por**: Kiro AI  
**Fecha**: Enero 16, 2026  
**Prioridad**: CRÍTICA  
**Status**: Pendiente testing Android

**Próximo paso**: Compilar APK y ejecutar los 3 tests
