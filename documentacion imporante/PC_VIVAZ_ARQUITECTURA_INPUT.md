# 🏗️ PC Vivaz — Arquitectura de Input (Diseño Técnico)

## 📐 Visión General

Este documento detalla la arquitectura completa del sistema de entrada para la PC Vivaz / Educational Computer 2000, desde el navegador hasta la ROM 6502.

---

## 🔄 Stack Completo de Input

### Capa 1: Hardware del Usuario
```
┌─────────────────────────────────────┐
│ Teclado Físico USB/Bluetooth        │
│ - 104 teclas estándar               │
│ - Drivers del OS (Windows/Linux)    │
└────────────┬────────────────────────┘
             ↓
```

### Capa 2: Navegador Web
```
┌─────────────────────────────────────┐
│ Browser Event System                │
│ - KeyboardEvent (keydown/keyup)     │
│ - Properties:                       │
│   • keyCode: 40 (numeric)           │
│   • which: 40 (alias de keyCode)    │
│   • code: "ArrowDown" (physical)    │
│   • key: "ArrowDown" (logical)      │
└────────────┬────────────────────────┘
             ↓
```

### Capa 3: JavaScript Application Layer
```
┌─────────────────────────────────────┐
│ app.js Event Handlers               │
│                                     │
│ window.addEventListener('keydown')  │
│ ├─ Global hotkeys (F5/F7/P/R)      │
│ ├─ KEY_ACTIONS mapping              │
│ └─ Conditional preventDefault()     │
│                                     │
│ Decision:                           │
│ • Fullscreen + Arrow → prevent     │
│ • Otherwise → pass through          │
└────────────┬────────────────────────┘
             ↓
```

### Capa 4: Emscripten Canvas
```
┌─────────────────────────────────────┐
│ HTML5 Canvas Element                │
│ - Receives native DOM events        │
│ - Forwards to WASM module           │
│ - Focus management                  │
└────────────┬────────────────────────┘
             ↓
```

### Capa 5: WebAssembly Input Driver
```
┌─────────────────────────────────────┐
│ rwebinput (RetroArch Web Input)     │
│                                     │
│ Functions:                          │
│ • rwebinput_keyboard_callback()     │
│ • rwebinput_key_down()              │
│ • rwebinput_key_up()                │
│                                     │
│ Translation:                        │
│ keyCode → Libretro keycode enum     │
└────────────┬────────────────────────┘
             ↓
```

### Capa 6: Libretro API
```
┌─────────────────────────────────────┐
│ Libretro Input Abstraction          │
│                                     │
│ Device Ports:                       │
│ • Port 1: RETRO_DEVICE_JOYPAD (1)   │
│ • Port 2: RETRO_DEVICE_MOUSE (2)    │
│ • Port 5: RETRO_DEVICE_FC_SUBORKB   │
│           = 1539 = ((5+1)<<8)|3     │
│                                     │
│ Function:                           │
│ • retro_input_state_callback()      │
└────────────┬────────────────────────┘
             ↓
```

### Capa 7: Emulator Core (FCEUmm)
```
┌─────────────────────────────────────┐
│ FCEUmm Libretro Core                │
│                                     │
│ Input Systems:                      │
│ • SI_GAMEPAD (standard controller)  │
│ • SI_ZAPPER (light gun)             │
│ • SI_ARKANOID (paddle)              │
│ • SI_FC_SUBORKB (keyboard matrix)   │
│                                     │
│ Initialization:                     │
│ if (port5 == 1539) {                │
│   FCEU_InitSuborKeyboard()          │
│ }                                   │
└────────────┬────────────────────────┘
             ↓
```

### Capa 8: Hardware Virtual
```
┌─────────────────────────────────────┐
│ Subor Keyboard Matrix Emulation     │
│                                     │
│ Matrix: 13 rows × 8 columns         │
│ Registers:                          │
│ • $4016 (read): Row data            │
│ • $4016 (write): Row select strobe  │
│ • $4017 (read): Extended data       │
│                                     │
│ Protocol:                           │
│ 1. Write $05/$04/$06 to $4016       │
│ 2. Read nibbles from $4016/$4017    │
│ 3. Reconstruct 8-bit row state      │
└────────────┬────────────────────────┘
             ↓
```

### Capa 9: ROM 6502 Code
```
┌─────────────────────────────────────┐
│ PC Vivaz System ROM                 │
│                                     │
│ Keyboard Scanner Routine:           │
│ • Address: $8000-$8050              │
│ • Reads 13 rows sequentially        │
│ • Builds keymap in RAM ($0600)      │
│                                     │
│ Application Code:                   │
│ • Hoja Mágica (text editor)         │
│ • PC Escribiendo (typing tutor)     │
│ • Mecanografía (keyboard trainer)   │
└────────────┬────────────────────────┘
             ↓
       USER SEES RESULT
```

---

## 🔑 Identificadores de Dispositivos Libretro

### Cálculo del Device ID para Puerto de Expansión

```c
// Formato Libretro:
// device_id = ((port + 1) << 8) | subclass

// Para Subor Keyboard en puerto 5:
port = 5
subclass = 3  // FC_SUBORKB

device_id = ((5 + 1) << 8) | 3
          = (6 << 8) | 3
          = (6 * 256) | 3
          = 1536 | 3
          = 1539
```

### Tabla de Device IDs

| Device | Port | Subclass | Valor Dec | Valor Hex | Uso |
|--------|------|----------|-----------|-----------|-----|
| JOYPAD | 1 | 0 | 1 | 0x01 | Gamepad estándar |
| MOUSE | 2 | 0 | 2 | 0x02 | Mouse NES/Famicom |
| FC_SUBORKB | 5 | 3 | 1539 | 0x603 | Teclado Subor |
| FC_FAMILYKB | 5 | 2 | 1538 | 0x602 | Family Keyboard |
| ZAPPER | 1 | 0 | 6 | 0x06 | Light gun |

### Por qué 1539 es Crítico

```javascript
// ❌ MALO: Device ID genérico
input_libretro_device_p5: '1'
→ FCEUmm interpreta como RETRO_DEVICE_JOYPAD
→ No inicializa hardware de teclado
→ Puerto de expansión queda en SIFC_NONE
→ Inputs no llegan a la ROM

// ✅ BUENO: Device ID específico
input_libretro_device_p5: '1539'
→ FCEUmm reconoce RETRO_DEVICE_FC_SUBORKB
→ Inicializa matriz de 13×8
→ Puerto de expansión = SIFC_FC_SUBORKB
→ Inputs llegan correctamente
```

---

## 🚦 Manejo de preventDefault()

### Problema Original

```javascript
// ❌ ANTES: Bloqueaba todos los eventos
window.addEventListener('keydown', (e) => {
    const action = KEY_ACTIONS[e.code];
    if (action) {
        e.preventDefault();  // 💥 Mata el evento completamente
        currentEmulator.pressDown(action);
    }
});
```

**Consecuencia:**
1. JavaScript intercepta `keydown`
2. `preventDefault()` cancela propagación
3. Canvas nunca recibe el evento
4. rwebinput no ve nada
5. ROM no recibe input

### Solución Implementada

```javascript
// ✅ DESPUÉS: preventDefault selectivo
window.addEventListener('keydown', (e) => {
    const action = KEY_ACTIONS[e.code];
    if (action) {
        // Solo prevenir si:
        // - Es una flecha (Arrow*)
        // - Y estamos en fullscreen
        if (e.key.startsWith('Arrow') && document.fullscreenElement) {
            e.preventDefault();
        }
        // El evento sigue su curso natural al canvas
        currentEmulator.pressDown(action);
    }
});
```

**Beneficios:**
1. Eventos llegan al canvas con todos sus atributos
2. rwebinput recibe `keyCode` y `which` completos
3. No hay side effects de scroll inesperado en fullscreen
4. Funcionalidad de página preservada en modo ventana

### Tabla de Decisión

| Condición | Tecla | Fullscreen | Acción |
|-----------|-------|------------|--------|
| Navegación | ↑↓←→ | ✅ Sí | preventDefault() + forward |
| Navegación | ↑↓←→ | ❌ No | Forward only |
| Hotkey | F5/F7 | N/A | preventDefault() + handler |
| Hotkey | P/R | N/A | preventDefault() + handler |
| Tipeo | A-Z | N/A | Forward only |
| Tipeo | 0-9 | N/A | Forward only |
| Especial | Space/Enter/Tab | N/A | Forward only |

---

## 🎹 Matriz del Teclado Subor

### Estructura Física

```
     Columna 0  1  2  3  4  5  6  7
Fila ───────────────────────────────
 0   │  ESC   1  2  3  4  5  6  7
 1   │  8     9  0  -  =  \  [  ]
 2   │  Tab   Q  W  E  R  T  Y  U
 3   │  I     O  P  Ent Cap A  S  D
 4   │  F     G  H  J  K  L  ;  '
 5   │  Spc   ,  .  /  Shf Ctl Alt Prt
 6   │  `     Ins Del Home End PgUp PgDn ↑
 7   │  ←     ↓  →  Num /  *  -  +
 8   │  Num0  1  2  3  4  5  6  7
 9   │  Num8  9  .  Ent F1 F2 F3 F4
 10  │  F5    F6 F7 F8 F9 F10 F11 F12
 11  │  BkSp  - - - - - - -
 12  │  (Reservado para extensiones)
```

### Protocolo de Lectura

```c
// Pseudocódigo de ROM PC Vivaz
void scan_keyboard() {
    for (row = 0; row < 13; row++) {
        // Seleccionar fila
        strobe_value = row < 5 ? 0x05 :
                       row < 9 ? 0x04 : 0x06;
        WRITE($4016, strobe_value);
        
        // Leer datos
        low_nibble = READ($4016) & 0x0F;
        high_nibble = READ($4017) & 0x0F;
        row_data = (high_nibble << 4) | low_nibble;
        
        // Guardar en RAM
        keyboard_map[row] = row_data;
    }
}
```

### Mapeo de Teclas Importantes

| Tecla | Fila | Columna | Bit | Máscara | Uso en PC Vivaz |
|-------|------|---------|-----|---------|-----------------|
| Esc | 0 | 0 | 0 | 0x01 | Salir / Cancelar |
| Enter | 3 | 3 | 3 | 0x08 | Confirmar / Nueva línea |
| Tab | 2 | 0 | 0 | 0x01 | Cambiar campo |
| Space | 5 | 0 | 0 | 0x01 | Espacio |
| ↑ | 6 | 7 | 7 | 0x80 | Cursor arriba |
| ↓ | 7 | 1 | 1 | 0x02 | Cursor abajo |
| ← | 7 | 0 | 0 | 0x01 | Cursor izquierda |
| → | 7 | 2 | 2 | 0x04 | Cursor derecha |
| Backspace | 11 | 0 | 0 | 0x01 | Borrar carácter |

---

## 🔌 Integración con Emscripten

### Métodos del Emulador

```javascript
// Interfaz expuesta por RetroArch WASM
interface EmulatorAPI {
    // Métodos de control de botones (gamepad)
    pressDown(action: string): void;
    pressUp(action: string): void;
    
    // Métodos de teclado (raw)
    keyboardDown(code: string): void;
    keyboardUp(code: string): void;
    
    // Métodos de sistema
    pause(): void;
    resume(): void;
    reset(): void;
    saveState(): ArrayBuffer;
    loadState(data: ArrayBuffer): void;
}
```

### Flujo de Llamadas

```javascript
// Usuario presiona tecla
window.addEventListener('keydown', (e) => {
    // 1. Intento primario: API de botones
    try {
        currentEmulator.pressDown('down');
    } catch (err) {
        // 2. Fallback: API de teclado raw
        try {
            currentEmulator.keyboardDown('ArrowDown');
        } catch (err2) {
            // Silenciar error (emulador no inicializado)
        }
    }
});
```

### Canvas Focus Management

```javascript
// Asegurar que el canvas tiene foco
const canvas = document.getElementById('nes-screen');
if (canvas && document.activeElement !== canvas) {
    canvas.focus();
}

// Listener de focus
canvas.addEventListener('click', () => {
    canvas.focus();
});
```

---

## 🧪 Debugging y Diagnóstico

### Puntos de Verificación

#### 1. Configuración Libretro
```javascript
console.log('Port 5 Device:', retroarchConfig.input_libretro_device_p5);
// Esperado: '1539' (string)
```

#### 2. Eventos del Navegador
```javascript
window.addEventListener('keydown', (e) => {
    console.log('KeyboardEvent:', {
        keyCode: e.keyCode,
        which: e.which,
        code: e.code,
        key: e.key,
        defaultPrevented: e.defaultPrevented
    });
});
```

#### 3. Llamadas al Emulador
```javascript
try {
    currentEmulator.keyboardDown(e.code);
    console.log('✅ Keyboard input sent:', e.code);
} catch (err) {
    console.error('❌ Failed to send input:', err);
}
```

#### 4. Estado del Canvas
```javascript
console.log('Canvas focused:', document.activeElement === canvas);
console.log('Fullscreen:', !!document.fullscreenElement);
```

### Herramientas de Diagnóstico

```javascript
// Agregar al HTML para debugging
<div id="debug-panel" style="position:fixed;top:0;right:0;background:rgba(0,0,0,0.8);color:#0f0;padding:10px;font-family:monospace;">
    <div id="debug-keycode">KeyCode: -</div>
    <div id="debug-device">Device P5: -</div>
    <div id="debug-focus">Focus: -</div>
    <div id="debug-fullscreen">Fullscreen: -</div>
</div>

<script>
window.addEventListener('keydown', (e) => {
    document.getElementById('debug-keycode').textContent = `KeyCode: ${e.keyCode} (${e.code})`;
});
setInterval(() => {
    document.getElementById('debug-focus').textContent = `Focus: ${document.activeElement?.id || 'none'}`;
    document.getElementById('debug-fullscreen').textContent = `Fullscreen: ${!!document.fullscreenElement}`;
}, 500);
</script>
```

---

## 📊 Métricas de Performance

### Latencia Típica (Desktop)

| Segmento | Tiempo | Acumulado |
|----------|--------|-----------|
| Hardware → Browser | ~1ms | 1ms |
| Browser → JavaScript | ~0.5ms | 1.5ms |
| JavaScript → Canvas | ~0.1ms | 1.6ms |
| Canvas → WASM | ~0.2ms | 1.8ms |
| WASM → Libretro | ~0.1ms | 1.9ms |
| Libretro → FCEUmm | ~0.1ms | 2ms |
| FCEUmm → Subor Matrix | ~0.5ms | 2.5ms |
| ROM procesa input | ~1-16ms* | 3.5-18.5ms |

*Depende del frame rate (60 FPS = 16.67ms por frame)

### Input Lag Total
- **Mejor caso:** ~3.5ms (input procesado en mismo frame)
- **Caso típico:** ~20ms (1 frame de delay + procesamiento)
- **Peor caso:** ~35ms (2 frames de delay)

**Comparación:**
- Hardware real NES: ~16-33ms
- Emulador desktop: ~10-25ms
- Emulador web: ~20-35ms (aceptable para uso educativo)

---

## 🔮 Futuras Mejoras (Roadmap)

### Fase 2: Gamepad Virtual Adaptado
```javascript
class SuborKeyboardAdapter {
    constructor() {
        this.mapping = {
            'up': 'ArrowUp',
            'down': 'ArrowDown',
            'left': 'ArrowLeft',
            'right': 'ArrowRight',
            'a': 'Enter',
            'b': 'Space',
            'select': 'Tab',
            'start': 'Escape'
        };
    }
    
    handleVirtualInput(button, isDown) {
        const key = this.mapping[button];
        if (key) {
            const event = new KeyboardEvent(
                isDown ? 'keydown' : 'keyup',
                { code: key, key: key, bubbles: true }
            );
            canvas.dispatchEvent(event);
        }
    }
}
```

### Fase 3: Mouse Support (Opciones)
1. **Opción A:** Fork de FCEUmm para exponer `SI_MOUSE`
2. **Opción B:** Micro-core JS dedicado para PC Vivaz
3. **Opción C:** Migrar a Mesen Web

### Fase 4: Teclado Virtual en Pantalla
- QWERTY completo táctil
- Auto-hide inteligente
- Feedback visual de teclas presionadas

---

**Fecha:** 5 de octubre de 2026  
**Versión:** 1.0 — Post-FASE 1  
**Autor:** Sistema de Documentación Técnica
