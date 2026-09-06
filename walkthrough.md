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

## 🧪 Resultados de Verificación
- **Compilación Nativa C++:** Exitosa en `build_COMPILE_FRESH/devilutionx.exe`.
- **Cero Regresiones:** Compatibilidad estricta de guardado, persistencia del Golem entre sesiones y estabilidad completa al cargar partidas.
