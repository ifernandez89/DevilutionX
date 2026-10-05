# 🚀 PC Vivaz — Quick Reference & Troubleshooting

## ⚡ Estado Actual (Post-FASE 1)

### ✅ Lo que funciona
- Video, CPU, ROM cargan correctamente
- Pantalla muestra menú de PC Vivaz
- Arquitectura completa de conversión UNIF → NES 2.0

### 🔧 Lo que se acaba de arreglar (FASE 1 - Oct 2026)
- Puerto de expansión configurado en `1539` (FC_SUBORKB)
- Eventos de teclado llegan al canvas Emscripten
- Driver `rwebinput` recibe inputs correctamente

### ⏳ Pendiente de prueba
- **Teclado físico en Desktop**: Esperando validación MVP

### ❌ Fuera de scope (por ahora)
- Mouse (requiere modificación de WASM o alternativa)
- Gamepad virtual adaptado para móviles
- Pointer Lock refinado

---

## 🎮 Prueba Rápida — ¿Funciona?

### Cargar ROM
1. Ir a la página del emulador NES
2. Cargar `pcvivaz-unif.nes`
3. Esperar que aparezca el menú principal

### Prueba de 30 Segundos
```
Presionar: ↓
¿Se mueve el cursor? → ✅ FUNCIONA

Presionar: Enter
¿Entra en la app? → ✅ FUNCIONA

Presionar: Esc
¿Vuelve al menú? → ✅ FUNCIONA

Escribir: H O L A (en Hoja Mágica)
¿Aparece texto? → ✅ FUNCIONA PERFECTO
```

---

## 🔧 Configuración Crítica

### Puerto de Expansión (Teclado Subor)
```javascript
// Packaging/emscripten/nes/app.js ~línea 557
input_libretro_device_p5: '1539'  // ¡DEBE ser string '1539'!

// ❌ NO usar:
// input_libretro_device_p5: '1'    // Genérico, no activa Subor
// input_libretro_device_p5: 1539   // Number, puede fallar en Libretro
```

### Manejo de Eventos
```javascript
// preventDefault() SOLO en fullscreen + flechas
if (e.key.startsWith('Arrow') && document.fullscreenElement) {
    e.preventDefault();
}
// De otro modo: dejar pasar el evento al canvas
```

---

## 🐛 Troubleshooting

### Problema: "No responde ninguna tecla"
**Diagnóstico:**
1. Abrir DevTools → Console
2. Verificar errores de JavaScript
3. Confirmar que `input_libretro_device_p5` es `'1539'` (string)

**Solución:**
- Si es `'1'`: cambiar a `'1539'` y recargar
- Si hay errores JS: revisar sintaxis en `app.js`
- Si canvas no tiene foco: hacer clic en la pantalla

### Problema: "Scroll de página interfiere con flechas"
**Diagnóstico:**
- Flechas mueven la página en lugar del cursor en PC Vivaz

**Solución:**
- Entrar en modo fullscreen (F11 o botón de fullscreen)
- O hacer clic en el canvas para darle foco

### Problema: "Algunas teclas funcionan, otras no"
**Diagnóstico:**
- Posible conflicto con hotkeys del navegador

**Solución:**
- Verificar que no haya extensiones que capturen teclas
- Probar en modo incógnito
- Verificar que `KEY_ACTIONS` no esté mapeando incorrectamente

### Problema: "Funciona en PC pero no en móvil"
**Esperado:**
- PC Vivaz en móvil requiere gamepad virtual adaptado (Fase 2)
- Actualmente solo funciona con teclado físico en Desktop

**Solución temporal:**
- Usar en Desktop por ahora
- Fase 2 implementará `SuborKeyboardAdapter`

---

## 📊 Mapa de Teclas

### Navegación
| Tecla | Función | Registro |
|-------|---------|----------|
| ↑     | Arriba  | $4016 Fila 8 |
| ↓     | Abajo   | $4016 Fila 8 |
| ←     | Izquierda | $4016 Fila 8 |
| →     | Derecha | $4016 Fila 8 |

### Control
| Tecla | Función | Registro |
|-------|---------|----------|
| Enter | Confirmar / Nueva línea | $4016 Fila 2 |
| Esc   | Salir / Cancelar | $4016 Fila 0 |
| Space | Espacio | $4016 Fila 5 |
| Tab   | Tabulador | $4016 Fila 2 |
| Backspace | Borrar | $4016 Fila 0 |

### Tipeo
| Teclas | Función | Registro |
|--------|---------|----------|
| A-Z    | Letras  | Varias filas $4016 |
| 0-9    | Números | Filas 3-4 $4016 |
| Shift  | Mayúsculas | $4016 Fila 1 |

---

## 🔄 Flujo de Señal Simplificado

```
Usuario → Browser keydown → app.js (sin preventDefault) 
→ Canvas Emscripten → rwebinput → Libretro (puerto 5 = 1539)
→ FCEUmm (FC_SUBORKB) → $4016/$4017 → PC Vivaz ROM → Pantalla
```

---

## 📝 Checklist de Validación

### Antes de Reportar un Bug
- [ ] ROM cargada correctamente (`pcvivaz-unif.nes`)
- [ ] Aparece menú de PC Vivaz en pantalla
- [ ] Canvas tiene foco (hacer clic si es necesario)
- [ ] `input_libretro_device_p5` es `'1539'` (string)
- [ ] No hay errores en DevTools Console
- [ ] Probado en Desktop PC (no móvil)
- [ ] Probado en Chrome/Edge/Firefox

### Si Todo lo Anterior es ✅
- [ ] Probar flecha ↓
- [ ] Probar Enter
- [ ] Probar Esc
- [ ] Probar tipeo A-Z

### Si Alguna Tecla Funciona
**🎉 ÉXITO → El circuito está cerrado.**

Reportar:
- Qué teclas funcionan
- Qué teclas no funcionan
- Logs de console si hay errores

---

## 🎯 Métricas de Éxito

### MVP (Mínimo Producto Viable)
- **1 tecla funciona** → 🟢 Señal excelente, circuito cerrado

### Básico
- **Navegación (flechas + Enter)** → 🟢 PC Vivaz utilizable

### Completo
- **Navegación + Tipeo (A-Z, 0-9)** → 🟢🟢 Sistema educativo funcional

### Perfecto
- **Todo lo anterior + Hotkeys (Tab, Esc, Backspace)** → 🟢🟢🟢 Experiencia completa

---

## 📚 Documentación Relacionada

### Detalles Técnicos
- `PC_VIVAZ_FASE1_FIX_INPUT.md` — Análisis profundo del fix
- `PC_VIVAZ_HARDWARE_Y_MAPEO.md` — Especificación de hardware

### Código
- `Packaging/emscripten/nes/app.js` — Implementación principal
- Línea ~557: Configuración puerto 5
- Línea ~966: Handler keydown
- Línea ~1019: Handler keyup

### Historial
- `CHANGELOG.md` — Entrada completa de FASE 1

---

## 🚨 Contacto y Soporte

### Si nada funciona después de validar el checklist:
1. Capturar screenshot de DevTools Console
2. Capturar screenshot de la pantalla de PC Vivaz
3. Anotar qué teclas probaste
4. Anotar navegador y versión
5. Reportar con toda la info anterior

### Si funciona parcialmente:
1. Documentar qué funciona
2. Documentar qué no funciona
3. Considerar si es suficiente para el uso previsto
4. Si no es suficiente, planificar Fase 2

---

**Última actualización:** 5 de octubre de 2026  
**Versión:** FASE 1 — Post-Fix  
**Estado:** Esperando Validación MVP
