# NIGHTMARE: ASHENMOOR — Sega Genesis / Mega Drive ARPG Demo

Demo jugable oficial de 16-bit para **Sega Genesis / Mega Drive**.

- **Perspectiva:** Top-Down 3/4 Oblicua (60 FPS fluidos)
- **Protagonista:** La Hechicera / Arcanista (mujer con túnica violeta, báculo arcano y cabellera dorada)
- **Niveles:** 3 Niveles Subterráneos + Jefe Final (*The Flayer*)
- **Formato:** ROM nativa de Sega Genesis (`.smd` y `.bin`)
- **Estado:** Prototipo funcional preservado de forma independiente. Desarrollo a gran escala pausado para priorizar el desarrollo del motor principal DevilutionX. Cero dependencias o modificaciones sobre el codebase de DevilutionX.

---

## 🎮 Controles (Gamepad de Genesis)

| Botón | Acción | Descripción |
|---|---|---|
| **D-Pad (Cruceta)** | Moverse | Movimiento fluido en 8 direcciones con animación de pasos. |
| **Botón B** | Centella Arcana (*Arcane Bolt*) | Dispara un proyectil mágico en la dirección hacia donde mira. Consume **4 MP**. |
| **Botón C** | Nova Arcana (*Arcane Burst*) | Explosión mágica circular que daña a todos los monstruos cercanos. Consume **18 MP**. |
| **Botón A** | Beber Poción | Si tienes pociones en el inventario, restaura **45 HP** al instante. |
| **START** | Reiniciar / Menú | Reinicia la partida desde el estado de Victoria o Muerte. |

---

## 🏰 Estructura de la Demo (3 Niveles)

1. **Nivel 1: Las Catacumbas Cenicentas (*Ashen Catacombs*)**  
   Paredes de piedra antigua y antorchas encendidas. Esqueletos patrullan los pasillos. Encuentra las escaleras de piedra para descender al Nivel 2.
2. **Nivel 2: La Cripta de los Martirizados (*Crypt of the Martyrs*)**  
   Laberinto de criptas con pilares y charcos de sangre. Los enemigos incluyen Necrófagos rápidos y esqueletos armados. Baja al Sanctum.
3. **Nivel 3: El Sanctum de la Agonía (*Sanctum of Agony*)**  
   Arena ceremonial con runas rojas en el suelo.  
   **JEFE FINAL: *The Flayer* (El Despellejador de Almas)**:
   - Sprite gigante de 32x32 píxeles con cuernos de hierro y cuchilla.
   - Persecución implacable y 180 HP.
   - ¡Derrótalo para completar la demo!

---

## 🕹️ Ubicación de la ROM

La ROM ha sido compilada y depositada en las siguientes ubicaciones para tu comodidad:
- **Carpeta del emulador Gens:** [`c:/Projects/DevilutionX/gens-win32-bin-2.14/nightmare_demo.smd`](file:///c:/Projects/DevilutionX/gens-win32-bin-2.14/nightmare_demo.smd)
- **Carpeta de tu colección de ROMs:** [`I:/Juegos/Hours of Fun/Roms/Sega/nightmare_demo.smd`](file:///I:/Juegos/Hours%20of%20Fun/Roms/Sega/nightmare_demo.smd)
- **Código fuente y proyecto:** [`c:/Projects/DevilutionX/nightmare_sega/`](file:///c:/Projects/DevilutionX/nightmare_sega/)

Para compilar de nuevo en cualquier momento:
```bash
python c:\Projects\DevilutionX\nightmare_sega\build.py
```
