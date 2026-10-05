# 🔍 PC Vivaz — Flowchart de Debugging (FASE 1)

## 🎯 Objetivo

Guía visual para diagnosticar problemas de input en PC Vivaz después de implementar FASE 1.

---

## 🧪 Prueba MVP — Diagrama de Decisión

```
          ┌─────────────────────────┐
          │ Cargar pcvivaz-unif.nes │
          └────────┬────────────────┘
                   │
                   ▼
          ┌─────────────────────────┐
          │ ¿Aparece menú PC Vivaz? │
          └────────┬────────────────┘
                   │
         ┌─────────┴─────────┐
         │                   │
        NO                  SÍ
         │                   │
         ▼                   ▼
    ┌─────────┐        ┌──────────┐
    │ PROBLEMA│        │ Presionar│
    │ DE ROM  │        │    ↓     │
    └─────────┘        └────┬─────┘
         │                   │
         │         ┌─────────┴─────────┐
         │         │                   │
         │        NO                  SÍ
         │         │                   │
         ▼         ▼                   ▼
    Ver Sección  INPUT            ┌──────────┐
    "ROM no      FALLÓ             │ Presionar│
     carga"      ↓                 │  Enter   │
                 │                 └────┬─────┘
                 │                      │
                 │            ┌─────────┴─────────┐
                 │            │                   │
                 │           NO                  SÍ
                 │            │                   │
                 │            ▼                   ▼
                 │      INPUT         ┌───────────────┐
                 │      PARCIAL       │ Escribir HOLA │
                 │         ↓          │ (en app)      │
                 │         │          └────┬──────────┘
                 │         │               │
                 │         │     ┌─────────┴─────────┐
                 │         │     │                   │
                 │         │    NO                  SÍ
                 │         │     │                   │
                 │         │     ▼                   ▼
                 │         │  INPUT          ┌────────────┐
                 │         │  BÁSICO         │   ÉXITO    │
                 │         │  (Navegación    │  COMPLETO  │
                 │         │   funciona)     │            │
                 │         │                 └────────────┘
                 │         │
                 └─────────┴─────► Ver tabla de debugging
```

---

## 📋 Tabla de Diagnóstico Rápido

| Síntoma | Causa Probable | Solución |
|---------|----------------|----------|
| No carga el menú | ROM corrupta o conversión UNIF falló | Verificar archivo ROM |
| Menú carga pero nada responde | Device ID incorrecto (no 1539) | Verificar config línea 557 |
| Solo algunas teclas funcionan | Mapeo incorrecto | Verificar KEY_ACTIONS |
| Teclas causan scroll | preventDefault no activo | Verificar fullscreen |
| Console muestra errores JS | Sintaxis o lógica | Ver logs detallados |

---

## 🔬 Checklist de Verificación Paso a Paso

### ✅ Paso 1: Verificar Configuración

```javascript
// En DevTools Console:
console.log(retroarchConfig.input_libretro_device_p5);
// DEBE mostrar: '1539' (string)

// ❌ Si muestra '1' → Config incorrecta
// ❌ Si muestra 1539 (number) → Puede fallar en Libretro
// ✅ Si muestra '1539' (string) → Correcto
```

### ✅ Paso 2: Verificar Eventos del Navegador

```javascript
// Agregar listener temporal:
window.addEventListener('keydown', (e) => {
    console.log('Key Event:', {
        keyCode: e.keyCode,
        which: e.which,
        code: e.code,
        key: e.key,
        defaultPrevented: e.defaultPrevented
    });
}, true);  // true = capture phase

// Presionar ↓
// DEBE mostrar:
// keyCode: 40
// which: 40
// code: "ArrowDown"
// key: "ArrowDown"
// defaultPrevented: false (o true solo en fullscreen)
```

### ✅ Paso 3: Verificar Canvas Focus

```javascript
// En DevTools Console:
const canvas = document.getElementById('nes-screen');
console.log('Canvas focused:', document.activeElement === canvas);

// ❌ Si muestra false → Hacer clic en el canvas
// ✅ Si muestra true → Canvas tiene foco
```

### ✅ Paso 4: Verificar Llamadas al Emulador

```javascript
// Modificar temporalmente app.js para logging:
try {
    console.log('Sending key to emulator:', e.code);
    currentEmulator.keyboardDown(e.code);
    console.log('✅ Key sent successfully');
} catch (err) {
    console.error('❌ Failed to send key:', err);
}
```

---

## 🐛 Debugging Avanzado

### Problema: "No funciona NINGUNA tecla"

#### Verificaciones:

1. **Config del Puerto 5**
```javascript
// app.js línea ~557
input_libretro_device_p5: '1539'  // ✅ DEBE ser '1539' (string)
```

2. **preventDefault() Condicional**
```javascript
// app.js línea ~970
if (e.key.startsWith('Arrow') && document.fullscreenElement) {
    e.preventDefault();  // ✅ SOLO si fullscreen + Arrow
}
```

3. **Canvas Montado**
```javascript
const canvas = document.getElementById('nes-screen');
console.log('Canvas exists:', !!canvas);
console.log('Canvas in DOM:', document.body.contains(canvas));
```

4. **Emulador Inicializado**
```javascript
console.log('Emulator exists:', !!currentEmulator);
console.log('Emulator methods:', Object.keys(currentEmulator));
```

---

### Problema: "Solo flechas funcionan, letras no"

#### Causa probable:
- KEY_ACTIONS mapea solo flechas
- Letras se envían vía fallback `keyboardDown()`

#### Verificación:
```javascript
// app.js línea ~960
const KEY_ACTIONS = {
    'ArrowUp': 'up',
    'ArrowDown': 'down',
    'ArrowLeft': 'left',
    'ArrowRight': 'right',
    // ... etc
};

// Verificar que el else branch se ejecuta:
if (action) {
    // Flechas van aquí
} else {
    // ✅ Letras DEBEN ir aquí
    try {
        currentEmulator.keyboardDown(e.code);
    } catch (_) {}
}
```

---

### Problema: "Funciona en desktop pero no en móvil"

#### Esperado:
- PC Vivaz en móvil requiere gamepad virtual adaptado (FASE 2)
- Actualmente solo funciona con teclado físico

#### Verificación:
```javascript
// app.js debe tener detección de plataforma:
const isDesktopPC = () => {
    return window.matchMedia('(pointer: fine)').matches;
};

console.log('Is Desktop PC:', isDesktopPC());
// ✅ Desktop → true
// ❌ Móvil → false
```

---

## 🔍 Panel de Debugging en Vivo

### Código para agregar al HTML (temporal):

```html
<div id="pc-vivaz-debug" style="
    position: fixed;
    top: 10px;
    right: 10px;
    background: rgba(0, 0, 0, 0.9);
    color: #0f0;
    padding: 15px;
    font-family: 'Courier New', monospace;
    font-size: 12px;
    border: 2px solid #0f0;
    border-radius: 5px;
    z-index: 10000;
    min-width: 300px;
">
    <div style="font-weight: bold; margin-bottom: 10px; color: #ff0;">
        🔍 PC VIVAZ DEBUG PANEL
    </div>
    <div id="debug-device">Device P5: -</div>
    <div id="debug-focus">Canvas Focus: -</div>
    <div id="debug-fullscreen">Fullscreen: -</div>
    <div id="debug-emulator">Emulator: -</div>
    <div id="debug-last-key">Last Key: -</div>
    <div id="debug-prevented">Prevented: -</div>
</div>

<script>
// Actualizar panel cada 500ms
setInterval(() => {
    const canvas = document.getElementById('nes-screen');
    document.getElementById('debug-device').textContent = 
        `Device P5: ${retroarchConfig?.input_libretro_device_p5 || 'N/A'}`;
    document.getElementById('debug-focus').textContent = 
        `Canvas Focus: ${document.activeElement === canvas}`;
    document.getElementById('debug-fullscreen').textContent = 
        `Fullscreen: ${!!document.fullscreenElement}`;
    document.getElementById('debug-emulator').textContent = 
        `Emulator: ${!!currentEmulator}`;
}, 500);

// Capturar eventos
let lastPrevented = false;
window.addEventListener('keydown', (e) => {
    document.getElementById('debug-last-key').textContent = 
        `Last Key: ${e.code} (${e.keyCode})`;
    lastPrevented = e.defaultPrevented;
    document.getElementById('debug-prevented').textContent = 
        `Prevented: ${lastPrevented}`;
}, true);
</script>
```

---

## 📊 Matriz de Síntomas vs Causas

| Síntoma | Device P5 | preventDefault | Focus | Emulator | Diagnóstico |
|---------|-----------|----------------|-------|----------|-------------|
| Nada funciona | '1' | N/A | N/A | N/A | ❌ Device ID incorrecto |
| Nada funciona | '1539' | true | false | true | ⚠️ Canvas sin foco |
| Nada funciona | '1539' | true | true | false | ❌ Emulador no init |
| Nada funciona | '1539' | true | true | true | 🐛 Bug en rwebinput |
| Solo flechas | '1539' | false | true | true | ⚠️ Mapeo incompleto |
| Scroll molesto | '1539' | false | true | true | ⚠️ Activar fullscreen |
| Todo funciona | '1539' | conditional | true | true | ✅ PERFECTO |

---

## 🎯 Checklist de Resolución

### Si NO funciona nada:

- [ ] Verificar `input_libretro_device_p5 === '1539'`
- [ ] Hacer clic en el canvas para darle foco
- [ ] Verificar que no hay errores en Console
- [ ] Confirmar que `currentEmulator` existe
- [ ] Probar en modo incógnito (sin extensiones)
- [ ] Probar con otro navegador

### Si funciona PARCIALMENTE:

- [ ] Documentar qué funciona y qué no
- [ ] Verificar KEY_ACTIONS para teclas mapeadas
- [ ] Confirmar que el else branch se ejecuta para otras teclas
- [ ] Revisar logs de console para errores específicos

### Si funciona PERFECTAMENTE:

- [ ] ✅ Celebrar 🎉
- [ ] Documentar casos de uso exitosos
- [ ] Reportar resultados detallados
- [ ] Planificar FASE 2

---

## 🔗 Referencias Rápidas

### Líneas Críticas en app.js

```
Línea ~557:  input_libretro_device_p5: '1539'
Línea ~970:  if (e.key.startsWith('Arrow') && document.fullscreenElement)
Línea ~1024: if (e.key.startsWith('Arrow') && document.fullscreenElement)
```

### Valores Correctos

```javascript
input_libretro_device_p5: '1539'  // ✅ String, no number
e.defaultPrevented: false          // ✅ Excepto fullscreen + Arrow
document.activeElement: canvas     // ✅ Canvas debe tener foco
currentEmulator: Object            // ✅ Debe existir
```

---

## 📞 Reporte de Bug (Template)

```markdown
### BUG REPORT - PC VIVAZ FASE 1

**Síntoma:**
[Describir qué no funciona]

**Configuración Verificada:**
- Device P5: [valor]
- Canvas Focus: [true/false]
- Fullscreen: [true/false]
- Emulator Exists: [true/false]

**Logs de Console:**
```
[pegar logs aquí]
```

**Teclas Probadas:**
- ↓: [✅/❌]
- Enter: [✅/❌]
- Letra A: [✅/❌]

**Navegador:**
[Chrome/Firefox/Edge] [versión]

**Sistema:**
[Windows/Mac/Linux]

**Reproducción:**
1. [Paso 1]
2. [Paso 2]
3. [Resultado inesperado]
```

---

**Última actualización:** 5 de octubre de 2026  
**Versión:** 1.0 — FASE 1 Debugging Guide
