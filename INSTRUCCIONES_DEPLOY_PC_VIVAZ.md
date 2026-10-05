# 🚀 Instrucciones para Deploy de PC Vivaz FASE 1 + 1.1

## ✅ Lo que se implementó

### **FASE 1 - Corrección Quirúrgica de Input**
- ✅ Device P5 configurado en `'1539'` (RETRO_DEVICE_FC_SUBORKB)
- ✅ preventDefault() condicional en keydown/keyup
- ✅ 6 documentos técnicos completos

### **FASE 1.1 - Sistema de Debugging Visual**
- ✅ Panel de debug flotante en tiempo real
- ✅ Logging completo en consola del navegador
- ✅ Indicadores de estado automáticos
- ✅ Cache-busting en app.js

---

## 🛠️ Cómo Hacer el Deploy

### **Opción 1: Ejecutar el Script Batch (Recomendado)**

1. Abre el explorador de archivos
2. Ve a: `d:\Projects\DevilutionX`
3. Haz doble clic en: **`commit_pcvivaz_fase1.bat`**
4. El script hará automáticamente:
   - `git add` de todos los archivos
   - `git commit` con mensaje descriptivo
   - `git push origin master`

### **Opción 2: Manual por Comandos**

Abre PowerShell o CMD en `d:\Projects\DevilutionX` y ejecuta:

```bash
git add CHANGELOG.md
git add Packaging/emscripten/nes/app.js
git add Packaging/emscripten/nes/index.html
git add PC_VIVAZ_FASE1.1_DEBUG_SYSTEM.md
git add PC_VIVAZ_FASE1_SUMMARY.md
git add "documentacion imporante/PC_VIVAZ_ARQUITECTURA_INPUT.md"
git add "documentacion imporante/PC_VIVAZ_DEBUG_FLOWCHART.md"
git add "documentacion imporante/PC_VIVAZ_FASE1_FIX_INPUT.md"
git add "documentacion imporante/PC_VIVAZ_QUICK_REFERENCE.md"
git add "documentacion imporante/PC_VIVAZ_README.md"
git add "documentacion imporante/PC_VIVAZ_RESUMEN_EJECUTIVO.md"

git commit -m "feat(pc-vivaz): Add FASE 1 input fix and visual debug system"

git push origin master
```

---

## 🌐 Verificar Deploy en GitHub Pages

### **Paso 1: Esperar a que se depliegue**
- Después del push, espera 2-5 minutos
- GitHub Pages se actualiza automáticamente

### **Paso 2: Abrir la URL del emulador**

Reemplaza con tu usuario y repo:
```
https://[TU-USUARIO].github.io/[TU-REPO]/Packaging/emscripten/nes/
```

**Ejemplo:**
```
https://xiphos.github.io/DevilutionX/Packaging/emscripten/nes/
```

### **Paso 3: Verificar el Debug Panel**

1. La página del emulador debe cargar
2. Deberías ver el **botón "🔍 DEBUG"** en la esquina superior derecha
3. El panel de debug debe aparecer automáticamente

---

## 🧪 Cómo Probar que Funciona

### **Test 1: Verificar que el Debug Panel está activo**

1. Abre la página del emulador
2. Verifica que aparece el panel de debug:
   ```
   🔍 PC VIVAZ DEBUG - FASE 1
   Estado: Esperando ROM...
   ```

### **Test 2: Cargar PC Vivaz**

1. Haz clic en **"📂 Cargar ROM"**
2. Selecciona tu archivo `pcvivaz-unif.nes`
3. Espera a que cargue

### **Test 3: Verificar Configuración**

En el panel de debug, deberías ver:
```
Estado: ✅ PC Vivaz detected
Device P5: 1539 (en verde ✅)
Core: fceumm
Computer Mode: ✅ YES (PC Vivaz)
```

**Si ves "Device P5: 1" en rojo:**
- El navegador está cacheando el archivo viejo
- Presiona **Ctrl + Shift + R** para recarga forzada

### **Test 4: Dar Focus al Canvas**

1. Haz clic en la pantalla del juego (sobre la imagen de PC Vivaz)
2. Verifica en el panel: **"Canvas Focus: ✅ YES"**

### **Test 5: Probar Teclas**

1. Presiona **↓** (flecha abajo)
2. Mira el panel de debug:
   ```
   Último Input:
   [14:32:15] Key: ArrowDown (ArrowDown)
   Action: down
   ```

3. Abre la consola del navegador (F12)
4. Deberías ver logs en verde:
   ```javascript
   [CONFIG] ✅ PC VIVAZ MODE ACTIVATED
   [INPUT] ⌨️ Key Down { code: "ArrowDown", ... }
   [INPUT] ✅ pressDown() successful
   ```

---

## 🔍 Diagnóstico de Problemas

### **Problema: Panel de debug no aparece**

**Causa:** Cache del navegador

**Solución:**
```
1. Presionar Ctrl + Shift + R (recarga forzada)
2. O presionar Ctrl + F5
3. O limpiar cache del navegador completamente
```

### **Problema: Device P5 muestra '1' en lugar de '1539'**

**Causa:** Navegador usando archivo antiguo

**Solución:**
```
1. Cerrar TODAS las ventanas del navegador
2. Abrir navegador nuevamente
3. Ir a la URL del emulador
4. Verificar que ahora muestra '1539'
```

### **Problema: Canvas Focus siempre es NO**

**Causa:** No hiciste clic en la pantalla

**Solución:**
```
1. Hacer clic directamente en la pantalla del juego
2. Debería cambiar a "✅ YES"
```

### **Problema: Teclas no responden pero logs se ven**

**Esto es importante:** Si ves logs en consola pero PC Vivaz no responde, significa:
- ✅ El fix de FASE 1 está activo
- ✅ Los eventos llegan correctamente
- ❌ El problema está en otro lado (FCEUmm WASM, ROM, etc.)

**Reporta:**
- Screenshot del panel de debug
- Logs de la consola (F12)
- Descripción de qué teclas probaste

---

## 📊 Checklist de Validación Completa

Antes de reportar resultados, verifica:

- [ ] Script batch ejecutado exitosamente
- [ ] `git push` completado sin errores
- [ ] GitHub Pages actualizado (2-5 minutos de espera)
- [ ] Página del emulador carga correctamente
- [ ] Panel de debug aparece
- [ ] ROM PC Vivaz carga
- [ ] "✅ PC Vivaz detected" aparece
- [ ] Device P5 = 1539 (verde)
- [ ] Computer Mode = ✅ YES
- [ ] Hiciste clic en la pantalla
- [ ] Canvas Focus = ✅ YES
- [ ] Probaste al menos 3 teclas (↓, Enter, A)
- [ ] Revisaste logs en consola (F12)

---

## 📸 Screenshots a Capturar

Para reportar resultados, captura:

1. **Panel de debug completo** (mostrando todos los valores)
2. **Consola del navegador** (logs verdes de CONFIG e INPUT)
3. **Pantalla del juego** (para ver si hay cambio visual)

---

## 🎯 Resultados Esperados

### **Escenario A: TODO FUNCIONA** ✅

```
Panel de Debug:
- Device P5: 1539 ✅
- Canvas Focus: ✅ YES
- Emulator Ready: ✅ YES

Consola:
- [CONFIG] ✅ PC VIVAZ MODE ACTIVATED
- [INPUT] ⌨️ Key Down
- [INPUT] ✅ pressDown() successful

PC Vivaz:
- El cursor se mueve con flechas
- Enter abre aplicaciones
- Se puede escribir texto
```

### **Escenario B: TECLAS LLEGAN PERO PC VIVAZ NO RESPONDE** ⚠️

```
Panel de Debug:
- Device P5: 1539 ✅
- Canvas Focus: ✅ YES
- Emulator Ready: ✅ YES

Consola:
- [CONFIG] ✅ PC VIVAZ MODE ACTIVATED
- [INPUT] ⌨️ Key Down
- [INPUT] ✅ pressDown() successful

PC Vivaz:
- NO responde a ninguna tecla
- La pantalla no cambia
```

**Esto significa:** El problema está en FCEUmm WASM o la ROM.

### **Escenario C: CONFIGURACIÓN INCORRECTA** ❌

```
Panel de Debug:
- Device P5: 1 ❌ (en rojo)
- O Computer Mode: ❌ NO

Consola:
- No aparece [CONFIG] ✅ PC VIVAZ MODE ACTIVATED
```

**Esto significa:** Cache del navegador, hacer Ctrl+Shift+R.

---

## 🚀 Próximos Pasos Según Resultado

### **Si Escenario A (funciona):**
🎉 ¡Éxito total! Documentar casos de uso y planificar FASE 2.

### **Si Escenario B (logs OK pero no responde):**
🔍 Investigar FCEUmm WASM. Posibles causas:
- FCEUmm no procesa el Device ID 1539 correctamente
- ROM tiene problema interno
- Incompatibilidad entre Libretro API y FCEUmm

### **Si Escenario C (config incorrecta):**
🔧 Solución simple: limpiar cache y recargar.

---

## 📞 Soporte

**Para reportar resultados:**

Envía:
1. Screenshot del panel de debug
2. Screenshot de la consola (F12) con logs
3. Descripción breve: "Escenario A/B/C"

---

**Fecha:** 5 de octubre de 2026  
**Versión:** FASE 1 + 1.1 — Ready for Deploy  
**Status:** ✅ Listo para Push
