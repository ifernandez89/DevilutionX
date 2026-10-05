# 🖥️ Arquitectura de Hardware, Mapeo y Emulación: Mini PC Vivaz (Ordenador Educativo)

> **Documento Técnico de Referencia**  
> **Sistema:** Nintendo Entertainment System / Famicom Educational Clone (Famiclón)  
> **Placa y Mapper:** UNL-EDU2000 / NES 2.0 Mapper 329  
> **Plataforma de Integración:** RetroHub NES WASM & Mesen Desktop  

---

## 1. Introducción y Contexto Histórico

La **Mini PC Vivaz** (*Ordenador Educativo 2000 / Educational Computer 2000*) no es un videojuego de consola tradicional, sino un **Famiclón educativo** basado en la arquitectura de 8 bits del procesador Ricoh 2A03 (compatible con MOS 6502).

A diferencia de una consola Famicom o NES estándar de sobremesa:
- La placa base se encontraba integrada dentro del chasis de un **teclado QWERTY completo de 101 teclas**.
- Incluía un **ratón serial** conectado al puerto auxiliar.
- Su software interno ejecuta un entorno gráfico tipo escritorio con procesador de textos (*PC Escribiendo*), hoja de cálculo (*Hoja Mágica*), base de datos de contactos (*Libros de Amigos*) y programas de mecanografía (*Juega a Tipear*).

---

## 2. Diferencias Fundamentales de Hardware: Consola vs. Ordenador Educativo

| Componente | Consola NES Clásica (Super Mario, etc.) | Mini PC Vivaz (Ordenador Educativo) |
| :--- | :--- | :--- |
| **Tipo de Dispositivo** | Mando de juego (RetroPad / Joypad) | Teclado Matricial QWERTY completo + Ratón serial |
| **Protocolo de Entrada** | Registro serie de 8 bits en `$4016` (1 bit por ciclo de reloj) | Matriz de 13 filas escaneadas mediante estrobos en `$4016` / `$4017` |
| **Botones Totales** | 8 botones físicos (Up, Down, Left, Right, B, A, Select, Start) | Más de 80 teclas individuales (alfanuméricas, signos, enter, backspace) |
| **Puntero en Pantalla** | No disponible (salvo juegos con pistola Zapper o Arkanoid) | Ratón Famicom de 2 botones con coordenadas de desplazamiento delta |
| **Memoria de Trabajo** | 2 KB de RAM interna en CPU | 32 KB de PRG-RAM paginada en `$6000-$7FFF` con respaldo de batería |
| **Memoria Gráfica** | 8 KB CHR-ROM fija o CHR-RAM | 8 KB CHR-RAM dinámica |

### ¿Por qué los juegos estándar sí reconocían tu teclado físico?
En los juegos convencionales, el emulador simplemente asocia tus teclas a los 8 bits del joystick (Flechas $\rightarrow$ D-Pad, Z/X $\rightarrow$ B/A, Enter $\rightarrow$ Start). El juego cree que hay un joystick enchufado y responde.

### ¿Por qué la PC Vivaz no respondía?
El sistema operativo de la PC Vivaz **ignora por completo el protocolo serie de joystick**. En su lugar, interroga continuamente las líneas de la matriz Subor Keyboard a través del puerto de expansión de Famicom. Si el emulador no simula el chip decodificador de teclado matricial, la ROM asume que no hay teclado conectado.

---

## 3. El Problema del Formato UNIF en la Web

La ROM suele distribuirse como `pcvivaz-unif.nes` en formato de contenedor **UNIF** (Universal NES Image Format):
- En UNIF, los metadatos residen en bloques de texto (`MAPR: UNL-EDU2000`, `PRG0`, `BATR`).
- Los motores Libretro en WebAssembly (tanto Nestopia como FCEUmm) inicializan las ROMs UNIF asignando controladores genéricos (`SI_GAMEPAD`) y dejan el puerto de expansión desconectado (`SIFC_NONE`).
- Nestopia Libretro **no incluye código de teclado matricial Subor** en su capa de comunicación frontend.
- FCEUmm sí incluye el soporte (`SIFC_SUBORKB`), pero requiere que la cabecera del cartucho instruya explícitamente al emulador para activar el puerto de expansión.

---

## 4. Solución Implementada en la Plataforma Web (RetroHub)

Para lograr compatibilidad 100% transparente sin requerir que el usuario modifique manualmente su archivo, implementamos:

### A. Transmutador Automático UNIF $\rightarrow$ NES 2.0 en Memoria
Al arrastrar o seleccionar la ROM en el navegador, una rutina en JavaScript intercepta el buffer y analiza si contiene la firma `UNIF` con placa `UNL-EDU2000`:
1. Extrae los bancos de programa `PRG0` y las banderas de memoria.
2. Construye la cabecera estándar de 16 bytes de **NES 2.0**:

```
Byte 0..3:   0x4E 0x45 0x53 0x1A   -> 'NES\x1A'
Byte 4:      PRG-ROM tamaño (unidades de 16 KB)
Byte 5:      0x00                  -> 8 KB CHR-RAM (sin CHR-ROM)
Byte 6:      0x92                  -> Mapper bajo (9) + Batería SRAM (2)
Byte 7:      0x48                  -> Mapper medio (4) + Firma NES 2.0 (0x08)
Byte 8:      0x01                  -> Mapper alto (1) -> Mapper 329 (0x149)
Byte 9:      0x00                  -> Bits superiores de tamaño
Byte 10:     0x90                  -> 32 KB PRG-RAM respaldada con batería ($6000)
Byte 11:     0x07                  -> 8 KB CHR-RAM
Byte 12:     0x01                  -> Región PAL / Dendy (Modo Famiclón nativo)
Byte 13:     0x00                  -> Consola estándar
Byte 14:     0x00                  -> Sin ROMs misceláneas
Byte 15:     0x27                  -> Default Expansion Device: Subor Keyboard + Mouse
```

El **Byte 15 con valor `0x27`** es el estándar de NESdev para ordenar al emulador enlazar de inmediato el teclado Subor y el ratón.

### B. Enrutamiento al Motor FCEUmm con Puertos Libretro
Para el modo PC Vivaz, el reproductor selecciona automáticamente **FCEUmm** y define la configuración de hardware:
- `input_libretro_device_p1: '1'` (Mando 1)
- `input_libretro_device_p2: '2'` (Ratón serial Famicom)
- `input_libretro_device_p5: '1'` (Puerto de expansión Famicom)
- `fceumm_zapper_mode: 'mouse'`
- `fceumm_mouse_sensitivity: '100'`

### C. Captura de Ratón (Pointer Lock) y Teclado Físico
- Al hacer clic en la pantalla, se activa la `Pointer Lock API` del navegador para sincronizar el puntero del mouse de la PC con el cursor en pantalla del sistema operativo educativo. Presionar `ESC` libera el cursor.
- Todas las pulsaciones de teclado físico (letras A-Z, números 0-9, Enter, Espacio, etc.) se despachan al emulador mediante `currentEmulator.keyboardDown(e.code)`.

### D. Botón de Descarga NES 2.0
El banner interactivo de **Modo Mini PC Vivaz Activo** incorpora el botón:
`💾 Descargar NES 2.0 (.nes)`  
Permite exportar al instante la ROM convertida para usarla localmente en cualquier emulador de escritorio.

---

## 5. Guía de Ejecución en Emuladores de Escritorio (Mesen)

Si deseas ejecutar la ROM convertida en tu computadora fuera del navegador:
1. Descarga el emulador **Mesen** (el más avanzado y compatible con Mapper 329).
2. Abre el archivo `.nes` generado.
3. Ve a `Options` $\rightarrow$ `Input`:
   - En **Expansion Port**, verifica que esté seleccionado **Subor Keyboard + Mouse**.
4. Tu teclado físico y mouse funcionarán de manera instantánea y cycle-accurate.
