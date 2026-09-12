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

### 4. ⚔️ Batallas Épicas en WebAssembly sin Congelamiento (Solución al Freeze en Combates Masivos)

#### A. Erradicación de Omisión de Cuadros (`drawGame` Skipping)
- **Causa Raíz:** `nthread_has_500ms_passed()` implementaba `*drawGame = ticksElapsed <= gnTickDelay;`. Si un tick de combate complejo tardaba >50ms, el motor omitía completamente la llamada a `DrawAndBlit()`. Al no renderizar cuadros sucesivos, se suspendían las subidas de textura WebGL/WebGPU (`texSubImage2D`), haciendo que el watchdog de telemetría detectara >6s sin fotogramas y reportara falsamente un cuelgue del motor.
- **Solución ([`Source/nthread.cpp`](file:///c:/Projects/DevilutionX/Source/nthread.cpp)):** Se forzó `*drawGame = true;` en `#if defined(__EMSCRIPTEN__)`, garantizando que el ciclo de presentación y refresco de pantalla permanezca siempre activo sin importar la carga de CPU.

#### B. Rendición Cooperativa al Bucle del Navegador (Yield ASYNCIFY)
- **Bucle Principal ([`Source/diablo.cpp`](file:///c:/Projects/DevilutionX/Source/diablo.cpp)):** Se introdujo una pausa cooperativa (`SDL_Delay(1)`) cada 16ms en el bucle principal `while (gbRunGame)` en WebAssembly. Esto permite que ASYNCIFY ceda temporalmente el control al event loop del navegador, procesando clics del mouse, teclado, repintados del DOM y manteniendo el navegador totalmente receptivo durante los enfrentamientos más densos.
- **Control de FPS y Sobrecarga ([`Source/engine/dx.cpp`](file:///c:/Projects/DevilutionX/Source/engine/dx.cpp)):** Anteriormente, `LimitFrameRate()` se saltaba cuando los fotogramas caían por debajo de 60 FPS (`frameDeadline <= tc`). Ahora, en WebAssembly siempre se asegura una pausa mínima de 1ms cada 16ms para prevenir que el motor gire al 100% de CPU en bucles síncronos cerrados.

#### C. Optimización de Búsqueda de Rutas A* del Golem
- **Corrección de Holding Cell & Reseteo de Rutas ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp)):**
  - Se movió la guarda `if (monster.position.tile == GolemHoldingCell) return false;` a nivel global en `AiPlanPath()`, impidiendo que golems inactivos en celdas de espera consuman CPU calculando líneas de visión.
  - Se restableció `monster.pathCount = 0;` para el Golem (antes omitido intencionalmente), evitando que `AiPlanWalk()` ejecute de forma forzada búsquedas A* de 1024 nodos en cada tick ininterrumpidamente.
  - En `GolumAi()`, se suprimió la doble búsqueda de camino hacia el jugador en el mismo tick cuando el Golem ya se encuentra trabado en combate cuerpo a cuerpo con un monstruo enemigo.

---

### 5. 🛡️ Erradicación de `memory access out of bounds` (WASM 0x4cc1aa) en Batallas y Renderizado

#### A. Diagnóstico y Desensamblado de la Instrucción 0x4cc1aa
- **Función Afectada:** Función interna 6476 del módulo WebAssembly (`SDL_Blit8to4` en SDL2), el blitter optimizado de 8 bits con paleta a 32 bits RGB888 que lee los píxeles de origen mediante Duff's device (`i32.load8_u align=0 offset=0`).
- **Secuencia de Fallo:** Durante batallas con docenas de números flotantes de daño (`DrawFloatingNumbers`) y combatientes en los límites del mapa (Golem vs Leoric y Na-Krul), las coordenadas `screenPosition` pasaban a ser negativas.
- **Underflow de Punteros en Subregiones:** `ClipSurface()` calculaba `std::min(rect.position.x + rect.size.width, out.w())` produciendo anchos negativos cuando el rectángulo estaba a la izquierda de la pantalla. Al construir la subregión, las dimensiones negativas provocaban underflow de enteros sin signo en aritmética de punteros (`at(x, y)` apuntaba a `~0xFFFFFF00`, fuera del heap de 512 MB de WebAssembly). El motor fallaba con `Uncaught RuntimeError: memory access out of bounds`, lo que detenía el bucle WebAssembly e inmediatamente activaba la alerta de congelamiento del watchdog 2.6s después.

#### B. Correcciones Aplicadas en el Núcleo
1. **Aislamiento en `Surface::subregion` ([`Source/engine/surface.hpp`](file:///c:/Projects/DevilutionX/Source/engine/surface.hpp)):**
   - `subregion()`, `subregionX()` y `subregionY()` ahora delimitan estrictamente `rx` y `ry` dentro de `[0, surface->w]` y `[0, surface->h]`, y `rw` y `rh` dentro de `[0, surface->w - rx]` y `[0, surface->h - ry]`. Es imposible generar una subregión con dimensiones negativas o desbordada del búfer físico.
2. **Recorte Seguro en `ClipSurface` y Salida Rápida en `DrawString` ([`Source/engine/render/text_render.cpp`](file:///c:/Projects/DevilutionX/Source/engine/render/text_render.cpp)):**
   - `ClipSurface()` ahora emplea `std::clamp` asegurando que tanto el ancho como el alto sean `>= 0`.
   - `DrawString()` y `DrawStringWithColors()` descartan de inmediato la operación si `clippedOut.w() <= 0 || clippedOut.h() <= 0`.
3. **Descarte de Números Flotantes Fuera de Pantalla ([`Source/qol/floatingnumbers.cpp`](file:///c:/Projects/DevilutionX/Source/qol/floatingnumbers.cpp)):**
   - `DrawFloatingNumbers()` valida de antemano si las coordenadas del texto están totalmente fuera del área visible y las omite con `continue` antes de invocar el renderizador de texto.
4. **Validación de Límites en `DoBlitScreen`, `UndrawCursor` y `Blit` ([`Source/engine/render/scrollrt.cpp`](file:///c:/Projects/DevilutionX/Source/engine/render/scrollrt.cpp), [`Source/engine/dx.cpp`](file:///c:/Projects/DevilutionX/Source/engine/dx.cpp), [`Source/engine/render/clx_render.cpp`](file:///c:/Projects/DevilutionX/Source/engine/render/clx_render.cpp)):**
   - `DoBlitScreen()` recorta el área contra los límites de pantalla `gnScreenWidth` y `gnScreenHeight`, descartando áreas no positivas antes de invocar a `BltFast()`.
   - `UndrawCursor()` valida `cursor.rect.size.width > 0 && cursor.rect.size.height > 0` antes de invocar `BlitCursor()`.
   - `Blit()` en `dx.cpp` verifica dimensiones válidas antes de delegar a `SDL_BlitSurface`.
   - `DoRenderBackwards()` y `RenderClxOutline()` verifican `out.w() > 0 && out.h() > 0` al inicio para abortar dibujos sobre regiones vacías.
5. **Cachebuster Actualizado a `v=nightmare-v6` ([`Packaging/emscripten/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/index.html)):**
   - Actualizada la referencia a `devilutionx.js?v=nightmare-v6`.

### 6. 🛡️ Erradicación de `memory access out of bounds` al Volver al Pueblo tras Derrotar a Na-Krul

#### A. Diagnóstico y Causa Raíz
- **Fuga de Luces y Desincronización de Invasión ([`Source/diablo.cpp`](file:///c:/Projects/DevilutionX/Source/diablo.cpp)):**
  - Al derrotar a Na-Krul en la Cripta 24 y tomar el portal a Tristán, `LoadGameLevel()` saltaba `InitLighting()` porque `CheckInvasionTrigger()` aún no se había evaluado.
  - Esto dejaba el arreglo global `ActiveLights` contaminado con todas las fuentes de luz del nivel 24 anterior con coordenadas de la mazmorra.
  - Al concluir la carga, `LoadGameLevelLightVision()` activaba `ProcessLightList()`, iterando sobre luces de la mazmorra con coordenadas inexistentes en el pueblo.
- **Acceso Fuera de Límites en `TileHasAny` ([`Source/levels/gendung.h`](file:///c:/Projects/DevilutionX/Source/levels/gendung.h), [`Source/lighting.cpp`](file:///c:/Projects/DevilutionX/Source/lighting.cpp)):**
  - `ProcessLightList()` llamaba a `TileHasAny(light.position.tile, TileProperties::Solid)` sin validar `InDungeonBounds()`.
  - En `gendung.h:298`, `TileHasAny` indexaba `dPiece[coords.x][coords.y]` y `SOLData[...]` sin chequeo de límites, provocando lectura fuera de la memoria lineal WASM (512 MB) y crash inmediato (`Uncaught RuntimeError: memory access out of bounds`).

#### B. Correcciones Aplicadas en el Núcleo
1. **Trigger Temprano y Reseteo Incondicional de Luces ([`Source/diablo.cpp`](file:///c:/Projects/DevilutionX/Source/diablo.cpp)):**
   - Se ejecuta `nightmare::invasion::InvasionManager::Get().CheckInvasionTrigger()` al inicio de `LoadGameLevel()`.
   - `InitLighting()` ahora se ejecuta siempre en toda transición de nivel (`lvldir != ENTRY_LOAD`), vaciando por completo `ActiveLights` antes de inicializar entidades en el nuevo nivel o pueblo.
2. **Blindaje Estricto en `TileHasAny` ([`Source/levels/gendung.h`](file:///c:/Projects/DevilutionX/Source/levels/gendung.h)):**
   - `TileHasAny()` ahora valida `if (!InDungeonBounds(coords)) return HasAnyOf(property, TileProperties::Solid);`. Garantiza que coordenadas fuera de límites nunca indexen `dPiece` ni `SOLData`.
3. **Validación Previa en `ProcessLightList` ([`Source/lighting.cpp`](file:///c:/Projects/DevilutionX/Source/lighting.cpp)):**
   - Se añadió `!InDungeonBounds(light.position.tile)` antes de invocar `TileHasAny`.
4. **Protección en Cursor ([`Source/cursor.cpp`](file:///c:/Projects/DevilutionX/Source/cursor.cpp)):**
   - Validación `static_cast<size_t>(monsterId) < MaxMonsters` antes de indexar `Monsters[monsterId]`.
5. **Cachebuster Actualizado a `v=nightmare-v11` ([`Packaging/emscripten/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/index.html)):**
   - Asegura la recarga inmediata de scripts en el navegador del usuario.

### 7. 🛡️ Erradicación de `table index is out of bounds` en WebAssembly & Preservación de Diagnóstico

#### A. Diagnóstico y Causa Raíz
- **Use-After-Free en el Callback de Sonido Duplicado ([`Source/engine/sound.cpp`](file:///c:/Projects/DevilutionX/Source/engine/sound.cpp)):**
  - Durante el combate, sonidos repetidos usan `DuplicateSound()`. En la implementación anterior, `result->SetFinishCallback([it](...) { duplicateSounds.erase(it); })` eliminaba el `SoundSample` desde *dentro* de la propia ejecución del callback de `Aulib::Stream`.
  - Al retornar de la lambda, el objeto `Stream` y su tabla de métodos virtuales estaban destruidos. La máquina de audio intentaba realizar un salto indirecto (`call_indirect` en WASM), lo que en WebAssembly lanzaba inmediatamente `RuntimeError: table index is out of bounds`.
- **Despacho Indirecto de Funciones sin Comprobación de Límites ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp), [`Source/tables/misdat.cpp`](file:///c:/Projects/DevilutionX/Source/tables/misdat.cpp), [`Source/tables/spelldat.h`](file:///c:/Projects/DevilutionX/Source/tables/spelldat.h)):**
  - `AiProc[monster.ai](monster)` se ejecutaba sin validar si `monster.ai` era negativo o sobrepasaba el tamaño de la tabla de IA.
  - `GetMissileData(missileId)` no comprobaba límites. Para `MissileID::Null` (-1) o IDs corruptos, accedía memoria no inicializada con punteros basura a `processFn` y `addFn`, provocando fallos en `call_indirect`.
  - `GetSpellData(spellId)` indexaba `SpellsData` sin validar el tamaño del vector.
- **Sobrescritura de la Pila de Llamadas por el Watchdog ([`Packaging/emscripten/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/index.html)):**
  - Al estrellarse el motor WebAssembly por una excepción, el modal capturaba la traza. Pero 6 segundos más tarde, el watchdog detectaba la falta de cuadros y llamaba a `triggerCrashReport('Congelamiento prolongado (>6s sin cuadros)', ...)`, sustituyendo la traza real por `null`.

#### B. Correcciones Aplicadas en el Núcleo
1. **Poda Segura de Sonidos Duplicados ([`Source/engine/sound.cpp`](file:///c:/Projects/DevilutionX/Source/engine/sound.cpp)):**
   - Se eliminó el callback que borraba el iterador desde dentro de `Stream`.
   - `DuplicateSound()` ahora poda de forma sincronizada con mutex los sonidos inactivos (`!(*it)->IsPlaying()`) al inicio antes de insertar nuevos elementos.
2. **Validación de Límites en Rutinas de IA ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp)):**
   - Comprobación estricta de índice y puntero nulo: `aiIndex >= 0 && aiIndex < std::size(AiProc) && AiProc[aiIndex] != nullptr`.
3. **Tablas Bounded para Proyectiles y Hechizos ([`Source/tables/misdat.cpp`](file:///c:/Projects/DevilutionX/Source/tables/misdat.cpp), [`Source/tables/spelldat.h`](file:///c:/Projects/DevilutionX/Source/tables/spelldat.h)):**
   - `GetMissileData()` y `GetSpellData()` ahora devuelven estructuras estáticas seguras vacías en lugar de acceder memoria fuera de límites.
4. **Protección Contra Sobrescritura en el Watchdog ([`Packaging/emscripten/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/index.html)):**
   - Bandera `hasFatalError` que impide al watchdog reemplazar un crash report con pila de llamadas real.
5. **Cachebuster Actualizado a `v=nightmare-v12` ([`Packaging/emscripten/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/index.html)):**
   - Actualizadas las referencias a `devilutionx.js?v=nightmare-v12` y `file-manager.js?v=nightmare-v12`.

### 8. ⏱️ Reloj por Defecto & Restauración Estética Original del Puente y Nido (Hive)

#### A. Reloj de Sesión por Defecto ([`Source/options.cpp`](file:///c:/Projects/DevilutionX/Source/options.cpp), [`Packaging/emscripten/emscripten_pre.js`](file:///c:/Projects/DevilutionX/Packaging/emscripten/emscripten_pre.js), [`Packaging/emscripten/devilutionx.js`](file:///c:/Projects/DevilutionX/Packaging/emscripten/devilutionx.js), [`Packaging/emscripten/file-manager.js`](file:///c:/Projects/DevilutionX/Packaging/emscripten/file-manager.js))
- En `ModOptions::ModEntry`, el estado habilitado por defecto se configuró en `name == "clock"`, permitiendo que el mod Lua nativo de reloj se active automáticamente tanto en PC como en Web sin requerir activación manual en menús o archivos de configuración.
- Los inicializadores web de `diablo.ini` aseguran la presencia de `clock=1` bajo la sección `[Mods]`.
- Muestra de manera permanente la hora y duración de la sesión en el HUD sin obstruir el juego.

#### B. Restauración del Puente y Entrada al Nido de Hellfire ([`Source/levels/town.cpp`](file:///c:/Projects/DevilutionX/Source/levels/town.cpp), [`Source/levels/town.h`](file:///c:/Projects/DevilutionX/Source/levels/town.h))
- Se retiró la función `TownOpenPeninsulaPassage()` que sustituía el capullo orgánico del Nido por un pasaje plano durante la invasión de Tristán.
- Dado que el jugador dispone de magia y teletransporte en el pueblo para sortear obstáculos y navegar libremente, se restauró al 100% el diseño, tiles, río, puente y la entrada original del Nido de Hellfire (Hive/Nest).
- Cachebuster web actualizado a `v=nightmare-v13` en [`Packaging/emscripten/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/index.html).

---

### 8. 🐛 Erradicación de `RuntimeError: unreachable` en WebAssembly (`doRewind` ASYNCIFY)

#### A. Diagnóstico y Causa Raíz
- **El error en producción:**
  ```
  RuntimeError: unreachable
      at devilutionx.wasm:wasm-function[882]:0x62f81
      at devilutionx.wasm:wasm-function[646]:0x3d260
      at ...
      at ret.<computed> (devilutionx.js:1:224923)
      at Object.doRewind (devilutionx.js:1:226463)
  ```
- **Mecanismo del fallo:**
  - En un commit anterior se configuró `-sASYNCIFY_IGNORE_INDIRECT=1` en `CMakeLists.txt` con la hipótesis de evitar colisiones en `__indirect_function_table`.
  - Sin embargo, en Emscripten, `ASYNCIFY_IGNORE_INDIRECT=1` desactiva deliberadamente la instrumentación de Asyncify para llamadas indirectas (`call_indirect`).
  - DevilutionX es un motor C++ que emplea profusamente llamadas indirectas en su bucle de ejecución (métodos virtuales, punteros a funciones, callbacks SDL y lambdas de render).
  - Al completar el primer fotograma (`RenderPresent -> Sleep -> emscripten_sleep(1)`), Asyncify desenrolló la pila. Al despertar, `Object.doRewind` intentó reconstruir el marco de ejecución a través de las tablas de reingreso; al encontrar funciones indirectas sin instrumentar, la máquina de estados cayó en `unreachable`.
  - Adicionalmente, los commits posteriores en C++ no se reflejaban porque el archivo `devilutionx.wasm` desplegado por GitHub Pages era el binario estático afectado.

#### B. Solución Aplicada y Despliegue
1. **Remoción de flags conflictivos:** Se retiró `-sASYNCIFY_IGNORE_INDIRECT=1` y `-sASSERTIONS=1` de [`CMakeLists.txt`](file:///c:/Projects/DevilutionX/CMakeLists.txt).
2. **Restauración de Binario Canónico:** Se restableció [`Packaging/emscripten/devilutionx.wasm`](file:///c:/Projects/DevilutionX/Packaging/emscripten/devilutionx.wasm) (6.647.125 bytes) con soporte completo de llamadas indirectas para ASYNCIFY.
3. **Invalidación de Caché:** Se actualizó el cachebuster a `v=nightmare-v19` en [`Packaging/emscripten/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/index.html).
### 9. 🛡️ Erradicación de `SDL Error: Parameter 'width' is invalid` al Inicializar la Ventana

#### A. Diagnóstico y Causa Raíz
- **El error:** Al cargar el motor en el navegador, un alert emergente detenía el juego con:
  ```
  SDL Error
  Parameter 'width' is invalid
  The error occurred at: Source\utils\sdl_wrap.h line 52
  ```
- **Mecanismo del fallo:**
  - En WebAssembly, `fitToScreen` está habilitado por defecto (`Graphics.fitToScreen = true`).
  - Al iniciar, `GetPreferredWindowSize()` invoca `CalculatePreferredWindowSize(width, height)`.
  - Esta función llama a `SDL_GetDesktopDisplayMode(0, &mode)`. Si el canvas en el navegador aún no tiene dimensiones finales o si `diablo.ini` no contiene `[Graphics]`, las operaciones de aspecto o `integerScaling` resultaban en `width <= 0`.
  - Con `width <= 0`, `CreateBackBuffer()` llamaba a `CreateRGBSurfaceWithFormat(0, gnScreenWidth, gnScreenHeight, ...)`.
  - La aserción de SDL2 `if (width <= 0) SDL_InvalidParamError("width")` disparaba fatalmente `ErrDlg()`.

#### B. Solución Aplicada
1. **Blindaje en capa JS (`emscripten_pre.js`, `file-manager.js`, `devilutionx.js`):**
   - Se asegura automáticamente en `diablo.ini` la presencia de `[Graphics]` con `Width=640`, `Height=480`, `Fit to Screen=0` y `Upscale=1`.
   - Previene que `CalculatePreferredWindowSize` intente recalcular el modo de pantalla en WebAssembly sobre un viewport aún no estabilizado.
2. **Guardas defensivas en C++ (`sdl_wrap.h`, `display.cpp`, `dx.cpp`, `options.cpp`):**
   - En `sdl_wrap.h`, `CreateRGBSurface` y `CreateRGBSurfaceWithFormat` garantizan `width >= 640` y `height >= 480`.
   - En `display.cpp`, `CalculatePreferredWindowSize` acota valores mínimos y previene divisiones por cero en `factor` y `mode.h`.
   - En `display.cpp`, `AdjustToScreenGeometry` asegura `gnScreenWidth = std::max(640, windowSize.width)` y `gnScreenHeight = std::max(480, windowSize.height)`.
   - En `dx.cpp`, `CreateBackBuffer()` garantiza dimensiones mínimas de 640x480.
   - En `options.cpp`, `OptionEntryResolution::LoadFromIni` valida que los anchos y altos leídos de disco sean mayores a cero.
3. **Invalidación de Caché:**
   - Actualizado a `v=nightmare-v20` en [`Packaging/emscripten/index.html`](file:///c:/Projects/DevilutionX/Packaging/emscripten/index.html).

---

## 🧪 Resultados de Verificación
- **Zero Crashes por `unreachable`:** Rebobinado de Asyncify con soporte indirecto completo restaurado.
- **Zero Errores de SDL `width`:** Blindaje de inicialización en JS y C++ asegurando `Width=640`, `Height=480`, `Fit to Screen=0` por defecto.
- **Reloj de Partida Activo:** Reloj en tiempo real visualizado a 60 FPS en el HUD.
- **Cachebuster Actualizado:** `v=nightmare-v20` activo.


