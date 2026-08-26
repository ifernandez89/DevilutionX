# Gothic Visual Enhancement — Plan V1
### DevilutionX · Capa de Remasterización Visual

> **Repositorio:** `ifernandez89/DevilutionX`  
> **Fecha del plan:** 2026-08-26  
> **Estado:** Pendiente de implementación

---

## Principio Central

> Diablo original + 10–15% de detalles visuales nuevos que producen un cambio de percepción del 40–50%.

Este plan **no** construye nueva arquitectura.  
Toma la interfaz existente, conserva absolutamente toda su lógica y geometría, y le aplica pequeños tratamientos visuales con alto impacto perceptual.

---

## 🔒 Reglas de Oro — Irrompibles

| # | Regla | Razón |
|---|-------|-------|
| 1 | **No mover nada** — posiciones originales son sagradas | Evita regressions en click-areas |
| 2 | **No cambiar tamaños** — especialmente slots de inventario/belt | Ahí empiezan los bugs de drag & drop |
| 3 | **No tocar el sistema de input/controls** | Hit-testing actual es correcto y funcional |
| 4 | **No tocar el renderer del mundo** — `remaster_render.cpp` existe y funciona muy bien | No reinventar lo que ya está probado |
| 5 | **No construir infraestructura responsive** | Una resolución de referencia, verificar regresiones obvias en una segunda |
| 6 | **Cada cambio pasa el test de frase única:** "El marco de vida ahora tiene hierro envejecido y un highlight." | Si no cabe en una frase, es demasiado grande |

---

## Pipeline de Render Existente — No Modificar

El archivo `Source/engine/render/remaster_render.cpp` ya implementa un pipeline GLES2/WebAssembly con:

- **Unsharp Mask (5-tap):** sharpening sutil sin ringing (`kSharpenWeight = 0.08`)
- **Bloom selectivo:** extrae highlights de antorchas, fuego, hechizos y oro (`threshold = 0.70`, `gain = 0.16`)
- **Color Grading:** gamma `1.04`, contraste S-curve `1.05`, saturación `1.06`
- **Vignette atmosférico:** `smoothstep(0.85, 0.35)` — oscurece bordes con elegancia

> ⚠️ **IMPORTANTE:** El pipeline aplica sobre el frame completo. Para que la UI (textos, HP/MP, tooltips) no sufra
> contaminación de color, los overlays góticos deben dibujarse **después** de que `RemasterProcessAndPresent()`
> retorna `false` y el compositor SDL aplica la capa de UI sobre la escena. Esta separación ya existe en
> DevilutionX — no modificar ese orden.

---

## Tabla de Prioridad Visual

| Elemento | Impacto visual | Riesgo | Prioridad |
|----------|----------------|--------|-----------|
| Marco HP | ⭐⭐⭐⭐⭐ | 🟢 Bajo | 1 |
| Marco Mana | ⭐⭐⭐⭐⭐ | 🟢 Bajo | 1 |
| Marco general HUD | ⭐⭐⭐⭐⭐ | 🟢 Bajo | 1 |
| Belt / Item slots | ⭐⭐⭐⭐ | 🟢 Bajo | 2 |
| Spell slots | ⭐⭐⭐⭐ | 🟢 Bajo | 2 |
| Fondo / Marco Inventario | ⭐⭐⭐⭐⭐ | 🟢 Bajo | 2 |
| Marco Stash | ⭐⭐⭐⭐ | 🟢 Bajo | 3 |
| Lluvia screen-space | ⭐⭐⭐⭐⭐ | 🟢 Bajo | 3 |
| Niebla / Atmósfera | ⭐⭐⭐ | 🟢 Bajo | 4 |
| Weather masks | ⭐⭐ | 🟠 Medio | ❌ NO V1 |
| Lluvia world-anchored | ⭐⭐ | 🟠 Medio | ❌ NO V1 |
| Responsive UI 4K-21:9 | ⭐⭐ | 🔴 Alto | ❌ Fuera de scope |
| Nuevo hit-testing | ⭐ | 🔴 Alto | ❌ Prohibido |

---

## FASE 0 — Congelar el Baseline

**Objetivo:** Punto de regresión irrompible.

### Qué hacer
```bash
git tag v1-baseline-gothic-pre
```
- Documentar screenshot de referencia en la resolución de trabajo actual
- Registrar que drag & drop, clics en slots, HP/MP, spell selection y belt funcionan correctamente

### Qué NO modificar jamás durante el proyecto
- `Source/controls/` — sistema de input
- `Source/inv.cpp` — lógica de inventario (matriz de slots)
- `Source/engine/render/remaster_render.cpp` — pipeline existente
- `Source/engine/render/scrollrt.cpp` — loop de render del mundo
- Cualquier hitbox, área de click, o lógica de drag & drop

---

## FASE 1 — Gothic HUD: HP & Mana

**Archivos principales:**
- `Source/panels/mainpanel.cpp`
- Assets en `assets/ui/gothic/` (marcos PNG como overlay)

### Concepto: Decorative Overlay
No reemplazamos los globos. Los **vestimos**.

```
[globo HP original] + [marco de hierro forjado encima] = Gothic HP
```

### HP (Vida)
- **Mantener:** posición, tamaño, área de llenado, lógica, color rojo
- **Agregar encima como overlay:**
  - Marco de hierro envejecido con remaches
  - Highlight sutil en el borde superior
  - Sombra interna suave
  - Textura de oxidación/piedra en los márgenes

### Mana
- Exactamente el mismo principio
- Marco: cristal/metal oscuro, detalles góticos
- Glow interior extremadamente sutil (no cambia el área lógica)

### Test de aprobación
- [ ] El globo llena y vacía exactamente igual que antes
- [ ] Los clics en HP/MP funcionan igual
- [ ] La lectura de valores es más clara o igual, nunca peor

---

## FASE 2 — Belt / Items / Spells

**Archivos principales:**
- `Source/panels/mainpanel.cpp`
- `Source/panels/spell_icons.cpp`
- `Source/panels/spell_list.cpp`

### Concepto: Slot Visual States
```
[slot vacío]      → borde de hierro desgastado + fondo muy oscuro
[slot con ítem]   → mismo borde, ítem dibujado encima sin cambios
[slot hover]      → highlight sutil en el borde
[slot seleccionado] → glow gótico dorado muy sutil
```

> ⚠️ **IMPORTANTE:** No cambiar el tamaño de los slots. El borde/frame debe
> ser un overlay que dibuja *sobre* el área existente, nunca expandirla o
> contraerla. Nunca tocar las coordenadas de hit-testing.

---

## FASE 3 — Inventario

**Archivos principales:**
- `Source/inv.cpp`

### Concepto
La matriz de slots permanece intacta. Solo cambia lo visual:

| Elemento | Original | Gótico V1 |
|----------|----------|-----------|
| Fondo | Marrón/piedra plana | Piedra oscura con textura y profundidad |
| Marco de la ventana | Marco metálico básico | Hierro forjado con esquinas decorativas |
| Slots individuales | Celda plana | Pequeño borde interior + sombra sutil |

**No tocar:** posición, tamaño de slots, lógica de equipamiento, lógica de drop.

---

## FASE 4 — Stash

### Concepto: Arcón Gótico
Identidad visual diferenciada del inventario.

| Elemento | Visual |
|----------|--------|
| Marco exterior | Madera envejecida + hierro + remaches |
| Fondo | Textura de madera oscura |
| Esquinas | Detalles de herrería gótica |
| Slots | Mismo tratamiento que inventario pero tono más cálido/madera |

El stash *parece* físicamente un arca distinta al inventario. La matriz lógica es intocable.

---

## FASE 5 — Lluvia en Tristram 🌧️

**Archivos nuevos:**
- `Source/engine/render/weather_overlay.cpp`
- `Source/engine/render/weather_overlay.hpp`

### Decisión: Screen-Space (V1)

Se elige **screen-space** sobre world-anchored porque:
- El jugador no distingue la diferencia en V1
- Coste de implementación 10x menor
- Riesgo de regresión cero (layer completamente independiente)
- Ya produce: "Está lloviendo en Tristram"

### Implementación
```
Condición: solo activo cuando el jugador está en Tristram (nivel 0)
Layer: overlay sobre la escena completa, DEBAJO de la capa UI
Tipo: pool estático de 300–500 partículas 2D en screen-space
```

### Parámetros de las gotas
- **Dirección:** levemente diagonal (viento simulado)
- **Velocidad:** moderada, variable ±10%
- **Estela:** 4–12 px de largo
- **Opacidad:** 15–35% (translúcidas)
- **Color:** blanco muy frío con tinte azulado

### Lo que NO necesitamos en V1
- ❌ Física
- ❌ Colisiones con tiles
- ❌ Máscaras de techo
- ❌ Splashes / impactos
- ❌ Paralaje world-anchored
- ❌ Simulación volumétrica

---

## FASE 6 — Atmósfera

Después de que la lluvia funcione, agregar **una sola capa de atmósfera**:

- Niebla muy suave en los bordes de pantalla (overlay con alpha 8–15%)
- Partículas de polvo/humedad flotando (muy lentas, muy opacas)
- Oscurecimiento leve del ambiente cuando llueve

**Una capa, máximo.** El pipeline de `remaster_render.cpp` ya aporta vignette
y color grading — no duplicar esos efectos.

---

## FASE 7 — Verificación de Integración con el Render Existente

Verificación, **no** implementación nueva.

### Checklist de compatibilidad con `remaster_render.cpp`
- [ ] Textos de UI siguen siendo legibles después del pipeline
- [ ] El bloom no contamina los textos blancos del HUD
- [ ] La vignette no oscurece tanto los bordes que los slots del belt queden ilegibles
- [ ] La lluvia overlay se ve correcta después del post-processing
- [ ] Los marcos góticos no se distorsionan con el sharpening del pipeline

> Si algún punto falla levemente, **primero ajustar los parámetros del overlay gótico**,
> no los del shader existente. El shader se modifica solo como último recurso absoluto.

---

## Resumen de Archivos

### Archivos Nuevos

| Archivo | Propósito |
|---------|-----------|
| `Source/engine/render/weather_overlay.cpp` | Sistema de lluvia screen-space |
| `Source/engine/render/weather_overlay.hpp` | Interfaz pública del sistema |
| `assets/ui/gothic/hp_frame.png` | Marco gótico del globo HP |
| `assets/ui/gothic/mp_frame.png` | Marco gótico del globo Mana |
| `assets/ui/gothic/hud_frame.png` | Marco general del HUD |
| `assets/ui/gothic/slot_border.png` | Overlay de borde para slots |
| `assets/ui/gothic/inv_background.png` | Fondo de inventario gótico |
| `assets/ui/gothic/inv_frame.png` | Marco del inventario |
| `assets/ui/gothic/stash_frame.png` | Marco del stash (arcón) |

### Archivos Modificados (Mínimo Quirúrgico)

| Archivo | Modificación |
|---------|-------------|
| `Source/panels/mainpanel.cpp` | Dibujar overlays decorativos encima de HP/MP/HUD |
| `Source/panels/spell_icons.cpp` | Dibujar borde gótico en spell slots |
| `Source/panels/spell_list.cpp` | Estados visuales de selección |
| `Source/inv.cpp` | Dibujar fondo/marco gótico del inventario |
| `Source/engine/render/scrollrt.cpp` | Llamada al overlay de lluvia (solo en Tristram) |

### Archivos Prohibidos — No Tocar

| Archivo | Razón |
|---------|-------|
| `Source/controls/` (todo) | Sistema de input — sagrado |
| `Source/engine/render/remaster_render.cpp` | Pipeline existente — funciona |
| `Source/pack.cpp` | Serialización — sin relación |
| `Source/engine/render/dun_render.cpp` | Renderer del mundo — sagrado |
| Todo `.cpp` de lógica de juego pura | Gameplay — sagrado |

---

## Resultado Esperado

```
Diablo original
     ↓  (sin tocar lógica ni geometría)
Diablo + marcos de hierro gótico en HUD
       + slots con borde metálico sutil
       + inventario con piedra oscura
       + stash como arcón de madera/hierro
       + lluvia en Tristram (screen-space)
       + niebla suave
       + pipeline remaster existente intacto
     =
"Diablo remasterizado perceptualmente — máximo impacto visual, mínimo riesgo de regresión"
```
