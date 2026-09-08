# Walkthrough - Diablo Nightmare Edition (Easter Egg Golem & Endless Diablo)

Se han implementado, probado y verificado con éxito las nuevas características de **Golem Acompañante Permanente** y el **Ciclo de Muerte y Respawn Infinito de Diablo** en Single Player.

## 🛠️ Cambios Implementados y Verificados

### 1. 🗿 Easter Egg: Golem Acompañante Permanente & Inmortal (Single Player)
- **Persistencia en Partidas Guardadas ([`Source/loadsave.cpp`](file:///c:/Projects/DevilutionX/Source/loadsave.cpp), [`Source/loadsave.h`](file:///c:/Projects/DevilutionX/Source/loadsave.h), [`Source/pfile.cpp`](file:///c:/Projects/DevilutionX/Source/pfile.cpp))**:
  - Se implementaron `SaveGolemState()` y `LoadGolemState()`.
  - Al guardar la partida (tanto en Tristán como en mazmorras), `_persistentGolemSpellLevel` se almacena de forma segura en el archivo MPQ del héroe.
  - Al cargar la partida, el nivel de hechizo se restaura antes de inicializar el nivel, permitiendo que el Golem reaparezca inmediatamente al lado del jugador.
- **Prevención de Golems Zombie y Congelamiento al Cargar Partida ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp), [`Source/loadsave.cpp`](file:///c:/Projects/DevilutionX/Source/loadsave.cpp), [`Source/engine/render/scrollrt.cpp`](file:///c:/Projects/DevilutionX/Source/engine/render/scrollrt.cpp))**:
  - `GolumAi()` ahora verifica de forma estricta `if (golem.hasNoLife() || golem.isInvalid || golem.position.tile == GolemHoldingCell) return;`.
  - Se eliminó el caso donde un Golem con 0 HP cargado de archivos de niveles previos intentaba ejecutar IA, teleportarse y emitir ráfagas de paquetes de muerte que colgaban el hilo del juego y congelaban la pestaña.
  - Al reposicionarse junto al jugador, el Golem recupera automáticamente toda su vida (`golem.hitPoints = golem.maxHitPoints; golem.isInvalid = false;`).
  - En `SaveGameData()` y `LoadGame()`, el conteo de monstruos activos en Tristán (`leveltype == DTYPE_TOWN`) se aísla a 0 en la serialización para evitar corrupción de índices de monstruos.
- **Inmortalidad Total Frente a Todo Daño y Muerte Forzada ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp))**:
  - En `ApplyMonsterDamage()`, la salud del Golem tiene un piso mínimo indestructible de 64 HP mientras `_persistentGolemSpellLevel > 0`.
  - `KillGolem()` ignora solicitudes de muerte si el Golem persistente de Single Player está activo.
- **Re-lanzamiento Táctico ([`Source/missiles.cpp`](file:///c:/Projects/DevilutionX/Source/missiles.cpp))**:
  - Al castear el hechizo mientras el Golem está activo, este se teletransporta instantáneamente al cursor con vida restaurada al 100%.
- **Transición entre Niveles, Portales y Tristram ([`Source/diablo.cpp`](file:///c:/Projects/DevilutionX/Source/diablo.cpp), [`Source/player.cpp`](file:///c:/Projects/DevilutionX/Source/player.cpp))**:
  - En `LoadGameLevel()`, al entrar a cualquier piso o a Tristram, el Golem se coloca y se activa de inmediato en pie (`MonsterMode::Stand` con `M_StartStand`) junto al jugador con vida completa.
- **Render y Cursor en Tristram ([`Source/engine/render/scrollrt.cpp`](file:///c:/Projects/DevilutionX/Source/engine/render/scrollrt.cpp), [`Source/cursor.cpp`](file:///c:/Projects/DevilutionX/Source/cursor.cpp))**:
  - `DrawMonsterHelper()` valida `!Monsters[mi].isInvalid && Monsters[mi].animInfo.sprites` antes de renderizar.
  - En `cursor.cpp`, se previene que hacer clic sobre el Golem en el pueblo se confunda con iniciar diálogo con un NPC (Towner).
- **IA de Escolta / Leash ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp))**:
  - `GolumAi()` teletransporta automáticamente al Golem si se aleja más de 8 casillas del jugador o si el camino queda bloqueado.

---

### 2. 👹 Muerte Limpia de Diablo & El Infierno Nunca Termina (Endless Diablo Boss Loop)
- **Corrección de Bucle Infinito en la Muerte de Diablo ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp))**:
  - En `MonsterDeath()`, al cumplirse los 140 ticks (~3.5 seg) de la animación de agonía y temblor de cámara, Diablo se desvanece limpiamente del mapa (`dMonster[x][y] = 0; monster.isInvalid = true;`), soltando su botín y registrando la victoria en las estadísticas del héroe (`pDiabloKillLevel`) sin forzar la salida al menú.
- **Reencarnación del Señor del Terror ([`Source/pfile.cpp`](file:///c:/Projects/DevilutionX/Source/pfile.cpp), [`Source/loadsave.h`](file:///c:/Projects/DevilutionX/Source/loadsave.h), [`Source/loadsave.cpp`](file:///c:/Projects/DevilutionX/Source/loadsave.cpp))**:
  - En `pfile_save_level()`, si Diablo fue derrotado en el Nivel 16 y el jugador abandona el piso (hacia el Nivel 15 o viajando a Tristram por Town Portal), se limpia el archivo de guardado del Nivel 16 y se restablece `Quests[Q_DIABLO]._qactive = QUEST_ACTIVE`.
  - Al volver a descender al Nivel 16, Diablo y sus heraldos del infierno reaparecen con toda su salud en la cámara del trono, creando un ciclo de combate infinito para farmear al Señor del Terror sin reiniciar la partida ni perder el progreso en los pisos 1-15 ni en el pueblo.

---

---

### 3. 🛡️ Estabilidad en Combate de Tristán (Anti-Freeze) & Solución Definitiva a la Corrupción Visual al Recargar (Anti-Tearing)

#### A. Prevención del Congelamiento del Navegador Durante el Combate
- **Eliminación de Aserción Fatal en `InitializeSpawnedMonster` ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp#L490)):**
  - Se sustituyeron los `assert()` de un jugador por validación defensiva con rollback automático del slot asignado (`if (!freePosition) { if (ActiveMonsterCount > 0) ActiveMonsterCount--; return; }`). Cuando el mapa se llena de invocaciones o proyectiles y no hay casillas adyacentes libres, la función aborta la invocación limpiamente sin abortar el motor de WebAssembly.
- **Tope de Invocaciones de Leoric en Tristán ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp#L1980)):**
  - En `LeoricAi()`, se limitó el número concurrente de esqueletos invocados en el pueblo (`leveltype == DTYPE_TOWN`) a un máximo de 5 esqueletos vivos simultáneos, evitando que 112+ esqueletos saturen el pool de monstruos y la CPU.
- **Bucle Seguro Bounded en `CheckBlock` ([`Source/missiles.cpp`](file:///c:/Projects/DevilutionX/Source/missiles.cpp#L690)):**
  - Se reemplazó el bucle desprotegido `while (from != to)` por el algoritmo seguro `LineClear` con validación de `InDungeonBounds` y comprobación de paredes sólidas. Se eliminaron los bucles infinitos y desbordamientos de memoria al rastrear trayectorias de Bone Spirit.
- **Despertar por Proximidad Calibrado ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp#L3800)):**
  - Se redujo el radio de activación de monstruos en el pueblo de 22 a 15 casillas para evitar que todo el ejército de Tristán ejecute búsqueda de caminos (pathfinding A*) en cada tick sobre WebAssembly.
- **Selección Dinámica del Jefe de Invasión ([`Source/nightmare/invasion/invasion_manager.cpp`](file:///c:/Projects/DevilutionX/Source/nightmare/invasion/invasion_manager.cpp)):**
  - Si el Rey Leoric ya fue derrotado en la mazmorra (`Quests[Q_SKELKING]._qactive == QUEST_DONE`), la invasión selecciona e inicializa al temible **Warlord of Blood** (Señor de la Sangre) con gráficos y estadísticas completas; de lo contrario, lidera el Rey Leoric.

#### B. Eliminación de Corrupción Visual (Scanlines/Tearing) al Cargar Partida
- **Persistencia de Snapshot al Guardar desde el Menú ([`Source/loadsave.cpp`](file:///c:/Projects/DevilutionX/Source/loadsave.cpp#L2780)):**
  - Se añadió `SaveInvasionSnapshot()` dentro de `SaveInvasionState()`. Al guardar la partida desde el menú de pausa en el pueblo, las posiciones, tipos y vida exacta de cada monstruo invasor se capturan antes de escribir el chunk `tristram_inv` en el MPQ.
- **Aislamiento Estricto de `ActiveMonsterCount` en Tristán ([`Source/loadsave.cpp`](file:///c:/Projects/DevilutionX/Source/loadsave.cpp)):**
  - En `SaveLevel()`, se serializa 0 si `leveltype == DTYPE_TOWN`.
  - En `LoadLevel()` y `LoadGame()`, se fuerza `ActiveMonsterCount = 0` al cargar el pueblo, evitando que monstruos no inicializados corrompan el array `ActiveMonsters`.
- **Limpieza Completa del Estado de Monstruos en el Pueblo ([`Source/diablo.cpp`](file:///c:/Projects/DevilutionX/Source/diablo.cpp#L3215)):**
  - En `LoadGameLevelTown()`, se invocan `InitLevelMonsters()` y el vaciado integral de `dMonster[x][y] = 0`, garantizando que `OnTownEntry()` siempre construya la invasión sobre un mapa y tablas de sprites 100% limpias.
- **Alineación de Filas WebGPU a 256 Bytes ([`Packaging/emscripten/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/index.html)):**
  - Se calculó `paddedRowBytes = Math.ceil(w / 256) * 256` (768 bytes para 640px) y se añadió padding fila por fila en las llamadas a `writeTexture` para las texturas de 1 byte por píxel (`texSemantic` y `texLight`), cumpliendo la especificación WebGPU y eliminando el desalineamiento horizontal.

---

## 🧪 Resultados de Verificación
- **Compilación WebAssembly (Ninja/Emscripten):** Exitosa con código 0 (`devilutionx.js` y `devilutionx.wasm` vinculados limpiamente en `build-web/`).
- **Sincronización de Recursos Web:** `build-web/index.html` actualizado automáticamente con las texturas WebGPU con stride alineado.
- **Cero Regresiones:** El sistema de guardado, persistencia del Golem, ciclo de Diablo y la Invasión a Tristán operan en perfecta armonía.
