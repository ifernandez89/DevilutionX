# 🖥️ PC VIVAZ & 8-Bit Computer Mode — Arquitectura e Integración

Este módulo implementa el soporte completo de periféricos y persistencia para la **PC VIVAZ** (Famiclone Educativo Subor/Nikita) y futuras computadoras de 8 bits en **Retro Hub**.

---

## 📐 Arquitectura de Entrada

El sistema desacopla los eventos del navegador (`KeyboardEvent`, `MouseEvent`) del núcleo del emulador a través de un **Enrutador de Entrada (`RetroInputRouter`)** y adaptadores especializados:

```
                  [ Eventos DOM Browser ]
                             │
                             ▼
                  [ RetroInputRouter ]
                             │
        ┌────────────────────┴────────────────────┐
        ▼                                         ▼
[ NESGamepadAdapter ]                   [ PCVivazInputAdapter ]
(Consola Estándar)                      (8-Bit Computer Mode)
• Joypad 1 & 2                          • Subor Keyboard Matrix (1539)
• D-Pad + 4 Botones                     • Subor Mouse Serial (Port 2)
                                        • Pointer Lock API
                                        • Game Focus (Full QWERTY)
```

---

## 🔌 Especificación de Puertos Libretro (FCEUmm / Mesen)

Para que el core FCEUmm habilite la matriz Subor y el ratón en la web, se configuran los siguientes descriptores:

| Puerto | Dispositivo Libretro | ID de Dispositivo | Explicación |
| :--- | :--- | :--- | :--- |
| **Port 1** | `RETRO_DEVICE_JOYPAD` | `1` | Mando primario / Deshabilitado en SO |
| **Port 2** | `RETRO_DEVICE_MOUSE` | `2` | **Subor Mouse** (Lecturas serie en `$4017`) |
| **Port 5 (Exp.)** | `RETRO_DEVICE_FC_SUBORKB` | **`1539`** | **Subor Keyboard** (Subclase 6: `(6 << 8) \| 3 = 1539`) |

---

## 🚀 Guía Rápida de Uso en Retro Hub

```typescript
import { RetroInputRouter, SRAMPersistenceManager } from './pc_vivaz_core/src';

// 1. Instanciar el enrutador
const inputRouter = new RetroInputRouter();

// 2. Configurar el modo antes de lanzar el core
inputRouter.setMode('pc_vivaz', {
  vivazOptions: {
    mouseSensitivity: 0.4, // Factor de escala para el cursor en resolución NES
    suppressBrowserShortcuts: true
  }
});

// 3. Obtener la configuración requerida para Nostalgist / Libretro
const deviceConfig = inputRouter.getRequiredLibretroConfig();

// 4. Lanzar el emulador
const nostalgist = await Nostalgist.launch({
  core: 'fceumm',
  rom: romFile,
  retroarchConfig: deviceConfig
});

// 5. Vincular instancia y canvas
inputRouter.setEmulatorInstance(nostalgist);
inputRouter.setCanvas(document.querySelector('canvas')!);

// 6. Inicializar persistencia de SRAM (Textos, Hojas de Cálculo)
const sramManager = new SRAMPersistenceManager();
await sramManager.init(nostalgist, 'pc_vivaz_rom');
```

---

## 🧪 Procedimiento de Testing Local

1. **Prueba de Escritura (Teclado Subor):**
   * Hacer clic en el canvas para asegurar el foco del DOM.
   * Abrir el procesador de palabras o BASIC de la PC Vivaz.
   * Escribir `A-Z`, números y presionar `Enter` y `Espacio`. Verificar que el texto aparece sin retraso.

2. **Prueba de Navegación (Ratón Subor):**
   * Al hacer clic en el canvas, el navegador activará el **Pointer Lock**.
   * Mover el ratón físico: el cursor en pantalla debe seguir el movimiento con fluidez.
   * Realizar clic izquierdo y derecho para abrir menús y seleccionar íconos.

3. **Prueba de Persistencia (IndexedDB):**
   * Escribir un párrafo en el procesador de texto.
   * Recargar la página (`F5`).
   * Al reiniciar la ROM, el texto debe restaurarse automáticamente desde la SRAM de IndexedDB.
