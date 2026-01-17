# 🎨 AJUSTES UX - Tiempos de NPCs y Créditos Automáticos
**Fecha**: Enero 16, 2026  
**Branch**: `npcWalk`  
**Status**: ✅ IMPLEMENTADO

---

## 🎯 CAMBIOS IMPLEMENTADOS

### 1. ⏱️ Tiempos de Movimiento de NPCs (4x más largos)
### 2. 🎬 Créditos Automáticos en Menú Principal

---

## 1. ⏱️ TIEMPOS DE MOVIMIENTO DE NPCs

### Problema Identificado
```
Usuario: "si un usuario se queda unos minutos en el town no sea 
tan brusco el 'movimiento' al repetirse en ciclos tan acotados"
```

### Solución: Cuadruplicar Tiempos (x4)

#### Antes (Original)
```cpp
constexpr uint32_t MIN_IDLE_TIME = 8000;   // 8 segundos
constexpr uint32_t MAX_IDLE_TIME = 15000;  // 15 segundos
```

**Frecuencia**: NPCs se mueven cada 8-15 segundos
**Problema**: Demasiado frecuente, se nota el patrón

#### Después (4x más largo)
```cpp
constexpr uint32_t MIN_IDLE_TIME = 32000;  // 32 segundos (was 8s)
constexpr uint32_t MAX_IDLE_TIME = 60000;  // 60 segundos (was 15s)
```

**Frecuencia**: NPCs se mueven cada 32-60 segundos
**Beneficio**: Mucho más sutil, no se nota el patrón

### Comparación de Frecuencia

#### Escenario: Usuario en Town por 5 minutos (300 segundos)

**Antes (8-15s)**:
```
Movimientos por NPC: ~20-37 veces
Total (8 NPCs): ~160-296 movimientos
Resultado: MUY frecuente, patrón obvio
```

**Después (32-60s)**:
```
Movimientos por NPC: ~5-9 veces
Total (8 NPCs): ~40-72 movimientos
Resultado: Sutil, natural, no intrusivo
```

### Beneficios UX

1. **Menos Intrusivo**
   - Usuario no se distrae con movimientos constantes
   - Movimiento se siente más "orgánico"

2. **Más Realista**
   - Personas reales no se mueven cada 10 segundos
   - 30-60 segundos es más natural

3. **Mejor para Gestión de Inventario**
   - Usuario puede organizar items sin distracciones
   - Movimientos no interrumpen concentración

4. **Mantiene Atmósfera**
   - Pueblo sigue sintiéndose vivo
   - Pero no "hiperactivo"

---

## 2. 🎬 CRÉDITOS AUTOMÁTICOS EN MENÚ

### Problema Identificado
```
Usuario: "a la cinematica de 'show credits' la cual retiramos 
el acceso en el menu principal, podriamos habiltarla como opcional?"
```

### Solución: Screensaver de Créditos

#### Comportamiento Implementado

**Menú Principal**:
```
1. Usuario ve menú (Player, Multi, Settings, Exit)
2. Si no hace nada por X segundos (configurable)
3. → Se reproducen créditos automáticamente
4. Al terminar créditos → Vuelve al menú
5. Repite ciclo
```

**Código Modificado**:
```cpp
// En menu.cpp - MAINMENU_ATTRACT_MODE
case MAINMENU_ATTRACT_MODE:
	if (gbIsSpawn && !HaveIntro())
		done = false;
	else if (gbActive) {
		// 🌑 NIGHTMARE: Reproducir créditos automáticamente
		// Mantener intro de estapada al inicio
		// Aquí solo reproducimos créditos como "screensaver"
		UiCreditsDialog();
	}
	break;
```

### Timeout Configurable

El timeout se configura en la llamada a `UiMainMenuDialog()`:

```cpp
// En menu.cpp - mainmenu_loop()
UiMainMenuDialog(gszVersionNumber, &menu, 30);
//                                         ^^ 30 segundos
```

**Valor actual**: 30 segundos de inactividad

### Comportamiento Detallado

#### Secuencia de Inicio (Primera vez)
```
1. Juego inicia
2. → Intro de estapada (SIEMPRE se reproduce)
3. → Menú principal aparece
4. → Timer de 30s comienza
5. → Si no hay input → Créditos automáticos
```

#### Secuencia de Créditos
```
1. Créditos se reproducen (scroll automático)
2. Usuario puede:
   - Presionar ESC → Vuelve al menú
   - Esperar → Créditos terminan → Vuelve al menú
3. → Menú principal aparece
4. → Timer de 30s reinicia
5. → Ciclo se repite
```

### Diferencia con Intro Original

**Intro Original** (Diablo 1):
```
- Se reproduce al inicio
- Muestra cinematica de Diablo
- Después va al menú
```

**Créditos como Screensaver** (Nightmare Edition):
```
- Intro de estapada al inicio (PERMANENTE)
- Créditos después de 30s de inactividad
- Funciona como "screensaver"
- Muestra equipo de desarrollo
```

### Beneficios UX

1. **Intro Permanente**
   - Estapada siempre se reproduce al inicio
   - Mantiene atmósfera de Nightmare Edition

2. **Créditos Visibles**
   - Ya no están "escondidos" en el menú
   - Se muestran automáticamente

3. **Screensaver Funcional**
   - Si usuario deja el juego en menú
   - Créditos se reproducen (como screensaver)
   - Evita burn-in de pantalla

4. **No Intrusivo**
   - 30 segundos es tiempo suficiente
   - Usuario puede cancelar con ESC
   - Vuelve al menú automáticamente

---

## 📊 COMPARACIÓN ANTES/DESPUÉS

### Movimiento de NPCs

| Aspecto | Antes (8-15s) | Después (32-60s) | Mejora |
|---------|---------------|------------------|--------|
| Frecuencia | Muy alta | Baja | ✅ Más sutil |
| Movimientos/5min | ~160-296 | ~40-72 | ✅ 75% menos |
| Intrusividad | Alta | Baja | ✅ No distrae |
| Realismo | Medio | Alto | ✅ Más natural |
| Atmósfera | Hiperactivo | Vivo pero calmado | ✅ Mejor balance |

### Créditos en Menú

| Aspecto | Antes | Después | Mejora |
|---------|-------|---------|--------|
| Acceso | Oculto | Automático | ✅ Visible |
| Intro estapada | Permanente | Permanente | ✅ Mantenido |
| Screensaver | No | Sí | ✅ Funcional |
| UX | Créditos perdidos | Créditos visibles | ✅ Mejor |

---

## 🧪 TESTING

### Test 1: Tiempos de NPCs
```bash
# Iniciar juego
# Ir al pueblo
# Quedarse 5 minutos
# Observar frecuencia de movimientos
```

**Verificar**:
- [ ] NPCs se mueven cada 32-60 segundos
- [ ] Movimiento es sutil, no intrusivo
- [ ] No se nota patrón repetitivo
- [ ] Atmósfera sigue siendo "viva"

### Test 2: Créditos Automáticos
```bash
# Iniciar juego
# Ver intro de estapada (debe reproducirse)
# Llegar al menú principal
# NO tocar nada por 30 segundos
```

**Verificar**:
- [ ] Intro de estapada se reproduce al inicio
- [ ] Después de 30s → Créditos se reproducen
- [ ] Créditos hacen scroll automático
- [ ] ESC vuelve al menú
- [ ] Al terminar créditos → Vuelve al menú
- [ ] Timer reinicia → Créditos se repiten

### Test 3: Interacción con Menú
```bash
# Iniciar juego
# En menú, esperar 25 segundos
# Mover cursor o presionar tecla
```

**Verificar**:
- [ ] Timer se reinicia al interactuar
- [ ] Créditos NO se reproducen si hay input
- [ ] Menú responde normal

---

## 🎯 FILOSOFÍA DE DISEÑO

### Movimiento de NPCs
```
"Sutil, no intrusivo"

- Pueblo debe sentirse vivo
- Pero no hiperactivo
- Movimiento debe ser sorpresa agradable
- No distracción constante
```

### Créditos Automáticos
```
"Screensaver funcional"

- Intro de estapada SIEMPRE al inicio
- Créditos como "bonus" después de timeout
- Muestra trabajo del equipo
- Evita menú estático por horas
```

---

## 📝 VALORES CONFIGURABLES

### Tiempos de NPCs (Source/towners.cpp)
```cpp
constexpr uint32_t MIN_IDLE_TIME = 32000;  // Mínimo entre movimientos
constexpr uint32_t MAX_IDLE_TIME = 60000;  // Máximo entre movimientos
constexpr uint32_t WALK_DURATION = 1500;   // Duración del movimiento
```

**Ajustar si**:
- Muy lento → Reducir MIN/MAX_IDLE_TIME
- Muy rápido → Aumentar MIN/MAX_IDLE_TIME

### Timeout de Créditos (Source/menu.cpp)
```cpp
UiMainMenuDialog(gszVersionNumber, &menu, 30);
//                                         ^^ Segundos de timeout
```

**Ajustar si**:
- Muy rápido → Aumentar valor (ej: 60)
- Muy lento → Reducir valor (ej: 20)

---

## 🔧 ARCHIVOS MODIFICADOS

### Source/towners.cpp
```
Línea 1207-1209: Tiempos cuadruplicados
- MIN_IDLE_TIME: 8000 → 32000 (4x)
- MAX_IDLE_TIME: 15000 → 60000 (4x)
```

### Source/menu.cpp
```
Línea 177-186: Créditos automáticos
- MAINMENU_ATTRACT_MODE ahora llama UiCreditsDialog()
- Mantiene intro de estapada al inicio
```

---

## ✅ CHECKLIST

### Código
- [x] Tiempos de NPCs cuadruplicados
- [x] Créditos automáticos implementados
- [x] Intro de estapada mantenida
- [x] Código compila sin errores
- [x] Ejecutable actualizado

### Testing (Pendiente)
- [ ] Tiempos de NPCs verificados (32-60s)
- [ ] Créditos se reproducen después de 30s
- [ ] Intro de estapada funciona
- [ ] ESC cancela créditos
- [ ] Timer reinicia al interactuar

### Documentación
- [x] Cambios documentados
- [x] Valores configurables explicados
- [x] Filosofía de diseño explicada

---

## 🎉 RESULTADO FINAL

### Movimiento de NPCs
```
Antes: Movimiento cada 8-15s (muy frecuente)
Después: Movimiento cada 32-60s (sutil)

Resultado: Pueblo vivo pero no hiperactivo
```

### Créditos Automáticos
```
Antes: Créditos ocultos, sin acceso
Después: Créditos como screensaver (30s timeout)

Resultado: Créditos visibles + Screensaver funcional
```

---

**Implementado por**: Kiro AI  
**Fecha**: Enero 16, 2026  
**Branch**: `npcWalk`  
**Status**: ✅ AMBOS CAMBIOS IMPLEMENTADOS

**¡UX mejorada significativamente!** 🎨✨
