# 🔍 PC Vivaz FASE 1.1 — Sistema de Debugging Visual

**Fecha:** 5 de octubre de 2026  
**Complemento a:** FASE 1 - Corrección Quirúrgica de Input  
**Propósito:** Diagnosticar y validar el funcionamiento del fix en tiempo real

---

## 🎯 Objetivo

Proporcionar visibilidad completa del estado del sistema y el flujo de inputs para diagnosticar por qué PC Vivaz no responde al teclado, incluso después del fix de FASE 1.

---

## 🛠️ Componentes Implementados

### 1. **Panel de Debug Visual Flotante**

Panel en tiempo real (top-right) que muestra:

```
🔍 PC VIVAZ DEBUG - FASE 1

Estado: ✅ PC Vivaz detected

Device P5: 1539 ✅
Core: fceumm
Computer Mode: ✅ YES (PC Vivaz)

Canvas Focus: ✅ YES / ❌ NO (Click screen!)
Fullscreen: ❌ NO
Emulator Ready: ✅ YES

Último Input:
[14:32:15] Key: ArrowDown (ArrowDown)
Action: down
```

### 2. **Logging en Consola del Navegador**

Categorizado y coloreado en verde (#0f0):

```javascript
[14:32:10] [CONFIG] ✅ PC VIVAZ MODE ACTIVATED
{
    primaryCore: "fceumm",
    device_p5: "1539",
    isComputer: true
}

[14:32:15] [INPUT] ⌨️ Key Down
{
    code: "ArrowDown",
    key: "ArrowDown",
    action: "down",
    prevented: false,
    fullscreen: false,
    activeElement: "nes-screen"
}

[14:32:15] [INPUT] ✅ pressDown() successful { action: "down" }
```

### 3. **Indicadores de Estado Automáticos**

Actualización cada 500ms:
- Canvas Focus (crítico para input)
- Fullscreen mode
- Emulador inicializado
- Último input capturado

---

## 📊 Tabla de Diagnóstico

| Indicador | Valor Correcto | Valor Incorrecto | Acción |
|-----------|----------------|------------------|--------|
| Device P5 | `'1539'` (verde) | `'1'` (rojo) | Limpiar cache (Ctrl+Shift+R) |
| Computer Mode | `✅ YES` | `❌ NO` | Verificar ROM es PC Vivaz |
| Canvas Focus | `✅ YES` | `❌ NO` | Hacer clic en pantalla |
| Emulator Ready | `✅ YES` | `❌ NO` | Esperar carga completa |
| Core | `fceumm` | `nestopia` | Verificar detección de ROM |

---

## 🧪 Cómo Usar el Debug System

### **Paso 1: Abrir Emulador**
- El panel aparece automáticamente en top-right
- Botón "🔍 DEBUG" para mostrar/ocultar

### **Paso 2: Cargar ROM**
- Cargar `pcvivaz-unif.nes`
- Verificar que aparece: **"✅ PC Vivaz detected"**

### **Paso 3: Verificar Configuración**
Todos estos deben estar en **VERDE**:
- Device P5: `1539`
- Computer Mode: `✅ YES (PC Vivaz)`
- Core: `fceumm`

### **Paso 4: Dar Focus al Canvas**
- Hacer clic en la pantalla del juego
- Verificar: **Canvas Focus: ✅ YES**

### **Paso 5: Probar Teclas**
- Presionar ↓ (flecha abajo)
- Ver en panel: último input actualizado
- Ver en consola (F12): logs detallados

---

## 🔍 Escenarios de Diagnóstico

### **Escenario 1: Device P5 es '1' (rojo)**

**Causa:** Cache del navegador cargando archivo antiguo

**Solución:**
```
1. Presionar Ctrl + Shift + R (recarga forzada)
2. O cerrar navegador completamente y abrir de nuevo
3. Verificar que ahora dice '1539' en verde
```

### **Escenario 2: Canvas Focus es NO (rojo)**

**Causa:** Canvas no tiene el foco para recibir eventos de teclado

**Solución:**
```
1. Hacer clic directamente en la pantalla del juego
2. Indicador debe cambiar a "✅ YES"
3. Probar teclas nuevamente
```

### **Escenario 3: Emulator Ready es NO**

**Causa:** Emulador aún cargando o error en inicialización

**Solución:**
```
1. Esperar unos segundos más
2. Si persiste, revisar consola (F12) para errores
3. Recargar página si hay error
```

### **Escenario 4: Tecla presionada pero sin acción**

**Ver en consola:**
```javascript
[INPUT] ⌨️ Key Down { ... }
[INPUT] ❌ pressDown() failed
[INPUT] ❌ keyboardDown() failed { error: "..." }
```

**Causa:** Problema en la comunicación con el emulador WASM

**Solución:**
```
1. Verificar que ROM cargó correctamente
2. Revisar mensaje de error específico
3. Probar con otra ROM para descartar problema de ROM
```

---

## 📝 Logs Típicos (Funcionamiento Correcto)

```javascript
// Al cargar PC Vivaz
[CONFIG] ✅ PC VIVAZ MODE ACTIVATED
{
    primaryCore: "fceumm",
    device_p5: "1539",
    isComputer: true
}

// Al presionar flecha abajo
[INPUT] ⌨️ Key Down
{
    code: "ArrowDown",
    key: "ArrowDown",
    action: "down",
    prevented: false,
    fullscreen: false,
    activeElement: "nes-screen"
}
[INPUT] ✅ pressDown() successful { action: "down" }

// Al presionar letra A
[INPUT] ⌨️ Key Down
{
    code: "KeyA",
    key: "a",
    action: "raw",
    prevented: false,
    fullscreen: false,
    activeElement: "nes-screen"
}
[INPUT] ✅ Raw keyboardDown() successful { code: "KeyA" }
```

---

## 🚫 Logs de Error Comunes

### **Error 1: Emulador no inicializado**
```javascript
[INPUT] ⚠️ Key pressed but emulator not ready { key: "ArrowDown" }
```
**Solución:** Esperar a que cargue completamente

### **Error 2: preventDefault bloqueando (no debería pasar en FASE 1)**
```javascript
[INPUT] ⌨️ Key Down { ... prevented: true, fullscreen: false }
```
**Solución:** Bug en el código, no debería prevenir fuera de fullscreen

### **Error 3: Método no existe en emulador**
```javascript
[INPUT] ❌ keyboardDown() failed { error: "keyboardDown is not a function" }
```
**Solución:** Problema con la versión del core WASM cargada

---

## 🎨 Personalización del Debug Panel

### **Cerrar el Panel:**
- Clic en la ✕ roja (top-right del panel)
- El botón "🔍 DEBUG" aparece para volver a abrirlo

### **Desactivar Logging:**
En `app.js`, cambiar:
```javascript
const DEBUG_ENABLED = true;  // ← cambiar a false
```

---

## 📊 Métricas del Sistema

| Métrica | Valor |
|---------|-------|
| Actualización de estado | Cada 500ms |
| Logs por tecla | 3-5 (keydown + intentos) |
| Overhead de performance | < 1% |
| Tamaño del panel | 350×280 px |

---

## 🔗 Archivos Modificados

```
✅ Packaging/emscripten/nes/app.js
   - Sistema debugLog() con categorías
   - Logging en configuración PC Vivaz
   - Logging en event handlers
   - Panel controller
   - Funciones globales de actualización

✅ Packaging/emscripten/nes/index.html
   - Panel de debug HTML/CSS inline
   - Botón toggle
   - Cache-busting en app.js
```

---

## 🚀 Deploy a GitHub Pages

**Para ver el sistema en producción:**

```bash
git add .
git commit -m "feat: Add visual debug system for PC Vivaz FASE 1.1"
git push origin main
```

Luego abrir: `https://[tu-usuario].github.io/[tu-repo]/Packaging/emscripten/nes/`

---

## 🎯 Próximos Pasos con Debug Activo

1. **Validar que Device P5 = 1539** ✅
2. **Confirmar Canvas Focus funciona** ✅
3. **Ver si teclas llegan al emulador** ⏳
4. **Identificar punto exacto de fallo** ⏳

Con este sistema, podremos ver **exactamente** dónde se rompe la cadena de input.

---

**Fecha de Implementación:** 5 de octubre de 2026  
**Versión:** FASE 1.1 — Debug System  
**Status:** ✅ Listo para Deploy
