# Solución Técnica Definitiva: PC Vivaz / Educational Computer 2000

## 1. Diagnóstico del Problema Original

Al ejecutar la ROM de **PC Vivaz / Educational Computer 2000** (`pcvivaz-unif.nes` / `pcvivaz.nes`) en emuladores genéricos compilados para Web/Libretro (como Nestopia o FCEUX en WASM):
* La CPU 6502 y la PPU arrancan correctamente, inicializan la memoria y renderizan la interfaz gráfica (GUI) con las aplicaciones (*Ordenador Educativo*, *PC Escribiendo*, *Hoja Mágica* y el sprite del puntero del ratón).
* **El cursor del ratón queda completamente congelado** y el teclado físico no responde.

### Causa Raíz a Nivel de Hardware y Código 6502:
Al realizar la desensamblación directa del bus de entrada en el Banco 0 (`pcvivaz.prg`), identificamos la rutina exacta del bucle principal de interrupción en `$8E29-$8E34`:

```assembly
$8E29: JSR $914C       ; Llama a la rutina de lectura serie del ratón
$8E2C: LDA $B5         ; Comprueba el contador de estado del paquete (0..2)
$8E2E: BNE $8E29       ; ¡Bucle hasta completar los 3 paquetes del ratón ($B5 == 0)!
$8E30: LDA $B4         ; Estado de conexión del ratón
$8E32: BNE +3          ; Si el ratón está conectado, procesa el ratón
$8E34: JSR $938A       ; De lo contrario, lee la matriz del teclado Subor
```

1. **Protocolo Propietario de 3 Paquetes del Ratón PC Vivaz**: No utiliza el protocolo estándar SNES Mouse de 32 bits ni el Zapper. Envía una secuencia serie de 3 bytes (un paquete por cada ciclo de sondeo) que identifica cada fase mediante los bits 0 y 1 (`0x01`, `0x02`, `0x03`). Si el emulador devuelve ceros (`0x00`), `$B5` nunca progresa a 1 o 2, o se bloquea esperando los bits de sincronía.
2. **Multiplexación de Matriz de Teclado Subor en `$4016` / `$4017`**: El teclado utiliza una matriz de 9 filas multiplexadas escribiendo `$05` (reset de fila), `$04` (lectura de nibble bajo en `$4017` bits D1..D4) y `$06` (lectura de nibble alto en `$4017` bits D1..D4 e incremento de fila).
3. **Desconexión en cores Libretro Web**: Los puertos `$4016` y `$4017` en los cores WASM genéricos solo enlazaban el gamepad estándar (8 bits serie) y descartaban los strobes de multiplexación de matriz y los paquetes serie del ratón PC Vivaz.

---

## 2. Protocolo de Hardware del Ratón PC Vivaz

### Ciclo de Disparo y Lectura Serie:
1. **Latch**: La ROM escribe `$01` en `$4016` (Strobe activo).
2. **Retardo**: Ejecuta 14 instrucciones `NOP` (28 ciclos de reloj de CPU).
3. **Clock / Latch Low**: Escribe `$06` en `$4016`.
4. **Lectura de 8 Bits**: Ejecuta 8 lecturas consecutivas de `$4017` en un bucle `ROR A -> ROL $B1` (transmisión serie MSB a LSB a través del **Bit 0 de `$4017`**).

### Estructura de los 3 Bytes del Ratón:
* **Paquete 1 (`$B6` en ROM)**:
  * Bits `0..1`: `0x01` (Identificador del Paquete 1)
  * Bit 7: Clic Izquierdo (1 = Pulsado)
  * Bit 6: Clic Derecho (1 = Pulsado)
  * Bit 5: Signo de $\Delta X$ (1 = Negativo / Izquierda)
  * Bit 4: Magnitud alta de $\Delta X$ ($16)
  * Bit 3: Signo de $\Delta Y$ (1 = Negativo / Arriba)
  * Bit 2: Magnitud alta de $\Delta Y$ ($16)
* **Paquete 2 (`$B7` en ROM)**:
  * Bits `0..1`: `0x02` (Identificador del Paquete 2)
  * Bits `2..5`: Magnitud baja (4 bits) de $\Delta X$: `(abs(deltaX) & 0x0F) << 2`
* **Paquete 3 (`$B8` en ROM)**:
  * Bits `0..1`: `0x03` (Identificador del Paquete 3)
  * Bits `2..5`: Magnitud baja (4 bits) de $\Delta Y$: `(abs(deltaY) & 0x0F) << 2`

---

## 3. Protocolo de Matriz de Teclado Subor

En `$938A`, la ROM escanea 9 filas de teclas:
1. Escribe `$05` a `$4016` para resetear el índice de fila a 0.
2. Para cada fila (0 a 8):
   * Escribe `$04` a `$4016` y lee `$4017` -> `(LSR A) & 0x0F` (Nibble bajo en D1..D4).
   * Escribe `$06` a `$4016` y lee `$4017` -> `(ROL A 3 veces) & 0xF0` (Nibble alto en D1..D4) y avanza la fila.
   * Guarda el byte de 8 teclas en la página cero (`$20` a `$28`).

### Mapa de Filas:
* **Fila 0**: `4`, `G`, `F`, `C`, `F2`, `E`, `5`, `V`
* **Fila 1**: `2`, `D`, `S`, `End`, `F1`, `W`, `3`, `X`
* **Fila 2**: `Insert`, `Backspace`, `PageDown`, `Flecha Derecha`, `F8`, `PageUp`, `Delete`, `Home`
* **Fila 3**: `9`, `I`, `L`, `,`, `F5`, `O`, `0`, `.`
* **Fila 4**: `]`, `Enter`, `Flecha Arriba`, `Flecha Izquierda`, `F7`, `[`, `\`, `Flecha Abajo`
* **Fila 5**: `Q`, `CapsLock`, `Z`, `Tab`, `Escape`, `A`, `1`, `Ctrl`
* **Fila 6**: `7`, `Y`, `K`, `M`, `F4`, `U`, `8`, `J`
* **Fila 7**: `-`, `;`, `'`, `/`, `F6`, `P`, `=`, `Shift`
* **Fila 8**: `R`, `T`, `H`, `N`, `F3`, `Espacio`, `6`, `B`

---

## 4. Arquitectura de Memoria (NES 2.0 Mapper 329 / UNL-EDU2000)

* **PRG-ROM**: 1024 KB (32 bancos de 32 KB intercambiables en `$8000-$FFFF`).
* **PRG-RAM (SRAM con batería)**: 32 KB (4 bancos de 8 KB en `$6000-$7FFF`).
* **CHR-RAM**: 8 KB / 16 KB unbanked.
* **Registro de Control (`$8000-$FFFF`)**:
  * Bits `0..4`: Banco de PRG-ROM (0 a 31).
  * Bit 5: Mirroring (0 = Vertical, 1 = Horizontal).
  * Bits `6..7`: Banco de PRG-RAM (0 a 3).

---

## 5. Implementación y Cómo Ejecutarlo

Hemos añadido a la carpeta `PCVIVAZ/` todo el ecosistema listo para funcionar sin instalaciones adicionales:

1. **`index.html`**: Interfaz gráfica moderna y retro con monitor CRT, soporte para captura de ratón (Pointer Lock API), filtros gráficos, diagnósticos en tiempo real y gestor de guardado de memoria `.srm`.
2. **`pcvivaz_core.js`**: Núcleo de emulación NES completo (CPU 6502, PPU, APU y periféricos) con emulación exacta del ratón y teclado de PC Vivaz.
3. **`rom_data.js`**: ROMs (`pcvivaz-unif.nes`, `pcvivaz.nes`) y memoria de guardado inicial (`pcvivaz-unif.srm`) embebidos para ejecución inmediata.
4. **`styles.css`**: Hoja de estilos con efectos de fósforo CRT, biseles retro y diseño responsivo.
5. **`run_pcvivaz.bat` / `server.py`**: Lanzador local con un solo clic.

### Para Iniciar:
* Haz doble clic en `run_pcvivaz.bat` (o ejecuta `python server.py`).
* Se abrirá automáticamente en tu navegador web.
* Haz clic sobre la pantalla para capturar el ratón físico de tu ordenador y controlar el puntero en la pantalla de PC Vivaz de forma fluida.
