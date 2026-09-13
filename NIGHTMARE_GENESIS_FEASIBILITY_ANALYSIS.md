# NIGHTMARE — Genesis/Mega Drive
## Análisis de Viabilidad Técnica y Creativa

**Fecha:** 12 de Septiembre de 2026  
**Proyecto:** NIGHTMARE — A 16-Bit Dark Fantasy Action RPG  
**Plataforma Objetivo:** Sega Genesis / Mega Drive  
**Framework de Desarrollo:** SGDK (SEGA Genesis Development Kit)

---

## RESUMEN EJECUTIVO

**¿Es posible? Sí, absolutamente.**

Este documento analiza la viabilidad técnica, legal, creativa y de desarrollo de crear **NIGHTMARE**, un ARPG isométrico original para Sega Genesis/Mega Drive inspirado en el género de los action RPG de los años 90, pero diseñado desde cero como una obra original.

**Conclusión preliminar:**
- ✅ **Técnicamente viable** con SGDK y las capacidades del hardware de 16-bit
- ✅ **Legalmente seguro** si se desarrolla como obra original sin assets de terceros
- ✅ **Creativamente superior** a un port directo
- ⚠️ **Desafiante** pero alcanzable con metodología iterativa

---

## PARTE I: ANÁLISIS TÉCNICO DEL HARDWARE

### 1.1 Especificaciones de Sega Genesis/Mega Drive

#### CPU y Memoria
```
CPU Principal:    Motorola 68000 @ 7.6 MHz (NTSC)
CPU de Audio:     Zilog Z80 @ 3.58 MHz
RAM Principal:    64 KB
VRAM:             64 KB
ROM:              Hasta 4 MB sin mapper (expandible con bank switching)
```

#### Sistema de Video (VDP - Video Display Processor)
```
Resolución:       320×224 (NTSC) / 320×240 (PAL)
Colores totales:  512 (9-bit RGB)
Paletas:          4 paletas × 16 colores = 64 colores simultáneos
Sprites:          Hasta 80 sprites en pantalla
                  Máximo 20 sprites por línea horizontal
Tiles:            8×8 píxeles
Planes:           2 planos de scroll (A y B) + 1 plano de ventana
Prioridad:        Sistema de capas con prioridad por tile
```

#### Sistema de Audio
```
YM2612 (FM):      6 canales FM + 1 canal DAC
PSG (SN76489):    3 canales de tono + 1 canal de ruido
Calidad DAC:      8-bit @ hasta 32 kHz
```

### 1.2 Restricciones Críticas

#### Presupuesto de VRAM (64 KB total)
```
VRAM Layout:
├── Plane A (Tilemap)          ~4 KB  (64×32 tiles)
├── Plane B (Tilemap)          ~4 KB  (64×32 tiles)
├── Window Plane               ~2 KB  (opcional)
├── Sprite Attribute Table     ~1 KB  (80 sprites)
├── Tile Data (escenario)     ~32 KB  (hasta 1024 tiles)
├── Tile Data (sprites)       ~16 KB  
└── Paletas                   ~128 bytes
```

**Implicación para NIGHTMARE:**
- Debemos ser extremadamente eficientes con tiles únicos
- Reutilización agresiva de tiles mediante flip horizontal/vertical
- Sistema de paginación para cambiar tilesets entre áreas

#### Límites de Sprites
```
Por frame:     80 sprites máximo
Por línea:     20 sprites máximo (320 píxeles de ancho)
Tamaño sprite: 1×1 hasta 4×4 tiles (8×8 a 32×32 píxeles)
```

**Implicación para NIGHTMARE:**
- Jugador: ~4-8 sprites (según animación)
- Enemigos: ~2-4 sprites cada uno → máximo 10-15 enemigos visibles
- Proyectiles/efectos: sprites pequeños (8×8 o 16×16)
- UI en overlay: tiles, no sprites

#### CPU Budget @ 60 FPS (16.67ms por frame)
```
Frame Budget Distribution:
├── VDP/DMA Operations        ~3-4 ms
├── Game Logic                ~4-5 ms
├── Collision Detection       ~2-3 ms
├── AI Update                 ~2-3 ms
├── Audio Update              ~1-2 ms
└── Buffer/Overhead           ~2-3 ms
```

---

## PARTE II: VIABILIDAD TÉCNICA POR SISTEMA

### 2.1 Motor Isométrico

#### Desafío: Representación de Profundidad

**Enfoque Propuesto: Pseudo-isométrica con Tiles Pre-renderizados**

```
Geometría del Tile:
   
    NW ─┬─ NE
        │       Tile isométrico: 32×16 píxeles
    SW ─┴─ SE   (4×2 tiles de 8×8)

Layout del Mundo:
    
    ┌────┐
    │ 00 │
 ┌──┼────┼──┐
 │01│ 02 │03│
 └──┼────┼──┘
    │ 04 │
    └────┘
```

**Implementación en SGDK:**
```c
// Estructura de tile de dungeon
typedef struct {
    uint16_t tileIndex;        // Índice base en VRAM
    uint8_t  heightLevel;      // Nivel de altura (0-7)
    uint8_t  flags;            // Walkable, Wall, Door, etc.
} DungeonTile;

// Mapa del dungeon (32×32 tiles)
#define DUNGEON_WIDTH  32
#define DUNGEON_HEIGHT 32
DungeonTile dungeonMap[DUNGEON_HEIGHT][DUNGEON_WIDTH];
```

**Precedentes en Genesis:**
- **Landstalker** (Climax Entertainment, 1992) — isométrico avanzado
- **Beyond Oasis** (Ancient, 1994) — perspectiva 3/4 con profundidad
- **Shadowrun** (Beam Software, 1994) — isométrico RPG

**Veredicto: ✅ VIABLE** — Existen múltiples ejemplos exitosos.

---

### 2.2 Sistema de Sprites y Animación

#### Jugador (Guerrero/Mago/Arquero)

**Especificación de Sprite:**
```
Tamaño:       24×32 píxeles (3×4 tiles = 12 tiles por frame)
Animaciones:  8 direcciones × 4 frames promedio

Animaciones Requeridas:
├── Idle (8 dir)      = 8 frames   → 96 tiles
├── Walk (8 dir)      = 32 frames  → 384 tiles
├── Attack (8 dir)    = 24 frames  → 288 tiles
├── Cast (8 dir)      = 16 frames  → 192 tiles
├── Hit (8 dir)       = 8 frames   → 96 tiles
└── Death             = 6 frames   → 72 tiles
                      TOTAL:        1,128 tiles
```

**Optimización mediante Flip:**
```
N, NE, E, SE pueden generar S, SW, W, NW mediante flip
→ Reducción del 50%: 564 tiles
```

**Storage:**
- 564 tiles × 32 bytes/tile = **18 KB en ROM**
- Carga dinámica a VRAM según animación activa: **~6-8 KB en VRAM**

#### Enemigos

**Budget por Enemigo:**
```
Tipo A (Zombi):    16×24 píxeles × 8 dir × 3 anim = ~144 tiles → 4.5 KB
Tipo B (Esqueleto): 16×24 píxeles × 8 dir × 3 anim = ~144 tiles → 4.5 KB
Tipo C (Mini-boss): 32×32 píxeles × 8 dir × 4 anim = ~512 tiles → 16 KB
```

**Veredicto: ✅ VIABLE** — Con paginación de tiles y optimización agresiva.

---

### 2.3 Sistema de Combate

#### Hit Detection

**Método 1: Tile-Based Collision**
```c
bool CheckMeleeHit(Entity* attacker, Entity* target) {
    // Bounding box simple
    int16_t dx = abs(attacker->x - target->x);
    int16_t dy = abs(attacker->y - target->y);
    
    return (dx < attacker->attackRange && dy < attacker->attackRange);
}
```

**Método 2: Collision Masks (más preciso)**
```c
// Máscara de colisión por frame de ataque
typedef struct {
    int8_t offsetX, offsetY;
    uint8_t width, height;
} HitBox;

const HitBox swordSwingMask[4] = {
    {16, 0, 24, 16},  // Frame 1
    {20, -8, 28, 20}, // Frame 2
    {16, -12, 32, 24},// Frame 3
    {8, -8, 28, 20}   // Frame 4
};
```

**Precedente:** 
- **Streets of Rage 2** — sistema de hitboxes sofisticado
- **Golden Axe** — detección por proximidad + frame de ataque

**Veredicto: ✅ VIABLE** — Múltiples enfoques probados.

---

### 2.4 Sistema de Proyectiles y Hechizos

#### Firebolt (Hechizo Básico)

```
Sprite:     8×8 píxeles (1 tile)
Animación:  4 frames de rotación
Path:       Trayectoria lineal desde caster hasta objetivo
Speed:      4-6 píxeles por frame
Pool:       Máximo 8 proyectiles simultáneos
```

**Implementación:**
```c
#define MAX_PROJECTILES 8

typedef struct {
    int16_t x, y;           // Posición fija 16-bit
    int16_t vx, vy;         // Velocidad (fixed point 8.8)
    uint8_t lifetime;       // Frames restantes
    uint8_t damage;
    uint8_t animFrame;
    bool    active;
} Projectile;

Projectile projectiles[MAX_PROJECTILES];

void UpdateProjectiles() {
    for (uint8_t i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) continue;
        
        projectiles[i].x += projectiles[i].vx;
        projectiles[i].y += projectiles[i].vy;
        projectiles[i].lifetime--;
        
        if (projectiles[i].lifetime == 0 || CheckCollision(&projectiles[i])) {
            projectiles[i].active = false;
        }
    }
}
```

#### Lightning (Hechizo AOE)

**Enfoque Visual:**
```
No renderizar rayo completo frame-by-frame.

En su lugar:
1. Flash blanco de pantalla (1 frame)
2. Segmentos de "bolt" tiles colocados en línea
3. Screen shake (2-3 frames)
4. Damage aplicado instantáneamente a enemigos en rango
```

**CPU Cost:** ~0.5ms (muy eficiente)

**Veredicto: ✅ VIABLE** — Los efectos visuales pueden ser simples pero impactantes.

---

### 2.5 Generación de Dungeons

#### Enfoque Híbrido: Pre-diseñado + Procedural

**Opción A: Dungeons Pre-diseñados (Recomendado para MVP)**
```
Ventajas:
+ Control total sobre diseño
+ Performance predecible
+ Más sencillo de debuggear
+ Permite storytelling preciso

Desventajas:
- Requiere herramientas de edición
- Menos rejugabilidad
```

**Opción B: Generación Procedural**
```
Ventajas:
+ Rejugabilidad infinita
+ Menor tamaño de ROM

Desventajas:
- CPU intensivo (solución: pre-generar al inicio del nivel)
- Más difícil de balancear
```

**Recomendación:** 
- **Fase 1 (MVP):** Niveles pre-diseñados
- **Fase 2:** Variaciones procedurales sobre layouts base

**Herramientas:**
- **Tiled Map Editor** → exportar a formato binario
- Script Python para conversión a arrays C

**Veredicto: ✅ VIABLE** — Enfoque pre-diseñado es estándar en la época.

---

### 2.6 Interfaz de Usuario

#### HUD Layout (Overlay Permanente)

```
┌─────────────────────────────────────┐
│ ❤️×5  🔮×3        GOLD: 245      Lv3│  ← Plane Window (siempre visible)
├─────────────────────────────────────┤
│                                     │
│                                     │
│         ÁREA DE JUEGO               │
│          (Dungeon)                  │
│                                     │
│                                     │
│                                     │
├─────────────────────────────────────┤
│  [⚔️]  [🛡️]  [💍]  [📜]            │  ← Barra de acceso rápido
└─────────────────────────────────────┘
```

**Implementación:**
- **Window Plane:** HUD superior (32 tiles de altura)
- **Plane A:** Dungeon background
- **Sprites:** Jugador, enemigos, items, efectos
- **Plane B:** Efectos de parallax / oscuridad (opcional)

#### Menú de Inventario (Pantalla completa)

```
┌─────────────────────────────────────┐
│  INVENTARIO                         │
│                                     │
│  ┌───┬───┬───┬───┐  ┌──────────┐  │
│  │ ⚔️ │🛡️ │💍│📿│  │ ESPADA   │  │
│  ├───┼───┼───┼───┤  │ LARGA    │  │
│  │📜│💊│💊│  │  │          │  │
│  ├───┼───┼───┼───┤  │ DMG: 12  │  │
│  │  │  │  │  │  │ STR: 10  │  │
│  └───┴───┴───┴───┘  └──────────┘  │
│                                     │
│  ORO: 245      PESO: 45/80          │
└─────────────────────────────────────┘
```

**Navegación:** D-Pad + botones (sin cursor de mouse necesario)

**Veredicto: ✅ VIABLE** — Diseño estándar para ARPG de consola.

---

### 2.7 Sistema de Audio

#### Música (YM2612 FM Synthesis)

**Composición específica para YM2612:**

```
Canales disponibles:
├── Canal 1: Melodía principal
├── Canal 2: Armonía / contramelodía
├── Canal 3: Bajo (modo especial con frecuencias independientes)
├── Canal 4: Pad atmosférico
├── Canal 5: Percusión (FM drums)
└── Canal 6: DAC sample (bass drum / snare)

PSG adicional:
├── Canal 1-3: Efectos ambientales
└── Canal 4 (ruido): Hi-hat, efectos
```

**Temas musicales:**
1. **Ciudad (Estado Normal)** — Melancólico, guitarra FM
2. **Ciudad (Corrupción)** — Disonancia creciente
3. **Cripta** — Órgano FM + percusión metálica
4. **Catacumbas** — Drones graves + ecos
5. **Minas** — Percusión industrial
6. **Santuario** — Coral oscuro FM
7. **Boss Battle** — Tempo rápido, agresivo

**Precedentes excelentes:**
- **Streets of Rage 2** (Yuzo Koshiro) — uso magistral del YM2612
- **Thunder Force IV** — composición épica FM
- **Castlevania Bloodlines** — atmósfera gótica perfecta

**Herrammienta:** 
- **DefleMask Tracker** — componer directamente para YM2612
- Exportar a formato VGM → convertir a XGM (SGDK)

**Veredicto: ✅ VIABLE** — El YM2612 es perfecto para atmósfera oscura.

---

#### Efectos de Sonido

**Budget de SFX:**
```
DAC Samples (8-bit):
├── Sword slash       (2 KB)
├── Arrow fire        (1 KB)
├── Spell cast        (3 KB)
├── Hit/damage        (1 KB)
├── Footsteps         (2 KB)
├── Door open         (2 KB)
└── Item pickup       (1 KB)
                TOTAL: ~12 KB en ROM
```

**PSG para SFX sintéticos:**
- Explosiones (barrido de frecuencia + ruido)
- Chispas eléctricas (ruido blanco modulado)
- Teletransporte (glissando)

**Veredicto: ✅ VIABLE** — Budget manejable.

---

## PARTE III: DIFERENCIACIÓN CREATIVA

### 3.1 Premisa Narrativa Original

#### ❌ NO USAR:
- Tristram
- Diablo / Mefisto / Baal
- Rey Leoric
- Deckard Cain
- La Catedral
- Horadrim
- Santuario (nombre del mundo)
- Clases exactas (Warrior, Rogue, Sorcerer)

#### ✅ CREAR DESDE CERO:

**Nombre de la Ciudad:** **ASHENMOOR**

**Premisa:**
> Ashenmoor, una ciudad-fortaleza al borde del Bosque Olvidado, ha sido consumida por una corrupción sobrenatural. Los ciudadanos desaparecen cada noche. Los que regresan ya no son humanos.
> 
> Bajo la ciudad, antiguas ruinas de una civilización pre-humana han despertado. Algo llamado **EL VÍNCULO** está rompiendo la barrera entre el mundo de los vivos y el Reino de las Sombras.
>
> Tú eres uno de los últimos **Vigilantes** — guerreros entrenados para sellar rupturas entre mundos. Pero esta vez, el Vínculo es demasiado fuerte. La única manera de detenerlo es destruirlo desde adentro.
>
> Debes descender a las **Profundidades de Ashenmoor** y cortar el Vínculo antes de que toda la región caiga en la oscuridad eterna.

---

### 3.2 Estructura del Juego

#### MUNDO: LAS PROFUNDIDADES DE ASHENMOOR

```
┌─────────────────────────────────────┐
│  ACTO I: LA CIUDAD                  │
├─────────────────────────────────────┤
│  ○ Ashenmoor (HUB)                  │
│     - NPCs, tiendas, quests         │
│     - Cambia según progreso         │
│                                     │
│  ○ Nivel 1: La Cripta Olvidada      │
│  ○ Nivel 2: Túneles del Osario      │
│                                     │
│  ⚔️ Boss: EL CARCELERO               │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│  ACTO II: LAS CATACUMBAS            │
├─────────────────────────────────────┤
│  ○ Nivel 3: Templo Profanado        │
│  ○ Nivel 4: Salones de los Caídos   │
│                                     │
│  ⚔️ Boss: ARCHIMANDRITA CORROMPIDO   │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│  ACTO III: LAS MINAS MALDITAS       │
├─────────────────────────────────────┤
│  ○ Nivel 5: Galerías Inundadas      │
│  ○ Nivel 6: Fundición Rota          │
│                                     │
│  ⚔️ Boss: COLOSO DE HIERRO IMPURO    │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│  ACTO IV: EL SANTUARIO DE SOMBRAS   │
├─────────────────────────────────────┤
│  ○ Nivel 7: Cámara del Vínculo      │
│  ○ Nivel 8: Nexo de las Almas       │
│                                     │
│  ⚔️ Boss Final: EL VÍNCULO VIVIENTE  │
└─────────────────────────────────────┘
```

**Total: 8 niveles + HUB dinámico**

---

### 3.3 Clases de Personaje

#### 1. **EL TEMPLARIO** (Clase cuerpo a cuerpo)
```
Estadísticas base:
├── Fuerza:       12
├── Destreza:      6
├── Inteligencia:  4
└── Vitalidad:    10

Armas preferidas: Espadas, mazas, hachas
Habilidades:
├── Golpe Sagrado (consume estamina, daño +150%)
├── Escudo de Luz (reduce daño 50% durante 5s)
└── Grito de Guerra (stun a enemigos cercanos)
```

#### 2. **LA RASTREADORA** (Clase a distancia)
```
Estadísticas base:
├── Fuerza:        6
├── Destreza:     12
├── Inteligencia:  6
└── Vitalidad:     8

Armas preferidas: Arco, ballesta, dagas
Habilidades:
├── Disparo Múltiple (3 flechas en abanico)
├── Trampa de Sombra (inmoviliza enemigos)
└── Sigilo (invisibilidad temporal)
```

#### 3. **EL ARCANISTA** (Clase mágica)
```
Estadísticas base:
├── Fuerza:        4
├── Destreza:      6
├── Inteligencia: 12
└── Vitalidad:     6

Hechizos:
├── Llamarada (proyectil de fuego)
├── Cadena de Relámpagos (golpea 3 enemigos)
├── Muro de Escarcha (barrera congelante)
├── Drenar Esencia (absorbe vida)
└── Invocación: Familiar (gólem temporal)
```

---

### 3.4 Sistema de Enemigos

#### Taxonomía de los Corrompidos

**TIER 1: Los Hundidos** (Niveles 1-2)
```
○ Cadáver Ambulante  — zombi lento, alto HP
○ Espectro           — fantasma rápido, bajo HP
○ Enlazado           — humano corrompido, ataque medio
```

**TIER 2: Los Devotos** (Niveles 3-4)
```
○ Monje Profanado    — caster enemigo, baja defensa
○ Guardián Pétreo    — golem lento, alta defensa
○ Flagelante         — berserker rápido
```

**TIER 3: Los Forjados** (Niveles 5-6)
```
○ Autómata Roto      — robot corrupto, resistente
○ Amalgama de Fuego  — elemental, ataque AOE
○ Fragmento de Hierro— mini-boss errante
```

**TIER 4: Los Nacidos del Vacío** (Niveles 7-8)
```
○ Devorador de Almas — demonio volador
○ Corruptor          — caster poderoso
○ Titán del Vínculo  — boss menor
```

---

### 3.5 Identidad Visual

#### Paleta de Colores

**Paleta 1: Ashenmoor (ciudad)**
```
#1A1A2E  — Negro azulado (sombras)
#16213E  — Azul noche (cielo/paredes)
#3D405B  — Gris pizarra (piedra)
#81A1C1  — Azul claro (ventanas, agua)
#E5B567  — Dorado apagado (luz de antorchas)
#D08770  — Naranja quemado (fuego)
```

**Paleta 2: Cripta**
```
#0F0F0F  — Negro absoluto
#2B2B2B  — Gris oscuro (piedra)
#4A4A4A  — Gris medio
#8B0000  — Rojo sangre oscuro
#CD5C5C  — Rojo sangre
#F0E68C  — Amarillo hueso
```

**Paleta 3: Minas**
```
#1C1C1C  — Negro carbón
#3E2723  — Marrón óxido
#5D4037  — Marrón tierra
#FF6F00  — Naranja lava
#FFAB00  — Amarillo metal fundido
#B0BEC5  — Gris metálico
```

**Paleta 4: Santuario**
```
#000000  — Negro puro
#1A0033  — Púrpura oscuro
#4A148C  — Púrpura profundo
#7B1FA2  — Púrpura místico
#E1BEE7  — Lila pálido (energía)
#00E676  — Verde enfermizo (corrupción)
```

---

### 3.6 Música Original

#### Temas Composicionales

**1. Ashenmoor (Normal)**
```
Tempo:       80 BPM
Tonalidad:   D menor
Atmósfera:   Melancólica, esperanzadora
Instrumentos:
├── YM2612 Ch1: Melodía de guitarra acústica simulada
├── YM2612 Ch2: Contrapunto de cuerda
├── YM2612 Ch3: Bajo suave
└── PSG Ch1-2: Viento ambiental
```

**2. Cripta**
```
Tempo:       60 BPM
Tonalidad:   C menor
Atmósfera:   Opresiva, claustrofóbica
Instrumentos:
├── YM2612 Ch1: Órgano FM (notas largas)
├── YM2612 Ch2: Disonancia (segundas menores)
├── YM2612 Ch5: Percusión metálica
└── PSG Ch4: Ruido blanco filtrado (eco)
```

**3. Boss Battle**
```
Tempo:       140 BPM
Tonalidad:   E menor
Atmósfera:   Intensa, agresiva
Instrumentos:
├── YM2612 Ch1-2: Melodía sincronizada (power chords FM)
├── YM2612 Ch5: Batería FM rápida
├── YM2612 Ch6 (DAC): Bombo y caja samples
└── PSG: Arpeggios rápidos
```

---

## PARTE IV: VIABILIDAD LEGAL

### 4.1 Análisis de Riesgo de Propiedad Intelectual

#### ✅ SEGURO (No infringe)

**Mecánicas de Juego:**
- Perspectiva isométrica
- Sistema de stats (STR/DEX/INT)
- Combate en tiempo real
- Dungeons con niveles
- Sistema de loot
- Clases de personaje

→ **Las mecánicas no están protegidas por copyright.**

**Género:**
- "Action RPG"
- "Dungeon crawler"
- "Hack and slash"

→ **Los géneros no pueden ser patentados.**

#### ⚠️ ZONA GRIS (Requiere cambios)

**Nombres muy similares:**
- ❌ "Tristram" → ✅ "Ashenmoor"
- ❌ "Cathedral" → ✅ "Cripta / Templo"
- ❌ "Diablo" → ✅ "El Vínculo"

**Estética visual muy similar:**
- Si los tiles parecen "screenshots convertidos"
- Si los sprites son claramente derivados

→ **Solución: Arte original desde cero, referencias indirectas.**

#### ❌ PROHIBIDO (Infracción directa)

**Assets extraídos:**
- Sprites de Diablo descompilados
- Música convertida de Diablo
- Mapas exportados de archivos .dun
- Diálogos copiados textualmente
- Logo de Blizzard / Diablo

→ **Solución: 0% de assets de terceros.**

---

### 4.2 Estrategia Legal Recomendada

#### 1. **Declaración de Obra Original**

En el README y en la ROM:
```
NIGHTMARE — A 16-Bit Dark Fantasy Action RPG
© 2026 [Tu Nombre / Estudio]

This is an original work inspired by classic action RPGs 
of the 1990s. All code, graphics, music, and content 
are created from scratch.

Not affiliated with or endorsed by any third parties.
```

#### 2. **Documentación del Proceso Creativo**

Mantener evidencia de:
- Pixel art creado desde cero (time-lapse, archivos .aseprite)
- Composiciones musicales originales (archivos de DefleMask)
- Código fuente con historial de Git
- Diseño de niveles en Tiled (archivos .tmx)

#### 3. **Licencia Open Source (Opcional)**

Si decides hacer el proyecto open source:
```
Código:    MIT License o GPL-3.0
Assets:    CC BY-NC-SA 4.0 (no comercial)
```

Esto demuestra transparencia y buena fe.

#### 4. **Política de Fan Projects**

Si en algún momento Blizzard/Microsoft contacta:
- Responder rápidamente
- Demostrar que es obra original
- Si solicitan cambios, cooperar

**Precedente positivo:**
- AM2R (Another Metroid 2 Remake) — C&D pero no litigio
- Pokémon Uranium — C&D, proyecto retirado
- OpenMW (Morrowind) — Bethesda lo permite (requiere juego original)

---

## PARTE V: HOJA DE RUTA DE DESARROLLO

### 5.1 Metodología Propuesta

**Enfoque: Iteración Vertical (Vertical Slice)**

No construir todos los sistemas al 10%, sino un nivel completo al 100%.

```
MVP (Minimum Viable Product):
└── 1 nivel jugable de principio a fin

Vertical Slice:
├── 1 clase jugable completa
├── 1 tileset completo (Cripta)
├── 3 tipos de enemigos
├── Sistema de combate funcional
├── 1 boss
├── Sistema de inventario básico
├── 1 tema musical
└── HUD completo
```

**Ventajas:**
- ✅ Prueba de concepto inmediata
- ✅ Detecta problemas técnicos temprano
- ✅ Permite iteración de gameplay
- ✅ Demostración para comunidad

---

### 5.2 Fases de Desarrollo

#### **FASE 0: Pre-producción** (2-3 semanas)

```
□ Configurar entorno SGDK
□ Compilar primer "Hello World" ROM
□ Estudiar ejemplos de SGDK (samples/sprite)
□ Crear pipeline de assets (Aseprite → SGDK)
□ Definir architecture de código
□ Crear GDD (Game Design Document) detallado
```

**Entregable:** ROM ejecutable con sprite que se mueve con D-Pad.

---

#### **FASE 1: Motor de Tiles** (3-4 semanas)

```
□ Implementar tilemap renderer isométrico
□ Crear tile editor workflow (Tiled → C array)
□ Sistema de cámara (scroll suave)
□ Collision detection (tiles walkable/bloqueados)
□ Sistema de puertas
□ Transición entre screens/rooms
```

**Entregable:** Dungeon navegable con cámara funcionando.

---

#### **FASE 2: Sistema de Jugador** (3-4 semanas)

```
□ Sprite de jugador con 8 direcciones
□ Animaciones: idle, walk, attack
□ Input handling (D-Pad + 3 botones)
□ Físicas de movimiento
□ Stats básicos (HP, damage)
□ Sistema de muerte/respawn
```

**Entregable:** Jugador controlable con animaciones fluidas.

---

#### **FASE 3: Sistema de Combate** (4-5 semanas)

```
□ Implementar enemigo #1 (Cadáver Ambulante)
□ AI básica (perseguir jugador)
□ Melee attack del jugador
□ Hit detection player → enemy
□ Hit detection enemy → player
□ Sistema de daño/muerte
□ Feedback visual (hit flash, knockback)
□ Drop de items básico
```

**Entregable:** Combate funcional contra zombis.

---

#### **FASE 4: Sistemas RPG** (3-4 semanas)

```
□ Sistema de inventario
□ Equipment system (arma, armadura, anillo)
□ Stats (STR, DEX, INT, VIT)
□ Sistema de experiencia/level up
□ Oro y compra/venta
□ Pantalla de character sheet
```

**Entregable:** RPG systems completos.

---

#### **FASE 5: Contenido Nivel 1** (3-4 semanas)

```
□ Diseñar layout completo Nivel 1
□ Crear todos los tiles necesarios (pixel art)
□ Implementar enemigos #2 y #3
□ Añadir cofres con loot
□ Boss del Nivel 1 (El Carcelero)
□ Entrada y salida del dungeon
□ Música y SFX
```

**Entregable:** **VERTICAL SLICE COMPLETA** 🎉

---

#### **FASE 6: Ashenmoor (HUB)** (4-5 semanas)

```
□ Diseñar ciudad de Ashenmoor
□ NPCs (vendedor, quest giver, herrero)
□ Sistema de diálogo
□ Tienda (compra/venta)
□ Sistema de quests básico
□ Estado dinámico de ciudad
```

**Entregable:** HUB funcional con NPCs.

---

#### **FASE 7-10: Contenido Completo** (12-16 semanas)

```
□ Niveles 2-8
□ Todos los enemigos
□ 4 bosses
□ Las 3 clases jugables
□ Sistema de hechizos completo
□ Toda la música (8 temas)
□ Polish y balance
□ Testing intensivo
```

**Entregable:** Juego completo.

---

### 5.3 Estimación Temporal

**Escenario Realista (1 persona, part-time):**

```
Fase 0:      3 semanas
Fase 1:      4 semanas
Fase 2:      4 semanas
Fase 3:      5 semanas
Fase 4:      4 semanas
Fase 5:      4 semanas
───────────────────────
SUBTOTAL:   24 semanas (~6 meses) → VERTICAL SLICE

Fase 6:      5 semanas
Fases 7-10: 16 semanas
Polish:      4 semanas
───────────────────────
TOTAL:      49 semanas (~12 meses) → JUEGO COMPLETO
```

**Con ayuda de IA (código, assets, música):**
- Reducción estimada: **30-40%**
- Nuevo total: **~8 meses** para juego completo

---

## PARTE VI: RECURSOS Y HERRAMIENTAS

### 6.1 Herramientas de Desarrollo

#### **Core Development**
```
□ SGDK (Sega Genesis Development Kit)
  → https://github.com/Stephane-D/SGDK
  → Versión: 2.00+ recomendada

□ GCC (m68k-elf-gcc)
  → Incluido con SGDK

□ Git (control de versiones)
  → Repositorio privado o público según preferencia
```

#### **Emuladores**
```
□ BlastEm (más preciso)
  → https://www.retrodev.com/blastem/

□ Gens KMod (debugging)
  → Breakpoints, VRAM viewer

□ RetroArch (testing en múltiples cores)
```

#### **Pixel Art**
```
□ Aseprite (recomendado)
  → Export a tileset PNG
  → Paletas personalizadas

□ GraphicsGale (alternativa gratuita)

□ Tile Molester (edición directa de tiles)
```

#### **Mapas**
```
□ Tiled Map Editor
  → Exportar a TMX
  → Script Python: tmx_to_c.py (crear)
```

#### **Música**
```
□ DefleMask Tracker
  → Composición nativa YM2612/PSG
  → Export a VGM

□ XGMTool (SGDK)
  → Convierte VGM a XGM (formato de SGDK)
```

#### **SFX**
```
□ Audacity (edición)
□ FamiTracker (síntesis retro)
□ sfxr / BFXR (síntesis procedural)
```

---

### 6.2 Recursos de Aprendizaje

#### **SGDK Tutorials**
```
□ Ohsat Games - SGDK Tutorial Series
  → https://www.ohsat.com/tutorial/

□ Danibus (YouTube) - SGDK desde cero

□ SGDK Documentation (oficial)
  → /sgdk/doc/
```

#### **Genesis Programming**
```
□ Sega Genesis Programming (libro)
  → Disponible en PDF

□ Plutiedev (recursos técnicos)
  → https://plutiedev.com/

□ Mega Cat Studios (tutoriales)
```

#### **Game Design**
```
□ "Game Programming Patterns" - Robert Nystrom
□ "Procedural Content Generation in Games" (PCG Book)
□ GDC Talks sobre ARPGs clásicos
```

---

### 6.3 Comunidades

```
□ r/SegaGenesis (Reddit)
□ r/SGDK (Reddit)
□ Sega-16 Forums
□ SegaXtreme Forums
□ Discord: SGDK Community
```

---

## PARTE VII: ANÁLISIS DE RIESGOS

### 7.1 Riesgos Técnicos

| Riesgo | Probabilidad | Impacto | Mitigación |
|--------|--------------|---------|------------|
| **Límite de sprites alcanzado** | Alta | Alto | - Sistema de culling agresivo<br>- Máximo 8-10 enemigos en pantalla<br>- Efectos minimalistas |
| **Flicker de sprites** | Media | Medio | - Prioritizar sprites críticos<br>- Alternar visibilidad frame por frame |
| **Performance de CPU (60 FPS)** | Media | Alto | - Profiling constante<br>- Fixed-point math en lugar de float<br>- Lookup tables |
| **Overflow de VRAM** | Alta | Alto | - Tile reuse agresivo<br>- Paginación por área<br>- Paletas compartidas |
| **Complejidad de AI** | Baja | Medio | - State machines simples<br>- Pathfinding básico (tile-based A*) |

---

### 7.2 Riesgos de Contenido

| Riesgo | Probabilidad | Impacto | Mitigación |
|--------|--------------|---------|------------|
| **Scope creep** | Alta | Alto | - Feature freeze después de Fase 5<br>- Lista de "nice to have" vs "must have" |
| **Pixel art incompleto** | Media | Alto | - Asset list detallado<br>- Pipeline automatizado<br>- Uso de IA para base + refinamiento manual |
| **Música insuficiente** | Baja | Medio | - 4-5 temas esenciales<br>- Reutilización contextual |
| **Balanceo de gameplay** | Media | Medio | - Playtesting frecuente<br>- Sistema de stats escalable |

---

### 7.3 Riesgos Legales

| Riesgo | Probabilidad | Impacto | Mitigación |
|--------|--------------|---------|------------|
| **C&D de Blizzard/Microsoft** | Baja | Alto | - 0% assets de Diablo<br>- Naming completamente diferente<br>- Declaración de originalidad |
| **Infracción no intencional** | Baja | Medio | - Revisión legal preventiva<br>- Comunidad de feedback |
| **Problemas con SGDK license** | Muy Baja | Bajo | - SGDK es MIT license (permisivo) |

---

## PARTE VIII: PRESUPUESTO DE RECURSOS

### 8.1 ROM Budget

**Target: 2 MB (16 Megabits) — tamaño común de juegos Genesis**

```
ROM Layout (2048 KB):
├── Código ejecutable            ~256 KB   (12%)
├── Tile data (todos niveles)    ~512 KB   (25%)
├── Sprite data                  ~384 KB   (19%)
├── Mapas de niveles             ~256 KB   (12%)
├── Música (XGM)                 ~384 KB   (19%)
├── SFX                          ~128 KB   (6%)
├── Strings/diálogos             ~64 KB    (3%)
└── Lookup tables / data         ~64 KB    (3%)
                        TOTAL:    2048 KB   (100%)
```

**Si necesitáramos más:**
- Expandir a 4 MB (32 Megabits) — perfectamente viable
- Bank switching para música (cargar bajo demanda)

---

### 8.2 RAM Budget (64 KB total)

```
RAM Allocation:
├── Stack                        ~4 KB
├── Heap (general)               ~8 KB
├── Jugador (stats, inventory)   ~2 KB
├── Enemigos (16 × ~512 bytes)   ~8 KB
├── Projectiles pool             ~2 KB
├── Items en suelo               ~2 KB
├── Particle system              ~4 KB
├── Tilemap buffer               ~8 KB
├── Sprite cache                 ~8 KB
├── Audio buffers                ~8 KB
├── Save game buffer             ~4 KB
└── Misc / overhead              ~6 KB
                        TOTAL:   ~64 KB
```

**Crítico:** Gestión cuidadosa de memoria dinámica.

---

### 8.3 VRAM Budget (64 KB)

```
VRAM Allocation (por frame):
├── Plane A Tilemap              ~3.5 KB  (64×28 tiles)
├── Plane B Tilemap              ~3.5 KB
├── Window Plane                 ~1 KB    (UI)
├── Sprite Attribute Table       ~1 KB
├── Tile Data (Background)       ~28 KB   (~875 tiles)
├── Tile Data (Sprites)          ~24 KB   (~750 tiles)
├── Paletas (4 × 16 colores)     ~128 bytes
└── Overhead                     ~2 KB
                        TOTAL:   ~63 KB
```

**Optimización:**
- Flip horizontal/vertical para reducir tiles únicos
- Paletas compartidas entre assets
- Streaming de tiles entre áreas

---

## PARTE IX: CONCLUSIÓN Y RECOMENDACIÓN

### 9.1 Veredicto Final

**✅ SÍ, ES COMPLETAMENTE VIABLE.**

Este proyecto tiene:

1. **Viabilidad Técnica Confirmada**
   - Precedentes exitosos en Genesis (Landstalker, Beyond Oasis)
   - SGDK proporciona herramientas maduras
   - Hardware suficiente para ARPG isométrico

2. **Seguridad Legal (si se ejecuta correctamente)**
   - 0% de assets de Diablo
   - Inspiración de género, no copia
   - Documentación de originalidad

3. **Diferenciación Creativa Clara**
   - Nueva narrativa (Ashenmoor vs Tristram)
   - Nuevos personajes y enemigos
   - Música original para YM2612
   - Arte desde cero

4. **Roadmap Realista**
   - Vertical Slice en ~6 meses (part-time)
   - Juego completo en ~8-12 meses
   - Enfoque iterativo reduce riesgo

---

### 9.2 Recomendación: Enfoque de 3 Fases

#### **FASE ALPHA: Vertical Slice** (Prioridad 1)
```
Objetivo: Demostrar que el concepto funciona
Contenido:
└── 1 nivel completo jugable (Cripta)
    ├── 1 clase (Templario)
    ├── 3 enemigos
    ├── 1 boss
    ├── Sistema de combate
    └── Inventario básico

Duración: 6 meses part-time
Criterio de éxito: ¿Es divertido? ¿Performance OK?
```

Si Alpha es exitoso → continuar.

---

#### **FASE BETA: Core Game** (Prioridad 2)
```
Objetivo: Juego completo pero sin polish
Contenido:
└── 8 niveles + Ashenmoor
    ├── 3 clases
    ├── 15 tipos de enemigos
    ├── 4 bosses
    └── Todas las mecánicas

Duración: +6 meses
Criterio de éxito: Completable de principio a fin
```

---

#### **FASE GOLD: Polish & Release** (Prioridad 3)
```
Objetivo: Producto final publicable
Contenido:
└── Bug fixing
└── Balanceo
└── Música completa
└── Testing en hardware real
└── Documentación

Duración: +2-3 meses
Release: ROM distribuible + código open source
```

---

### 9.3 Sugerencia de Nombre del Proyecto

**Nombre Oficial:** **NIGHTMARE: Ashenmoor**

**Tagline:** *"A Lost Chapter of 16-Bit Dark Fantasy"*

**En la pantalla de título:**
```
╔═══════════════════════════════════╗
║                                   ║
║         N I G H T M A R E         ║
║                                   ║
║           ASHENMOOR               ║
║                                   ║
║    A 16-BIT DARK FANTASY ARPG    ║
║                                   ║
║         © 2026 [Studio]           ║
║                                   ║
╚═══════════════════════════════════╝

      [PRESS START]
```

---

### 9.4 Primeros Pasos Concretos

**Si decides comenzar HOY, estos serían los pasos:**

```bash
# Día 1: Setup
1. Descargar SGDK
2. Configurar entorno (GCC, emulador)
3. Compilar "Hello World"
4. Ejecutar en BlastEm

# Día 2-7: Aprendizaje
5. Tutorial de SGDK (mover sprite)
6. Tutorial de tiles y tilemap
7. Estudiar ejemplo "sonic" de SGDK

# Semana 2: Primer Prototipo
8. Crear tile isométrico simple (suelo + pared)
9. Cargar tilemap 10×10
10. Sprite de jugador caminando (4 direcciones)

# Semana 3-4: Proof of Concept
11. Sistema de cámara
12. Colisiones básicas
13. "Dungeon" de prueba navegable
```

**Después de eso, ya tendrías la base para escalar.**

---

## ANEXOS

### ANEXO A: Referencias Visuales Recomendadas

**Juegos de Genesis para estudiar:**
1. **Landstalker** — isométrico, profundidad, puzzles
2. **Beyond Oasis** — combate fluido, animaciones
3. **Shadowrun** — perspectiva isométrica RPG
4. **Streets of Rage 2** — hit detection, feedback
5. **Castlevania Bloodlines** — atmósfera gótica
6. **Shining Force** — UI de RPG, menús

**ARPGs de referencia (otras plataformas):**
1. **Secret of Mana** (SNES) — combate en tiempo real
2. **Zelda: Link to the Past** (SNES) — dungeons, puzzles
3. **Ys III** (Genesis) — ARPG de acción lateral

---

### ANEXO B: Estructura de Código Recomendada

```c
/src
├── /core
│   ├── main.c              // Entry point
│   ├── game.c              // Game loop principal
│   └── vdp.c               // VDP helpers
│
├── /engine
│   ├── tilemap.c           // Tilemap renderer
│   ├── camera.c            // Sistema de cámara
│   ├── collision.c         // Detección de colisiones
│   └── physics.c           // Física básica
│
├── /entities
│   ├── player.c            // Lógica del jugador
│   ├── enemy.c             // Sistema de enemigos
│   ├── projectile.c        // Proyectiles
│   └── item.c              // Items
│
├── /systems
│   ├── combat.c            // Sistema de combate
│   ├── inventory.c         // Inventario
│   ├── stats.c             // RPG stats
│   └── ai.c                // Inteligencia artificial
│
├── /ui
│   ├── hud.c               // HUD en pantalla
│   ├── menu.c              // Menús
│   └── dialog.c            // Sistema de diálogo
│
├── /data
│   ├── maps/               // Mapas de niveles
│   ├── sprites/            // Sprite data
│   ├── tiles/              // Tile data
│   └── tables/             // Lookup tables
│
└── /audio
    ├── music.c             // Reproductor de música
    └── sfx.c               // Efectos de sonido
```

---

### ANEXO C: Checklist de Legalidad

**Antes de publicar el proyecto, verificar:**

- [ ] 0% de código fuente de Diablo/DevilutionX copiado
- [ ] 0% de sprites extraídos o derivados
- [ ] 0% de tiles extraídos o derivados
- [ ] 0% de música convertida de Diablo
- [ ] 0% de SFX extraídos
- [ ] Nombres propios completamente diferentes
- [ ] Logo y branding originales
- [ ] Disclaimer de "no affiliation" incluido
- [ ] Licencia open source definida (si aplica)
- [ ] Créditos apropiados (SGDK, herramientas)

---

### ANEXO D: Métricas de Éxito

**¿Cómo saber si el proyecto es exitoso?**

**Métricas Técnicas:**
- [ ] 60 FPS consistentes en hardware real
- [ ] Sin crashes o glitches mayores
- [ ] Tiempo de carga de nivel < 2 segundos
- [ ] Input latency < 50ms

**Métricas de Gameplay:**
- [ ] Vertical Slice completable en 15-20 minutos
- [ ] Combate se siente satisfactorio
- [ ] Progresión de personaje clara
- [ ] Balance razonable (no demasiado fácil/difícil)

**Métricas de Comunidad:**
- [ ] Feedback positivo en r/SegaGenesis
- [ ] Al menos 1 video de YouTube cubriendo el proyecto
- [ ] Descargas > 500 en primera semana
- [ ] Funciona en hardware real (Everdrive, etc.)

---

## PALABRAS FINALES

Este documento demuestra que **NIGHTMARE para Genesis es completamente viable** tanto técnica como legalmente, siempre que se desarrolle como una obra original inspirada en el género, no como un port directo.

El proyecto sería:
- **Un homenaje** a los ARPGs clásicos
- **Una demostración técnica** de lo que la Genesis puede hacer
- **Una exploración creativa** de "qué habría sido si..."
- **Un experimento de desarrollo moderno** con herramientas retro

Más que "port impossible", sería **"el Diablo que nunca existió"** — una arqueología imaginaria de una versión alternativa de los 90s.

**La pregunta no es "¿se puede hacer?"**

**La pregunta es: "¿cuándo empezamos?"**

---

**Siguiente paso recomendado:** 
Configurar SGDK y compilar el primer "Hello World" ROM para validar el entorno de desarrollo.

**¿Listo para empezar la Fase 0?** 🎮

---

*Documento generado: 12 de Septiembre de 2026*  
*Versión: 1.0*  
*Estado: LISTO PARA INICIAR DESARROLLO*
