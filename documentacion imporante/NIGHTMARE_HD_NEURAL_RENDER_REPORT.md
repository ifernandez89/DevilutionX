# 🛡️ NIGHTMARE HD & NEURAL RENDER 2.5D — REPORTE TÉCNICO COMPLETO

**Versión:** 1.3  
**Fecha:** Septiembre 8, 2026  
**Motor:** DevilutionX Enhanced + NIGHTMARE Engine Core  
**Plataformas:** WebAssembly (HTML5/WebGPU/WebGL), Windows/Linux Native (OpenGL/GLES)

---

## 🧭 1. FILOSOFÍA DE DISEÑO & VISIÓN

> *"La tecnología agrega información física de superficie, micro-relieve, curvatura y propiedades de material; JAMÁS inventa, distorsiona ni reinventa la identidad y silueta icónica de Diablo 1 (1996)."*

En las versiones anteriores, los filtros de post-procesamiento artificiales oscurecían excesivamente la pantalla y aplicaban halos circulares estáticos que ensuciaban la experiencia. 

El nuevo **NIGHTMARE HD** abandona los trucos de "oscurecimiento genérico" y se basa en una arquitectura **G-Buffer 2.5D + PBR (Physically Based Rendering)** que descompone la escena píxel a píxel y aplica respuestas físicas reales a la luz, aplicable de forma **100% universal a todo el juego** (desde Tristram hasta el nivel 24 del Infierno/Colmena).

---

## 🏗️ 2. ARQUITECTURA DEL SISTEMA IMPLEMENTADO

El sistema opera en tres capas integradas:

```
┌─────────────────────────────────────────────────────────────┐
│                    MOTOR DEVILUTIONX C++                    │
│   • Renderizado de Tiles, Sprites, Proyectiles e Ítems       │
│   • Captura en tiempo real de G-Buffer (RGB, Profundidad,    │
│     Lightmap y Semantic Material ID) en scrollrt.cpp        │
└──────────────────────────────┬──────────────────────────────┘
                               │ (Exportación C-API WASM / Heap)
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                NIGHTMARE HD SHADING PIPELINE                │
│   • Shaders WebGPU (WGSL) / WebGL2 / GLES (C++)             │
│   • Detección de Normales Tangenciales & Biseles            │
│   • Especularidad Metálica según Rugosidad Física (PBR)      │
│   • Micro-sombras en pliegues de tela y hendiduras piedra    │
│   • Adaptación cromática según Bioma (Mazmorras 1 a 24)     │
└──────────────────────────────┬──────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────┐
│             ATMÓSFERA REACTIVA & DIABLO RAIN 2.0            │
│   • Partículas isométricas en 3 capas de profundidad        │
│   • Destellos de relámpago con doble flash                  │
│   • Ascuas incandescentes cerca de fuentes de fuego         │
│   • Cero círculos u óvalos estáticos en pantalla            │
└─────────────────────────────────────────────────────────────┘
```

---

## 💎 3. COMPONENTES EXACTOS EN FUNCIONAMIENTO

### A. G-Buffer en C++ (`Source/nightmare/neural/gbuffer.cpp`)
El motor de renderizado clasifica la escena durante el pase de dibujo en memoria contigua:
1. **Canal RGB (32-bit RGBA):** Cuadro renderizado puro de Diablo 1.
2. **Canal de Profundidad Lógica (Z-Depth):** Profundidad calculada a partir de las coordenadas isométricas del mapa (`dPiece` / pantalla), permitiendo oclusión geométrica y cálculo de sombras.
3. **Canal Lightmap:** Luminancia local de antorchas, hechizos y radio de luz del héroe.
4. **Canal Semantic ID (Materiales):** Identificador de clase semántica:
   - `0 = Void` (Fondo/Límites)
   - `1 = Floor` (Piedra, barro, roca volcánica)
   - `2 = Wall` (Muros de casas, columnas góticas, arcos)
   - `3 = Player` (Héroe: Guerrero, Pícara, Hechicero)
   - `4 = NPC` (Pobladores: Cain, Griswold, Pepin, Adria)
   - `5 = Water / Lava` (Río de Tristram, pozos de sangre, magma)
   - `6 = Interactive` (Cofres, sagrarios, barriles, antorchas)
   - `7 = Monster` (Enemigos y Jefes: Blood Knights, Balrogs, Diablo)
   - `8 = Missile` (Hechizos, rayos, bolas de fuego)
   - `9 = Door` (Puertas de madera y rejas de hierro)

---

### B. Tabla de Propiedades Físicas (PBR Roughness)
Cada superficie reacciona de forma diferente ante el movimiento de las fuentes de luz:

| Material ID | Superficie | Rugosidad (Roughness) | Comportamiento Lumínico |
| :--- | :--- | :--- | :--- |
| **Material 3** | **Acero / Oro / Armaduras** | `0.22` (Baja) | Brillos especulares nítidos en placas, hombreras y filos de espadas. Refleja fuentes de luz sin parecer cromo plástico. |
| **Material 5** | **Cuero / Botas / Corsé** | `0.60` (Media) | Lustre suave satinado en armaduras de cuero y carjac de flechas. |
| **Material 1** | **Piedra / Mampostería** | `0.85` (Alta) | Dispersión difusa con micro-sombras en las hendiduras de mortero. |
| **Material 4** | **Tela / Túnica / Manto** | `0.92` (Muy Alta) | Micro-sombras en pliegues y respuesta aterciopelada en la túnica roja del Guerrero y los mantos del Hechicero. |
| **Material 7** | **Agua / Charcos** | `0.05` (Ultra Baja) | Reducción de rugosidad en suelo ante lluvia para reflejos tenues de antorchas. |
| **Material 8** | **Emissive (Fuego / Ojos)** | `0.00` (Auto-luz) | Emisión pura reservada a llamas, orbes mágicos y ojos demoníacos. |

---

### C. Shaders de Post-Procesamiento (`remaster_render.cpp` & `shaders/tristram_enhancer.wgsl`)
1. **Mapeo de Normales Tangenciales:** Algoritmo Sobel/Luminance que calcula la curvatura física de las hombreras y corazas sin requerir modelos 3D externos.
2. **Unsharp Masking Libre de Halos (5-Tap):** Realce de micro-detalles y bordes sin distorsión de píxeles.
3. **Dynamic Tone Mapping & Gamma S-Curve:** Mayor contraste natural manteniendo total legibilidad de la partida (sin oscurecer ni quemar blancos).
4. **Bloom Selectivo:** Extracción y difusión suave en altas luces (fuego, hechizos, joyas, filos de metal).

---

### D. Diablo Rain 2.0 & Clima Atmosférico (`Packaging/emscripten/index.html`)
1. **Lluvia Isométrica Multi-Capa:**
   - **Capa Lejana:** Micro-gotas veloces y tenues en el fondo.
   - **Capa Media:** Gotas principales con morfología variable y velocidad natural.
   - **Capa Cercana:** Trazos largos con efecto de cámara.
2. **Física de Ráfagas de Viento:** Oscilación senoidal suave con aceleraciones periódicas de viento.
3. **Salpicaduras en Suelo (Pool Zero-GC):** Salpicaduras elípticas en el ángulo isométrico sin impacto de memoria en el navegador.
4. **Doble Destello de Relámpago:** Iluminación atmosférica en dos pulsos sincronizada con la tormenta.
5. **Ascuas Incandescentes:** Partículas que flotan en la noche con atenuación senoidal.
6. **Limpio y Sin Óvalos:** Se eliminaron todas las fuentes de luz estáticas falsas que dibujaban manchas circulares en pantalla.

---

### E. Reconstructor por Lotes & Benchmark Visual (`tools/` & `hd_benchmark.html`)
* **`tools/hd_asset_reconstructor.py`:** Generador procedural en Python puro que crea tuplas PBR de 5 canales (Albedo, Normales, Profundidad, Rugosidad y Material ID) y el archivo `nightmare_hd_manifest.json`.
* **`Packaging/emscripten/hd_benchmark.html`:** Suite interactiva para navegador donde se puede probar:
  - 3 Héroes: Guerrero, Pícara, Hechicero.
  - 6 Familias de Monstruos: Blood Knight, Balrog, Reaper, Esqueleto, Súcubo, Diablo.
  - 5 Biomas: Tristram, Catedral, Catacumbas, Cavernas, Infierno.
  - Antorcha móvil interactiva con el ratón.
  - 6 Canales inspeccionables: HD Composite, Original 1996, Normal Map, Material ID, Rugosidad, Profundidad.

---

## 🎮 4. CONTROLES DEL JUGADOR

* **Botón en Pantalla (Barra Superior):** `⚡ Neural Render: ON` / `⚔️ Original: 1996`
* **Tecla Rápida:** Pulsa **`F2`** en cualquier momento para alternar instantáneamente entre el renderizado remasterizado y el render plano clásico de 1996.
