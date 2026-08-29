# NIGHTMARE AUDIT MATRIX — DevilutionX / Hellfire / Restoration Layer

> **Proyecto:** DevilutionX NIGHTMARE  
> **Fork Base:** `ifernandez89/DevilutionX`  
> **Fecha de Auditoría:** 28 de Agosto, 2026  
> **Contrato de Proveniencia:**  
> `HEREDADO DE DEVILUTIONX` ≠ `HEREDADO DE HELLFIRE` ≠ `MODIFICADO POR NIGHTMARE` ≠ `CONTENIDO ORIGINAL RECUPERABLE` ≠ `CONTENIDO NUEVO DE NIGHTMARE`

---

## Leyenda de Estados

| Icono | Estado | Descripción |
|:---:|:---|:---|
| 🟢 | **Ya funcional** | Operativo y probado en el motor actual |
| 🟡 | **Existe pero incompleto/no conectado** | Código o datos parciales cargados, pendiente enlace total |
| 🔵 | **Existe en assets originales** | Assets reales en el MPQ, dormidos sin uso en NIGHTMARE |
| 🔴 | **No existe / Por implementar** | Requiere desarrollo nuevo o reconstrucción |
| ⭐ | **Prioridad de Recuperación** | Candidato principal para próximos sprints de restauración |

---

## 01. CLASSES (Clases de Personajes)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Warrior** | Heredado de DevilutionX | 🟢 | Combate melé clásico, Repair skill. |
| **Rogue** | Heredado de DevilutionX | 🟢 | Disparo con arco, Disarm Trap skill. |
| **Sorcerer** | Heredado de DevilutionX | 🟢 | Magia pura, Recharge Staff skill. |
| **Monk** | Heredado de Hellfire | 🟢 | Staff Cleave, ropa ligera (+2 AC/lvl), skill Search innato, daño unarmed `(Str+Dex)*lvl/150`. |
| **Barbarian** | Heredado de Hellfire | 🟢 | Habilitado en DevilutionX. Hacha/Maza bonificado, Rage skill, swing en área. |
| **Bard** | Heredado de Hellfire | 🟢 | Habilitado en DevilutionX. Dual Wielding (espadas), Identify skill. |

---

## 02. NPCs (Personajes No Jugables)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Pobladores Vanilla** (Cain, Adria, Pepin, Griswold, Ogden, Gillian, Farnham, Wirt) | Heredado de DevilutionX | 🟢 | Rutinas de diálogo, comercio y gossip vanilla completamente operativas. |
| **NPCs Hellfire** (Lester, Complete Nut) | Heredado de Hellfire | 🟢 | Integrados para el acceso a la Colmena (Hive) y la Cripta. |
| **Tremain the Priest** | Contenido Original Recuperable | 🟡 ⭐ | Sprite `priest8.cel` y 8 audios `priest00-07.wav` en MPQ. Conectado en `Source/nightmare/restoration/registry.cpp`. |
| **NPCs Secundarios / Ambientales** | Contenido Nuevo de Nightmare | 🔴 | Planeados para eventos del mundo y expansiones de lore. |

---

## 03. QUESTS (Misiones)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Misiones Diablo Vanilla** (Butcher, Leoric, Poisoned Water, Lazarus, etc.) | Heredado de DevilutionX | 🟢 | 100% funcionales con generación procedural original. |
| **Misiones Hellfire** (Grave Matters, Cornerstone of the World, Jersey's Jersey, Defiler) | Heredado de Hellfire | 🟢 | 100% integradas. |
| **Fleshdoom & Shadowfang** | Contenido Original Recuperable | 🟡 ⭐ | Quest de Tremain. Audios `priest05-07.wav` en MPQ. Spawn procedural en Catacumbas 5-7. |
| **Map of the Stars (Profecía Estelar)** | Contenido Original Recuperable | 🟡 ⭐ | Diálogos `TEXT_DOOM1-10` (`Cain22.wav`, etc.) en MPQ. Restauración narrativa sin timer. |
| **Worm Infestation** | Contenido Original Recuperable | 🔵 ⭐ | Audios `healer23/24.wav` y mapas no usados en MPQ. |
| **Butcher's Chamber & Easter Egg Wirt** | Contenido Original Recuperable | 🟡 ⭐ | Cinemática `fbutch3.smk` y audios invertidos `pegboy21/22.wav` en MPQ. Hooked en C++. |
| **Lost Treasure / Island of Sunless Sea / Izual** | Contenido Nuevo de Nightmare | 🔴 | Reconstrucción en Sprint 6 basada en documentos de diseño originales. |

---

## 04. MONSTERS (Monstruos y Jefes)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Monstruos Vanilla y Hellfire** | Heredado de DevilutionX / Hellfire | 🟢 | Todos los tipos y bosses únicos originales activos. |
| **Incinerator (`MT_INCIN` / `fireman`)** | Contenido Original Recuperable | 🟡 ⭐ | Sprites y frames (Stand, Walk, Attack, Hit, Death, Special) en MPQ. IA `FireManAi` en `Source/monster.cpp`. |
| **Arch-Lich Malignus (`MT_DARKMAGE`)** | Contenido Original Recuperable | 🟡 ⭐ | Sprites y audios `dmag*.wav` en MPQ. Mapeado a `CounselorAi` con offsets CLX/CEL. |
| **Bosses Únicos Recuperados** (Wrathfire, Warpfire) | Contenido Original Recuperable | 🟡 ⭐ | Tabulados para disponiblidad Retail en `monstdat.tsv` y `unique_monstdat.tsv`. |
| **Monstruos Cortados de Hellfire / Beta** | Contenido Original Recuperable | 🔵 | Modelos de gusanos y variantes durmientes en archivos del motor. |

---

## 05. ITEMS (Objetos y Artefactos)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Ítems Vanilla y Hellfire** | Heredado de DevilutionX / Hellfire | 🟢 | Generación de armaduras, armas, anillos, amuleto y pergaminos. |
| **Shadowfang** (Espada Única) | Contenido Original Recuperable | 🟡 ⭐ | Objeto único dormido en tablas. Recompensa de Fleshdoom. |
| **Lightforge** (Maza Única) | Contenido Original Recuperable | 🟡 ⭐ | Objeto único dormido. Recompensa final de Tremain. |
| **Strange Map** (Quest Item) | Contenido Original Recuperable | 🟡 ⭐ | Asset en MPQ. Item disparador para la Profecía Estelar. |
| **Azurewrath** (Espada de Izual) | Contenido Original Recuperable | 🔵 ⭐ | Referenciada en datos de ítems y lore. |

---

## 06. SPELLS / SKILLS (Hechizos y Habilidades)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Hechizos Vanilla y Hellfire** (Firebolt, Mana Shield, Search, Berserk, etc.) | Heredado de DevilutionX / Hellfire | 🟢 | Todos los hechizos de libro y bastón funcionales. |
| **Hechizos Dormidos / Incompletos** (Sentinel, Blood Boil, Invisibility, Ring of Fire) | Contenido Original Recuperable | 🔵 ⭐ | Lógica y gráficos parciales en el MPQ original. |

---

## 07. DUNGEONS / LEVELS (Mazmorras y Niveles)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Niveles 1-16** (Catedral, Catacumbas, Cuevas, Infierno) | Heredado de DevilutionX | 🟢 | Generación procedural de mapas 100% fiel. |
| **Niveles Hellfire** (Hive 1-4, Crypt 1-4) | Heredado de Hellfire | 🟢 | Gráficos y tilesets integrados. |
| **Área sin usar de Gusanos / Dungeon Tilesets** | Contenido Original Recuperable | 🔵 ⭐ | Tilesets y layouts presentes en el MPQ de Diablo/Hellfire. |

---

## 08. SPRITES / ANIMATIONS (Gráficos y Animaciones)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Sprites Héroes, Monstruos y Mazmorras** | Heredado de DevilutionX | 🟢 | Renderizado en formato CLX/CEL con soporte de resoluciones modernas. |
| **`priest8.cel`** (Sprite de Tremain) | Contenido Original Recuperable | 🟡 ⭐ | Cargado en C++ para renderizar a Tremain en la iglesia. |
| **`darkmage\dmage` & `fireman\firem`** | Contenido Original Recuperable | 🟡 ⭐ | Requieren ajustes de offsets de bounding box en DevilutionX. |

---

## 09. AUDIO / VOICE (Sonido y Voces)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Banda Sonora y SFX Vanilla/Hellfire** | Heredado de DevilutionX | 🟢 | Composición de Matt Uelmen y SFX nativos. |
| **Audios de Tremain** (`priest00.wav` a `priest07.wav`) | Contenido Original Recuperable | 🟡 ⭐ | Mapeados en `Source/nightmare/restoration/registry.cpp`. |
| **Wirt al Revés** (`pegboy21.wav`, `pegboy22.wav`) | Contenido Original Recuperable | 🟡 ⭐ | Integradora de probabilidad (5%-10%) tras visitar la sala del Carnicero. |
| **Profecía de Cain** (`Cain22.wav` / `TEXT_DOOM1-10`) | Contenido Original Recuperable | 🟡 ⭐ | 81 segundos de audio de Deckard Cain recuperados. |
| **Sonidos Ambientales de Tristram** (`sfx/animals/*`) | Contenido Original Recuperable | 🟡 ⭐ | Perros, vacas, ovejas y cerdos encolados condicionalmente. |

---

## 10. DIALOGUE / GOSSIP (Sistemas de Diálogo)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Diálogos de Quests y Gossip Vanilla** | Heredado de DevilutionX | 🟢 | Funcionales mediante `textdat.tsv`. |
| **Gossip Oculto sobre Tremain** | Contenido Original Recuperable | 🟡 ⭐ | Audios `Witch48`, `Storyt32`, `Drunk33`, `Bmaid38`, `Bsmith54`. |
| **Gossip Post-Carnicero & Lore de Leoric** | Contenido Original Recuperable | 🟡 ⭐ | Audios `TEXT_FARNHAM16-22`, `TEXT_PEPIN12-30`, `TEXT_GRISWOLD14-37`. |

---

## 11. AI / GAMEPLAY SYSTEMS (Inteligencia Artificial y Motor)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **IA de Monstruos Vanilla / Hellfire** | Heredado de DevilutionX | 🟢 | Patrones de movimiento, teletransporte y casteo. |
| **`FireManAi`** (Comportamiento del Incinerador) | Contenido Original Recuperable | 🟡 ⭐ | Lógica de disparo y rodado especial (bola de fuego). |
| **`CounselorAi` para Arch-Lich Malignus** | Contenido Original Recuperable | 🟡 ⭐ | Teletransporte evasivo y descarga arcana. |
| **Sistema de Estado del Mundo (`world_state.lua`)** | Contenido Nuevo de Nightmare | 🔴 | Tensión ambiental en Tristram según el progreso del jugador. |

---

## 12. LUA (Scripting de Contenido)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Integración Lua en Engine** | Modificado por Nightmare | 🟢 | Motor Lua empotrado en `Source/lua/`. |
| **Scripts de Quests Nightmare** (`fleshdoom.lua`, `map_of_stars.lua`) | Contenido Nuevo de Nightmare | 🟡 ⭐ | Lógica de misiones modularizada. |

---

## 13. GRAPHICS / LIGHTING / WEATHER (Motor Gráfico y Clima)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Sistema de Iluminación y Sombras** | Heredado de DevilutionX | 🟢 | Basado en tablas de luz originales. |
| **Weather Overlay (Sistema de Lluvia)** | Heredado de DevilutionX / Modificado | 🟢 | Efecto de lluvia en Tristram totalmente funcional. |
| **Gothic Visual Enhancement** | Modificado por Nightmare | 🟡 ⭐ | Mejoras atmosféricas de paleta y contraste (documentado en plan gráfico). |

---

## 14. UI / HUD (Interfaz de Usuario)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Interfaz Original / Escalada Panorámica** | Heredado de DevilutionX | 🟢 | Soporte para resoluciones panorámicas y UI clásica. |
| **Quest Log Extendido & Restoration Registry UI** | Contenido Nuevo de Nightmare | 🔴 ⭐ | Interfaz para visualizar el progreso de restauración sin romper la inmersión. |

---

## 15. NETWORK / MULTIPLAYER (Red y Multijugador)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **DvlNet Framework** (Loopback, TCP, UDP, ZeroTier) | Heredado de DevilutionX | 🟢 | Multijugador síncrono funcional. |

---

## 16. SAVE / DATA (Guardado de Partidas)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Sistema de Guardado `.sv` / `.hsv`** | Heredado de DevilutionX | 🟢 | Persistencia de inventario, mapas y estadísticas. |
| **Persistencia de Estado Restoration** | Modificado por Nightmare | 🟡 ⭐ | Registro en savegame de eventos de restauración completados (ej. muerte de Tremain). |

---

## 17. WEBASSEMBLY (Compilación para Navegador)

| Elemento | Proveniencia | Estado | Notas |
|:---|:---|:---:|:---|
| **Build WebAssembly (Emscripten / `build-em`)** | Heredado de DevilutionX | 🟢 | Compilación portable ejecutable en navegadores web. |
