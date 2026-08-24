# DevilutionX Web — Subtle Graphics Remaster
## GPU Post-Processing Integration

## Objetivo

Implementar una única capa gráfica de remasterización sutil sobre el port WebAssembly existente de DevilutionX.

Objetivo artístico:

> "Así podría verse Diablo 1 si hubiera recibido un pequeño remaster gráfico oficial manteniendo exactamente sus assets y estética originales."

La mejora debe ser:

- sutil
- claramente visible
- inmediatamente reconocible
- elegante
- técnicamente eficiente
- respetuosa del pixel art y la paleta original
- sin modificar los assets originales
- sin modificar la lógica del juego
- sin sliders
- sin múltiples presets
- sin Graphics Settings Lab

El usuario debe recibir directamente una única presentación REMASTER.

---

# 1. Arquitectura actual confirmada

El repositorio ya dispone de un build WebAssembly específico mediante Emscripten.

La versión web utiliza:

- WebAssembly
- Emscripten
- SDL
- OpenGL ES 2.0 / WebGL
- canvas HTML
- IndexedDB para almacenamiento de datos/configuración

El README del proyecto declara renderizado OpenGL ES 2.0 acelerado por hardware en WebAssembly.

El código de render de DevilutionX también conserva operaciones de composición/blitting sobre buffers de memoria.

Por lo tanto, NO reemplazar el renderer existente.

---

# 2. Decisión de arquitectura

La implementación debe utilizar el pipeline OpenGL/WebGL existente.

NO implementar inicialmente:

- Canvas 2D post-processing
- procesamiento de imagen mediante JavaScript
- captura de screenshots por frame
- readback GPU → CPU
- conversión framebuffer → ImageData → Canvas
- procesamiento de cada píxel mediante JavaScript

Preferir:

```text
DevilutionX rendering
        ↓
existing game framebuffer
        ↓
GPU texture / offscreen framebuffer
        ↓
GPU post-processing shaders
        ↓
default framebuffer
        ↓
existing WebGL canvas
```

---

# 3. Recompilación

## SÍ: realizar una recompilación inicial del build WebAssembly.

No asumir que la capa puede agregarse de manera puramente externa sin modificar el engine.

El objetivo de esta recompilación es crear una integración gráfica correcta que permita:

1. conservar el render original de DevilutionX;
2. interceptar su resultado antes de presentarlo definitivamente;
3. renderizar ese resultado hacia una textura/FBO;
4. ejecutar shaders de post-processing;
5. presentar finalmente el resultado procesado en el framebuffer visible.

Esta debe ser una modificación mínima y localizada.

NO modificar innecesariamente:

- gameplay
- lógica
- input
- audio
- savegames
- MPQ handling
- IndexedDB
- File Manager
- sistema de menús
- lógica de iluminación original

---

# 4. Antes de modificar código

Realizar una auditoría del pipeline actual.

Identificar exactamente:

- dónde se crea el contexto OpenGL/WebGL;
- dónde se crea el framebuffer principal;
- dónde se presenta el frame;
- dónde termina el framebuffer generado por DevilutionX;
- dónde se realiza el escalado;
- dónde se realiza el swap/present;
- qué funciones se ejecutan por frame;
- qué textura o buffer contiene la imagen final;
- si actualmente existe algún framebuffer intermedio reutilizable.

Buscar específicamente en:

```text
Source/
Source/engine/
Source/engine/render/
Source/platform/
Packaging/emscripten/
CMake/platforms/emscripten.cmake
```

No implementar el remaster hasta identificar este punto exacto.

---

# 5. Objetivo de integración

El resultado deseado es:

```text
                     GAME
                      │
                      ▼
              Original renderer
                      │
                      ▼
                Game framebuffer
                      │
                      ▼
               Remaster texture
                      │
                      ▼
             ┌─────────────────┐
             │ GPU POSTPROCESS  │
             └────────┬────────┘
                      │
          ┌───────────┼───────────┐
          ▼           ▼           ▼
      Color grade   Bloom     Dynamic light
          │           │           │
          └───────────┼───────────┘
                      ▼
                   Vignette
                      │
                      ▼
                    Sharpen
                      │
                      ▼
              Visible framebuffer
                      │
                      ▼
                    Canvas
```

Evitar cualquier readback de GPU hacia CPU.

---

# 6. WebGL target

Mantener compatibilidad con el entorno Web actual del proyecto.

No introducir WebGPU todavía.

Utilizar inicialmente:

- OpenGL ES 2.0 compatible
- WebGL-compatible GLSL
- framebuffer objects
- texture targets
- fullscreen quad/triangle
- fragment shaders

El objetivo es que la primera versión funcione en los navegadores modernos donde actualmente funciona el port.

---

# 7. Remaster pipeline

Implementar UN SOLO pipeline.

Orden inicial:

```text
Original frame
    ↓
Color grading
    ↓
Bloom
    ↓
Dynamic 2D lighting
    ↓
Vignette
    ↓
Subtle sharpen
    ↓
Output
```

El orden puede modificarse únicamente si las pruebas visuales o las restricciones del pipeline OpenGL lo justifican.

---

# 8. Color grading

Implementar un shader de color grading conservador.

Aplicar:

- contraste ligeramente mejorado;
- gamma ligeramente corregida;
- saturación muy ligeramente ajustada;
- mejor separación entre sombras, medios tonos y luces.

No cambiar la identidad de la paleta original.

Evitar:

- sobresaturación;
- negros aplastados;
- blancos quemados;
- apariencia HDR;
- estética moderna agresiva.

No crear sliders.

Los valores deben ser constantes internas cuidadosamente elegidas.

---

# 9. Bloom

Implementar bloom muy sutil.

Pipeline conceptual:

```text
Original frame
      ↓
Bright pass
      ↓
Small blur
      ↓
Low intensity bloom
      ↓
Additive blend
```

Aplicar principalmente sobre:

- antorchas;
- fuego;
- hechizos;
- efectos mágicos;
- elementos claramente luminosos.

El bloom debe ser bajo.

Si es posible, ejecutar el bright pass y blur sobre una textura de menor resolución para reducir el coste.

No crear glow exagerado.

---

# 10. Dynamic 2D Lighting

Implementar una primera versión extremadamente simple.

No tocar todavía el sistema de iluminación original del juego.

Agregar una máscara de iluminación GPU sobre el frame.

Concepto:

```text
Game frame
     +
light mask
     ↓
fragment shader
     ↓
lit frame
```

Cada fuente puede representarse internamente como:

```text
x
y
radius
intensity
color
```

Inicialmente utilizar pocas fuentes.

Prioridad:

1. antorchas;
2. fuego;
3. efectos mágicos.

La luz debe utilizar:

- gradiente radial;
- falloff suave;
- color cálido;
- intensidad baja;
- pequeña variación temporal.

No implementar:

- normal mapping;
- height mapping;
- sombras;
- deferred lighting;
- ray tracing;
- compute shaders;
- iluminación físicamente correcta.

La intención es aportar profundidad y atmósfera, no reemplazar el sistema de Diablo.

---

# 11. Vignette

Agregar una vignette extremadamente ligera.

Debe ser perceptualmente discreta.

El jugador no debería pensar:

"hay una vignette".

Debe percibir:

"la imagen tiene mejor atmósfera".

Utilizar un valor fijo bajo.

---

# 12. Sharpen / scaling

Mantener la resolución interna original del juego.

No realizar AI upscaling.

No utilizar ESRGAN.

No utilizar FSR en esta etapa.

No crear múltiples modos de escalado.

Preservar:

- pixel integrity;
- bordes;
- sprites;
- estética original.

Aplicar únicamente un sharpen muy moderado después del pipeline.

El resultado debe verse limpio en pantallas modernas sin eliminar la apariencia pixel-art.

---

# 13. No crear configuración de usuario

NO agregar:

- sliders;
- presets;
- menú de gráficos;
- checkbox por efecto;
- modo Original;
- modo CRT;
- modo Cinematic;
- modo Ultra;
- opciones avanzadas.

El proyecto tendrá una única presentación:

## Remaster

Los parámetros quedan definidos internamente.

---

# 14. Rendimiento

El rendimiento es una prioridad.

Evitar:

```text
GPU
 ↓
CPU
 ↓
JavaScript
 ↓
GPU
```

Preferir:

```text
GPU framebuffer
 ↓
GPU texture
 ↓
GPU shader
 ↓
GPU framebuffer
```

Minimizar:

- framebuffer copies;
- GPU readbacks;
- allocations por frame;
- creación de texturas por frame;
- conversiones de formato;
- operaciones JavaScript por frame.

Color grading, vignette y sharpen deberían poder combinarse en un único fullscreen pass cuando resulte conveniente.

Bloom puede utilizar passes adicionales.

Dynamic lighting debe mantenerse sencillo.

---

# 15. Compatibilidad con el port existente

Preservar completamente:

- File Manager;
- carga de DIABDAT.MPQ;
- carga de SPAWN.MPQ;
- IndexedDB;
- partidas guardadas;
- fullscreen;
- controles;
- audio;
- configuración;
- gameplay.

La única diferencia observable debe ser la nueva presentación gráfica.

---

# 16. Desarrollo incremental

## Fase 0 — Auditoría

No implementar efectos todavía.

Identificar el punto exacto donde:

```text
game render
        ↓
final framebuffer
        ↓
present
```

Confirmar qué objetos OpenGL están disponibles.

Documentar los archivos modificados necesarios.

---

## Fase 1 — GPU fullscreen pass

Crear primero un pipeline mínimo:

```text
Game
 ↓
texture/FBO
 ↓
fullscreen shader
 ↓
screen
```

El shader inicialmente debe producir una imagen visualmente equivalente al render original.

Objetivo:

Demostrar que el frame puede pasar por el nuevo pipeline sin alterar gameplay ni presentación.

---

## Fase 2 — Color grading + vignette + sharpen

Agregar:

- color grading;
- vignette;
- subtle sharpen.

Objetivo:

Obtener la primera mejora visual inmediata con un coste mínimo.

---

## Fase 3 — Bloom

Agregar bloom muy sutil.

Validar especialmente:

- antorchas;
- fuego;
- hechizos;
- UI;
- zonas oscuras.

---

## Fase 4 — Dynamic 2D Lighting

Agregar las primeras fuentes de luz.

Comenzar con:

- Tristram;
- dungeon;
- antorchas.

No intentar cubrir todos los efectos del juego inicialmente.

---

# 17. Criterios de aceptación

La implementación se considera correcta cuando:

- el juego continúa funcionando normalmente;
- los saves continúan funcionando;
- el File Manager continúa funcionando;
- fullscreen continúa funcionando;
- no aparecen artefactos;
- no aparecen flashes;
- no aparece tearing adicional;
- el render mantiene la estética original;
- la mejora visual es inmediatamente reconocible;
- el bloom no domina la escena;
- la iluminación no destruye las zonas oscuras;
- los sprites siguen pareciendo originales;
- el coste de rendimiento es pequeño.

La comparación visual debe hacerse especialmente en:

- Tristram;
- dungeon;
- antorchas;
- zonas oscuras;
- fuego;
- hechizos;
- combate;
- UI.

---

# 18. Regla artística principal

NO hacer que Diablo 1 parezca un juego moderno.

Hacer que parezca:

> Diablo 1 original con una pequeña remasterización gráfica oficial.

Mantener:

- assets originales;
- sprites originales;
- mapas originales;
- paleta característica;
- estética pixel-art;
- composición original.

Agregar únicamente:

- mejor tratamiento de color;
- iluminación atmosférica;
- bloom sutil;
- mejor presentación en pantallas modernas;
- post-processing GPU.

---

# 19. Futuro — fuera de alcance

No implementar ahora:

- normal maps;
- height maps;
- parallax;
- partículas avanzadas;
- WebGPU;
- compute shaders;
- FSR;
- AI upscaling;
- ESRGAN;
- reconstrucción de assets;
- renderer 3D nuevo;
- sombras dinámicas complejas.

La primera versión debe demostrar que un pipeline GPU pequeño y bien diseñado puede mejorar sustancialmente la presentación de DevilutionX sin modificar su identidad.

---

# Resultado final esperado

```text
DevilutionX original
       +
GPU post-processing
       +
sutil iluminación
       +
bloom controlado
       +
color grading
       +
mejor nitidez
       ↓
DEVILUTIONX WEB REMASTER
```

Debe sentirse como:

> "Así podría verse Diablo 1 si hubiera recibido un pequeño remaster gráfico oficial manteniendo exactamente sus assets y estética originales."