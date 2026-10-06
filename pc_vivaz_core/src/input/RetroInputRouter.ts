/**
 * RetroInputRouter.ts
 * 
 * Enrutador central de entrada para Retro Hub.
 * Permite cambiar dinámicamente entre Modo Consola (Joypads) y Modo 8-Bit Computer (PC Vivaz, Subor, etc.)
 */

import { InputAdapter, NESGamepadAdapter } from './NESGamepadAdapter';
import { PCVivazInputAdapter, PCVivazAdapterOptions } from './PCVivazInputAdapter';

export type SystemInputMode = 'console_nes' | 'pc_vivaz' | 'custom';

export class RetroInputRouter {
  private activeAdapter: InputAdapter | null = null;
  private canvas: HTMLCanvasElement | null = null;
  private emulatorInstance: any = null;

  constructor(emulatorInstance?: any, canvas?: HTMLCanvasElement) {
    if (emulatorInstance) this.emulatorInstance = emulatorInstance;
    if (canvas) this.canvas = canvas;
  }

  public setEmulatorInstance(instance: any): void {
    this.emulatorInstance = instance;
  }

  public setCanvas(canvas: HTMLCanvasElement): void {
    const wasAttached = this.activeAdapter !== null;
    if (wasAttached) {
      this.activeAdapter?.detach();
    }
    this.canvas = canvas;
    if (wasAttached && this.canvas) {
      this.activeAdapter?.attach(this.canvas);
    }
  }

  /**
   * Cambia el modo de entrada del sistema
   */
  public setMode(mode: SystemInputMode, options: { vivazOptions?: PCVivazAdapterOptions } = {}): InputAdapter {
    if (this.activeAdapter) {
      this.activeAdapter.detach();
      this.activeAdapter = null;
    }

    switch (mode) {
      case 'pc_vivaz':
        this.activeAdapter = new PCVivazInputAdapter(this.emulatorInstance, options.vivazOptions);
        break;
      case 'console_nes':
      default:
        this.activeAdapter = new NESGamepadAdapter(this.emulatorInstance);
        break;
    }

    if (this.canvas) {
      this.activeAdapter.attach(this.canvas);
    }

    return this.activeAdapter;
  }

  /**
   * Carga un adaptador personalizado
   */
  public setCustomAdapter(adapter: InputAdapter): void {
    if (this.activeAdapter) {
      this.activeAdapter.detach();
    }
    this.activeAdapter = adapter;
    if (this.canvas) {
      this.activeAdapter.attach(this.canvas);
    }
  }

  /**
   * Devuelve la configuración de dispositivos que debe pasarse al core Libretro al iniciar la ROM
   */
  public getRequiredLibretroConfig(): Record<string, number> {
    if (!this.activeAdapter) {
      return {
        input_libretro_device_p1: 1,
        input_libretro_device_p2: 1
      };
    }
    return this.activeAdapter.getLibretroDeviceConfig();
  }

  public getActiveAdapter(): InputAdapter | null {
    return this.activeAdapter;
  }

  public destroy(): void {
    if (this.activeAdapter) {
      this.activeAdapter.detach();
      this.activeAdapter = null;
    }
    this.canvas = null;
    this.emulatorInstance = null;
  }
}
