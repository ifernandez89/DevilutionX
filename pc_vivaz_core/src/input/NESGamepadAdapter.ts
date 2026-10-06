/**
 * NESGamepadAdapter.ts
 * 
 * Adaptador de entrada para juegos de consola estándar de NES.
 * Mapea controles tradicionales (D-Pad + 4 Botones) hacia RETRO_DEVICE_JOYPAD en Puerto 1 / Puerto 2.
 */

export interface InputAdapter {
  id: string;
  name: string;
  attach(canvas: HTMLCanvasElement): void;
  detach(): void;
  getLibretroDeviceConfig(): Record<string, number>;
}

export interface GamepadButtonMapping {
  up: string[];
  down: string[];
  left: string[];
  right: string[];
  a: string[];
  b: string[];
  select: string[];
  start: string[];
}

export const DEFAULT_NES_KEYMAP: GamepadButtonMapping = {
  up: ['ArrowUp', 'KeyW'],
  down: ['ArrowDown', 'KeyS'],
  left: ['ArrowLeft', 'KeyA'],
  right: ['ArrowRight', 'KeyD'],
  a: ['KeyX', 'KeyK'],
  b: ['KeyZ', 'KeyJ'],
  select: ['ShiftRight', 'Digit9'],
  start: ['Enter', 'Digit0']
};

export class NESGamepadAdapter implements InputAdapter {
  public readonly id = 'nes_gamepad';
  public readonly name = 'Consola NES Estándar (Gamepad)';

  private canvas: HTMLCanvasElement | null = null;
  private keyMap: GamepadButtonMapping;

  constructor(
    private emulatorInstance: any,
    customKeyMap: Partial<GamepadButtonMapping> = {}
  ) {
    this.keyMap = { ...DEFAULT_NES_KEYMAP, ...customKeyMap };
  }

  public getLibretroDeviceConfig(): Record<string, number> {
    return {
      input_libretro_device_p1: 1, // RETRO_DEVICE_JOYPAD
      input_libretro_device_p2: 1  // RETRO_DEVICE_JOYPAD
    };
  }

  public attach(canvas: HTMLCanvasElement): void {
    this.canvas = canvas;
    window.addEventListener('keydown', this.handleKeyDown, { passive: false });
    window.addEventListener('keyup', this.handleKeyUp, { passive: false });
  }

  public detach(): void {
    window.removeEventListener('keydown', this.handleKeyDown);
    window.removeEventListener('keyup', this.handleKeyUp);
    this.canvas = null;
  }

  private handleKeyDown = (e: KeyboardEvent) => {
    const btn = this.findButtonForCode(e.code);
    if (btn) {
      e.preventDefault();
      if (typeof this.emulatorInstance?.pressDown === 'function') {
        this.emulatorInstance.pressDown(btn, 0);
      }
    }
  };

  private handleKeyUp = (e: KeyboardEvent) => {
    const btn = this.findButtonForCode(e.code);
    if (btn) {
      e.preventDefault();
      if (typeof this.emulatorInstance?.pressUp === 'function') {
        this.emulatorInstance.pressUp(btn, 0);
      }
    }
  };

  private findButtonForCode(code: string): string | null {
    for (const [btnName, codes] of Object.entries(this.keyMap)) {
      if (codes.includes(code)) return btnName;
    }
    return null;
  }
}
