/**
 * PCVivazInputAdapter.ts
 * 
 * Adaptador de entrada especializado para PC VIVAZ y sistemas Famiclone Educativos (Subor).
 * 
 * Configuración de hardware:
 * - Puerto 1: Deshabilitado o Joypad secundario.
 * - Puerto 2: Subor Mouse (Serie $4017).
 * - Puerto de Expansión (p5): RETRO_DEVICE_FC_SUBORKB (ID 1539).
 * 
 * Características:
 * 1. Mapeo de teclado completo QWERTY (A-Z, 0-9, F1-F12, símbolos, modificadores).
 * 2. Supresión de atajos globales del navegador (Game Focus mode).
 * 3. Captura de ratón con Pointer Lock API y factor de escala para evitar overflow en el bus de 8 bits.
 * 4. Dispatch a Emscripten JSEvents / RetroArch Keyboard Driver.
 */

import { InputAdapter } from './NESGamepadAdapter';
import { lookupRetroKey } from './SuborKeyMap';

export interface PCVivazAdapterOptions {
  mouseSensitivity?: number; // Factor de escala (def: 0.4x)
  suppressBrowserShortcuts?: boolean;
  onKeyActivity?: (code: string, pressed: boolean) => void;
  onMouseActivity?: (dx: number, dy: number, left: boolean, right: boolean) => void;
}

export class PCVivazInputAdapter implements InputAdapter {
  public readonly id = 'pc_vivaz';
  public readonly name = 'PC VIVAZ (Subor Keyboard 1539 + Subor Mouse)';

  private canvas: HTMLCanvasElement | null = null;
  private isPointerLocked = false;
  private mouseSensitivity: number;
  private suppressShortcuts: boolean;

  // Estado del ratón
  private mouseState = {
    dx: 0,
    dy: 0,
    btnLeft: false,
    btnRight: false,
    btnMiddle: false
  };

  constructor(
    private emulatorInstance: any,
    private options: PCVivazAdapterOptions = {}
  ) {
    this.mouseSensitivity = options.mouseSensitivity ?? 0.4;
    this.suppressShortcuts = options.suppressBrowserShortcuts ?? true;
  }

  /**
   * Configuración de puertos requerida para Libretro / FCEUmm / Mesen
   */
  public getLibretroDeviceConfig(): Record<string, number> {
    return {
      input_libretro_device_p1: 1,    // Joypad estándar
      input_libretro_device_p2: 2,    // RETRO_DEVICE_MOUSE (Subor Mouse)
      input_libretro_device_p5: 1539  // RETRO_DEVICE_FC_SUBORKB ((6 << 8) | 3)
    };
  }

  public attach(canvas: HTMLCanvasElement): void {
    this.canvas = canvas;

    // 1. Pointer Lock al hacer click en el canvas
    this.canvas.addEventListener('click', this.handleCanvasClick);
    document.addEventListener('pointerlockchange', this.handlePointerLockChange);

    // 2. Eventos de Teclado Globales (Game Focus)
    window.addEventListener('keydown', this.handleKeyDown, { passive: false });
    window.addEventListener('keyup', this.handleKeyUp, { passive: false });

    // 3. Eventos de Ratón Relativos
    window.addEventListener('mousemove', this.handleMouseMove);
    window.addEventListener('mousedown', this.handleMouseDown);
    window.addEventListener('mouseup', this.handleMouseUp);
    window.addEventListener('contextmenu', this.handleContextMenu);
  }

  public detach(): void {
    if (this.canvas) {
      this.canvas.removeEventListener('click', this.handleCanvasClick);
    }
    document.removeEventListener('pointerlockchange', this.handlePointerLockChange);

    window.removeEventListener('keydown', this.handleKeyDown);
    window.removeEventListener('keyup', this.handleKeyUp);

    window.removeEventListener('mousemove', this.handleMouseMove);
    window.removeEventListener('mousedown', this.handleMouseDown);
    window.removeEventListener('mouseup', this.handleMouseUp);
    window.removeEventListener('contextmenu', this.handleContextMenu);

    if (this.isPointerLocked && document.exitPointerLock) {
      document.exitPointerLock();
    }
    this.canvas = null;
  }

  // --- Manejo de Teclado ---

  private handleKeyDown = (e: KeyboardEvent) => {
    // Evitar que teclas críticas hagan scroll o cambien de pestaña
    if (this.suppressShortcuts) {
      if (['Space', 'Backspace', 'Tab', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'F1', 'F2', 'F3', 'F4', 'F5', 'F11'].includes(e.code)) {
        e.preventDefault();
      }
    }

    const retroKey = lookupRetroKey(e);
    if (retroKey !== undefined) {
      this.dispatchKeyToEmulator(retroKey, e.code, true);
    }
    this.options.onKeyActivity?.(e.code, true);
  };

  private handleKeyUp = (e: KeyboardEvent) => {
    const retroKey = lookupRetroKey(e);
    if (retroKey !== undefined) {
      this.dispatchKeyToEmulator(retroKey, e.code, false);
    }
    this.options.onKeyActivity?.(e.code, false);
  };

  /**
   * Inyecta la tecla tanto a la capa de Emscripten como a los hooks del core Libretro
   */
  private dispatchKeyToEmulator(retroKey: number, domCode: string, isDown: boolean): void {
    // 1. Si el emulador expone API directa de teclado
    if (typeof this.emulatorInstance?.sendKeyboardEvent === 'function') {
      this.emulatorInstance.sendKeyboardEvent(retroKey, isDown);
      return;
    }

    // 2. Acceso al módulo Emscripten subyacente (JSEvents / GLFW)
    const emModule = typeof this.emulatorInstance?.getEmscriptenModule === 'function' 
      ? this.emulatorInstance.getEmscriptenModule() 
      : null;

    if (emModule) {
      // Inyección en la tabla interna de teclado de RetroArch
      if (typeof emModule._rarch_keyboard_event === 'function') {
        emModule._rarch_keyboard_event(isDown ? 1 : 0, retroKey, 0, 0, 0);
      }
    }

    // 3. Fallback: Despachar KeyboardEvent sintético sobre el canvas para el hook de Emscripten
    if (this.canvas) {
      const syntheticEvent = new KeyboardEvent(isDown ? 'keydown' : 'keyup', {
        code: domCode,
        key: domCode.replace(/^Key|^Digit/, ''),
        which: retroKey,
        keyCode: retroKey,
        bubbles: true,
        cancelable: true
      });
      this.canvas.dispatchEvent(syntheticEvent);
    }
  }

  // --- Manejo de Ratón ---

  private handleMouseMove = (e: MouseEvent) => {
    if (!this.isPointerLocked) return;

    // Aplicar sensibilidad y normalización
    const scaledX = Math.round(e.movementX * this.mouseSensitivity);
    const scaledY = Math.round(e.movementY * this.mouseSensitivity);

    this.mouseState.dx = scaledX;
    this.mouseState.dy = scaledY;

    this.dispatchMouseToEmulator(scaledX, scaledY);
    this.options.onMouseActivity?.(scaledX, scaledY, this.mouseState.btnLeft, this.mouseState.btnRight);
  };

  private handleMouseDown = (e: MouseEvent) => {
    if (e.button === 0) this.mouseState.btnLeft = true;
    if (e.button === 2) this.mouseState.btnRight = true;
    if (e.button === 1) this.mouseState.btnMiddle = true;

    this.dispatchMouseButtons();
    this.options.onMouseActivity?.(0, 0, this.mouseState.btnLeft, this.mouseState.btnRight);
  };

  private handleMouseUp = (e: MouseEvent) => {
    if (e.button === 0) this.mouseState.btnLeft = false;
    if (e.button === 2) this.mouseState.btnRight = false;
    if (e.button === 1) this.mouseState.btnMiddle = false;

    this.dispatchMouseButtons();
    this.options.onMouseActivity?.(0, 0, this.mouseState.btnLeft, this.mouseState.btnRight);
  };

  private dispatchMouseToEmulator(dx: number, dy: number): void {
    if (typeof this.emulatorInstance?.sendMouseDeltas === 'function') {
      this.emulatorInstance.sendMouseDeltas(dx, dy);
      return;
    }

    const emModule = typeof this.emulatorInstance?.getEmscriptenModule === 'function' 
      ? this.emulatorInstance.getEmscriptenModule() 
      : null;

    if (emModule && typeof emModule._retro_mouse_delta === 'function') {
      emModule._retro_mouse_delta(1 /* Port 2 */, dx, dy);
    }
  }

  private dispatchMouseButtons(): void {
    if (typeof this.emulatorInstance?.sendMouseButtons === 'function') {
      this.emulatorInstance.sendMouseButtons(this.mouseState.btnLeft, this.mouseState.btnRight);
      return;
    }

    const emModule = typeof this.emulatorInstance?.getEmscriptenModule === 'function' 
      ? this.emulatorInstance.getEmscriptenModule() 
      : null;

    if (emModule && typeof emModule._retro_mouse_buttons === 'function') {
      let mask = 0;
      if (this.mouseState.btnLeft) mask |= 1;
      if (this.mouseState.btnRight) mask |= 2;
      emModule._retro_mouse_buttons(1 /* Port 2 */, mask);
    }
  }

  // --- Pointer Lock Helpers ---

  private handleCanvasClick = async () => {
    if (this.canvas && document.pointerLockElement !== this.canvas) {
      try {
        await this.canvas.requestPointerLock();
      } catch (err) {
        console.warn('Pointer lock denegado o no disponible:', err);
      }
    }
  };

  private handlePointerLockChange = () => {
    this.isPointerLocked = document.pointerLockElement === this.canvas;
  };

  private handleContextMenu = (e: MouseEvent) => {
    if (this.isPointerLocked) {
      e.preventDefault(); // Permitir clic derecho en PC Vivaz sin abrir menú del navegador
    }
  };
}
