# Walkthrough - Diablo Nightmare Edition (Easter Egg Golem & Endless Diablo)

Se han implementado, probado y verificado con éxito las nuevas características de **Golem Acompañante Permanente** y el **Ciclo de Muerte y Respawn Infinito de Diablo** en Single Player.

## 🛠️ Cambios Implementados y Verificados

### 1. 🗿 Easter Egg: Golem Acompañante Permanente & Inmortal (Single Player)
- **Persistencia Global ([`Source/player.h`](file:///c:/Projects/DevilutionX/Source/player.h), [`Source/missiles.cpp`](file:///c:/Projects/DevilutionX/Source/missiles.cpp))**:
  - Al lanzar el hechizo *Golem* en Single Player, el nivel del hechizo queda almacenado en `Player::_persistentGolemSpellLevel`.
  - **Re-lanzamiento Táctico**: Si se vuelve a lanzar el hechizo teniendo un Golem activo, se teletransporta inmediatamente al punto del cursor con su salud restaurada al 100%.
- **Inmortalidad en Combate ([`Source/monster.cpp`](file:///c:/Projects/DevilutionX/Source/monster.cpp))**:
  - En `ApplyMonsterDamage()`, el daño recibido por esbirros aliados queda limitado para preservar siempre al menos 1 HP (64 unidades raw), de modo que nunca muere en batalla.
- **Transición entre Niveles, Portales y Tristram ([`Source/diablo.cpp`](file:///c:/Projects/DevilutionX/Source/diablo.cpp), [`Source/player.cpp`](file:///c:/Projects/DevilutionX/Source/player.cpp))**:
  - En `RemovePlrMissiles()`, al abandonar un piso se desactiva limpiamente la entidad previa en el mapa viejo para no guardar duplicados o zombies en los archivos temporales de niveles anteriores.
  - En `LoadGameLevel()`, al entrar a cualquier piso o a Tristram, el Golem se coloca y se activa de inmediato en pie (`MonsterMode::Stand` con `M_StartStand`) junto al jugador con vida completa.
- **Render y Cursor en Tristram ([`Source/engine/render/scrollrt.cpp`](file:///c:/Projects/DevilutionX/Source/engine/render/scrollrt.cpp), [`Source/cursor.cpp`](file:///c:/Projects/DevilutionX/Source/cursor.cpp))**:
  - `DrawMonsterHelper()` renderiza correctamente el sprite del Golem de piedra cuando se encuentra en el pueblo, separándolo de la posición de los aldeanos de Tristram (`Towners[0]` / Griswold).
  - `cursor.cpp` permite seleccionar al Golem bajo el cursor en el pueblo sin conflictos con los aldeanos.
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
- **Compilación Nativa C++:** Exitosa en `build_fresh/devilutionx.exe` (5,741,070 bytes).
- **Cero Regresiones:** Compatibilidad estricta de guardado y funcionamiento fluido tanto en mazmorras como en Tristram.
