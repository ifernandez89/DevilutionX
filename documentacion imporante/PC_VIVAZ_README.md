# 📚 PC Vivaz — Índice de Documentación

## 🎯 Estado Actual del Proyecto

**Fecha:** 5 de octubre de 2026  
**Versión:** FASE 1 Implementada — Esperando Validación MVP

### ✅ Completado
- Conversión UNIF → NES 2.0 funcional
- Mapper 329 reconocido por FCEUmm
- Video, CPU, ROM funcionando perfectamente
- **[NUEVO]** Puerto de expansión configurado correctamente (1539)
- **[NUEVO]** Eventos de teclado pasan al canvas Emscripten
- **[NUEVO]** Circuito completo de input cerrado (teóricamente)

### 🔬 En Prueba
- Validación MVP de teclado físico en Desktop

### ⏳ Pendiente
- Gamepad virtual adaptado para móviles (Fase 2)
- Soporte de mouse (evaluación pendiente)
- Pointer Lock refinado

---

## 📖 Documentación Disponible

### 🚀 Para Usuarios

#### [`PC_VIVAZ_QUICK_REFERENCE.md`](PC_VIVAZ_QUICK_REFERENCE.md)
**¿Qué es?** Guía rápida de uso y troubleshooting.  
**Cuándo leer:** Cuando quieras probar PC Vivaz ahora mismo.  
**Contenido:**
- Prueba de 30 segundos
- Troubleshooting común
- Mapa de teclas
- Checklist de validación

---

### 🔧 Para Desarrolladores

#### [`PC_VIVAZ_FASE1_FIX_INPUT.md`](PC_VIVAZ_FASE1_FIX_INPUT.md)
**¿Qué es?** Análisis profundo del fix de input de Fase 1.  
**Cuándo leer:** Cuando quieras entender QUÉ se hizo y POR QUÉ.  
**Contenido:**
- Diagnóstico de las 3 causas raíz
- Solución quirúrgica implementada
- Filosofía del fix (mínimo cambio)
- Flujo de señal completo
- Prueba MVP detallada

#### [`PC_VIVAZ_ARQUITECTURA_INPUT.md`](PC_VIVAZ_ARQUITECTURA_INPUT.md)
**¿Qué es?** Especificación técnica completa del sistema de input.  
**Cuándo leer:** Cuando quieras entender CÓMO funciona todo el stack.  
**Contenido:**
- Stack completo (9 capas: Hardware → ROM)
- Cálculo de Device IDs Libretro
- Matriz del teclado Subor (13×8)
- Integración con Emscripten
- Debugging y diagnóstico
- Métricas de performance
- Roadmap futuro

#### [`PC_VIVAZ_HARDWARE_Y_MAPEO.md`](PC_VIVAZ_HARDWARE_Y_MAPEO.md)
**¿Qué es?** Especificación de hardware del Educational Computer 2000.  
**Cuándo leer:** Cuando necesites referencia de hardware original.  
**Contenido:**
- Especificaciones del hardware Subor
- Protocolo del teclado matricial
- Diferencias con joystick estándar
- Configuración de registros `$4016`/`$4017`

---

### 📝 Historial de Cambios

#### [`CHANGELOG.md`](../CHANGELOG.md)
**¿Qué es?** Registro completo de todos los cambios del proyecto.  
**Cuándo leer:** Cuando quieras ver el historial cronológico.  
**Entrada relevante:** Sección "FASE 1 — Corrección Quirúrgica de Input"

---

## 🗺️ Mapa de Decisiones

### "¿Por dónde empiezo?"

```
┌─────────────────────────────────────────────┐
│ ¿Qué quieres hacer?                         │
└─────────────────┬───────────────────────────┘
                  │
        ┌─────────┴─────────┐
        │                   │
    PROBAR PC VIVAZ    ENTENDER/MODIFICAR
        │                   │
        ↓                   ↓
 Quick Reference      ¿Qué aspecto?
        │                   │
        │         ┌─────────┴─────────┐
        │         │                   │
        │    FIX RECIENTE        ARQUITECTURA
        │         │                   │
        │         ↓                   ↓
        │   FASE1_FIX_INPUT   ARQUITECTURA_INPUT
        │         │                   │
        │         └─────────┬─────────┘
        │                   │
        └─────────┬─────────┘
                  ↓
            HARDWARE_Y_MAPEO
            (Referencia HW)
```

---

## 🎓 Conceptos Clave

### ¿Qué es PC Vivaz?
Es una ROM de "Educational Computer 2000" (también conocida como PC Vivaz), un clon de Famicom/NES educativo fabricado en China/Rusia en los 90s. Incluye aplicaciones educativas como procesador de texto, hoja de cálculo y tutor de mecanografía.

### ¿Por qué es especial técnicamente?
- **Mapper 329 (UNL-EDU2000):** No estándar, poco documentado
- **Teclado matricial Subor:** 13 filas × 8 columnas, no es joystick
- **Mouse serie:** Protocolo propietario de 3 bytes
- **1 MB de ROM:** Mucho más grande que juegos NES típicos (32-512 KB)
- **32 KB SRAM con batería:** Para guardar documentos

### ¿Por qué fue difícil de hacer funcionar?
1. **Formato UNIF:** Contenedor no estándar, requirió conversión a NES 2.0
2. **Device ID incorrecto:** FCEUmm no activaba el hardware de teclado
3. **preventDefault() agresivo:** Bloqueaba los eventos antes de llegar al emulador
4. **Documentación escasa:** Muy poca info sobre el hardware Subor en inglés

### ¿Qué se logró en Fase 1?
- ✅ Cerrar el circuito de input Browser → ROM
- ✅ Activar el hardware de teclado Subor (Device ID 1539)
- ✅ Permitir paso de eventos nativos al canvas
- ✅ Sin modificar ROM, sin recompilar WASM, sin fork

---

## 🧪 Prueba Rápida (30 Segundos)

```bash
# 1. Abrir navegador
# 2. Ir a la página del emulador NES
# 3. Cargar pcvivaz-unif.nes
# 4. Esperar que aparezca el menú

# 5. Probar:
Presionar: ↓     → ¿Se mueve el cursor?
Presionar: Enter → ¿Entra en la app?
Escribir: HOLA   → ¿Aparece texto?

# Si CUALQUIER cosa funciona → ✅ ÉXITO
```

---

## 📊 Matriz de Documentación

| Documento | Usuario | Dev Junior | Dev Senior | Investigador |
|-----------|---------|------------|------------|--------------|
| Quick Reference | ⭐⭐⭐ | ⭐⭐ | ⭐ | - |
| FASE1 Fix | ⭐ | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐ |
| Arquitectura Input | - | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ |
| Hardware y Mapeo | - | ⭐ | ⭐⭐ | ⭐⭐⭐ |
| CHANGELOG | ⭐ | ⭐⭐ | ⭐⭐ | ⭐ |

---

## 🚨 Troubleshooting Rápido

### ❓ "No funciona nada"
→ Lee: [`Quick Reference — Troubleshooting`](PC_VIVAZ_QUICK_REFERENCE.md#-troubleshooting)

### ❓ "Quiero entender el fix"
→ Lee: [`FASE1 Fix — Diagnóstico`](PC_VIVAZ_FASE1_FIX_INPUT.md#-diagnóstico-técnico-profundo)

### ❓ "¿Cómo funciona la matriz de teclado?"
→ Lee: [`Arquitectura — Matriz Subor`](PC_VIVAZ_ARQUITECTURA_INPUT.md#-matriz-del-teclado-subor)

### ❓ "¿Qué es el Device ID 1539?"
→ Lee: [`Arquitectura — Device IDs`](PC_VIVAZ_ARQUITECTURA_INPUT.md#-identificadores-de-dispositivos-libretro)

### ❓ "¿Por qué preventDefault() era el problema?"
→ Lee: [`FASE1 Fix — Causa 2`](PC_VIVAZ_FASE1_FIX_INPUT.md#-causa-2-bloqueo-total-de-eventos-dom)

---

## 🔗 Enlaces Externos

### Recursos Oficiales
- [Libretro Docs](https://docs.libretro.com/) — API de RetroArch
- [FCEUmm GitHub](https://github.com/libretro/libretro-fceumm) — Core de NES
- [NESdev Wiki](https://wiki.nesdev.com/) — Documentación de hardware NES

### Comunidad
- [Famiclones Discord](https://discord.gg/famiclones) — Comunidad de clones de Famicom
- [NESdev Forums](https://forums.nesdev.com/) — Foros de desarrollo para NES

### Herramientas
- [Mesen](https://www.mesen.ca/) — Emulador de alta precisión (referencia)
- [FCEUX](http://fceux.com/) — Emulador con debugging avanzado

---

## 📞 Contacto y Contribuciones

### Reportar Bugs
1. Verificar checklist en Quick Reference
2. Capturar logs de DevTools Console
3. Capturar screenshot de la pantalla
4. Reportar con toda la info

### Contribuir Código
1. Leer `FASE1_FIX_INPUT.md` para entender el contexto
2. Leer `ARQUITECTURA_INPUT.md` para entender el stack
3. Seguir la filosofía: cambios mínimos y quirúrgicos
4. Documentar todo cambio en CHANGELOG.md

### Agregar Documentación
- Mantener el estilo técnico pero accesible
- Incluir ejemplos de código
- Agregar diagramas cuando sea posible
- Actualizar este README con nuevos docs

---

## 📜 Licencia y Créditos

### Créditos
- **ROM PC Vivaz:** Subor / Zhongshan Xiaoyan Industrial Co.
- **FCEUmm Core:** Equipo Libretro
- **RetroArch:** Equipo Libretro
- **Documentación y Fix:** Equipo Nightmare Edition (2026)

### Agradecimientos
- Comunidad NESdev por la documentación de hardware
- Comunidad Famiclones por preservar el conocimiento
- Emscripten por hacer posible WASM en el navegador

---

## 🎯 Próximos Pasos

### Inmediato (Ahora)
1. ✅ Validar MVP con prueba de 30 segundos
2. 📊 Reportar resultados (qué funciona, qué no)
3. 🐛 Ajustar si es necesario

### Corto Plazo (Días)
1. 📱 Planificar Fase 2: Gamepad virtual adaptado
2. 🖱️ Evaluar opciones para mouse
3. 📚 Documentar casos de uso exitosos

### Mediano Plazo (Semanas)
1. 🎮 Implementar SuborKeyboardAdapter
2. 📄 Soporte de persistencia SRAM (.srm files)
3. 🔧 Refinamientos de UX

### Largo Plazo (Meses)
1. 🖱️ Decidir estrategia de mouse (fork/Mesen/workaround)
2. 📦 Empaquetar como PWA standalone
3. 🌐 Integración completa en Retro Hub

---

**Última actualización:** 5 de octubre de 2026  
**Mantenido por:** Equipo de Documentación Técnica  
**Versión del Documento:** 1.0
