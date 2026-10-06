/**
 * SRAMPersistenceManager.ts
 * 
 * Gestor de almacenamiento persistente respaldado por IndexedDB.
 * Guarda la memoria SRAM (8KB - 32KB) de la PC VIVAZ donde se almacenan
 * los textos escritos en el procesador de palabras, las hojas de cálculo
 * y los programas de usuario.
 */

export interface PersistenceOptions {
  dbName?: string;
  storeName?: string;
  autoSaveIntervalMs?: number; // Guardado automático periódico (def: 15 seg)
}

export class SRAMPersistenceManager {
  private dbName: string;
  private storeName: string;
  private db: IDBDatabase | null = null;
  private autoSaveTimer: any = null;
  private autoSaveIntervalMs: number;
  private emulatorInstance: any = null;
  private currentRomHashOrId: string = 'pc_vivaz_default';

  constructor(options: PersistenceOptions = {}) {
    this.dbName = options.dbName ?? 'RetroHub_Storage_v1';
    this.storeName = options.storeName ?? 'nes_sram_saves';
    this.autoSaveIntervalMs = options.autoSaveIntervalMs ?? 15000;
  }

  public async init(emulatorInstance: any, romIdentifier: string): Promise<void> {
    this.emulatorInstance = emulatorInstance;
    this.currentRomHashOrId = romIdentifier;

    await this.openDatabase();
    await this.restoreSRAM();

    this.startAutoSave();

    // Guardado de emergencia al cerrar la pestaña
    window.addEventListener('beforeunload', this.handleBeforeUnload);
  }

  private openDatabase(): Promise<IDBDatabase> {
    return new Promise((resolve, reject) => {
      const request = indexedDB.open(this.dbName, 1);

      request.onupgradeneeded = (e: any) => {
        const db = e.target.result;
        if (!db.objectStoreNames.contains(this.storeName)) {
          db.createObjectStore(this.storeName, { keyPath: 'romId' });
        }
      };

      request.onsuccess = () => {
        this.db = request.result;
        resolve(this.db);
      };

      request.onerror = () => {
        console.error('Error al abrir IndexedDB:', request.error);
        reject(request.error);
      };
    });
  }

  /**
   * Guarda el buffer de SRAM actual en IndexedDB
   */
  public async saveSRAM(): Promise<boolean> {
    if (!this.db || !this.emulatorInstance) return false;

    let sramData: Uint8Array | null = null;

    // Obtener SRAM del emulador
    if (typeof this.emulatorInstance.getSaveData === 'function') {
      sramData = await this.emulatorInstance.getSaveData();
    } else if (typeof this.emulatorInstance.getEmscriptenModule === 'function') {
      const em = this.emulatorInstance.getEmscriptenModule();
      if (em?.FS && em.FS.analyzePath('/retroarch/userdata/saves/pc_vivaz.srm').exists) {
        sramData = em.FS.readFile('/retroarch/userdata/saves/pc_vivaz.srm');
      }
    }

    if (!sramData || sramData.length === 0) return false;

    return new Promise((resolve, reject) => {
      const transaction = this.db!.transaction([this.storeName], 'readwrite');
      const store = transaction.objectStore(this.storeName);

      const record = {
        romId: this.currentRomHashOrId,
        sram: sramData,
        timestamp: Date.now()
      };

      const request = store.put(record);

      request.onsuccess = () => resolve(true);
      request.onerror = () => {
        console.error('Error al guardar SRAM en IndexedDB:', request.error);
        reject(request.error);
      };
    });
  }

  /**
   * Restaura la SRAM previamente guardada
   */
  public async restoreSRAM(): Promise<Uint8Array | null> {
    if (!this.db || !this.emulatorInstance) return null;

    return new Promise((resolve) => {
      const transaction = this.db!.transaction([this.storeName], 'readonly');
      const store = transaction.objectStore(this.storeName);
      const request = store.get(this.currentRomHashOrId);

      request.onsuccess = () => {
        if (request.result && request.result.sram) {
          const sramData: Uint8Array = request.result.sram;
          if (typeof this.emulatorInstance.loadSaveData === 'function') {
            this.emulatorInstance.loadSaveData(sramData);
          }
          resolve(sramData);
        } else {
          resolve(null);
        }
      };

      request.onerror = () => {
        console.warn('No se encontró SRAM previa para esta ROM.');
        resolve(null);
      };
    });
  }

  /**
   * Exporta la SRAM como archivo binario descargable (.sav)
   */
  public async exportSaveFile(): Promise<void> {
    await this.saveSRAM();
    const data = await this.restoreSRAM();
    if (!data) {
      alert('No hay datos de guardado disponibles para exportar.');
      return;
    }

    const blob = new Blob([data as any], { type: 'application/octet-stream' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `${this.currentRomHashOrId}_backup.sav`;
    a.click();
    URL.revokeObjectURL(url);
  }

  private startAutoSave(): void {
    this.stopAutoSave();
    this.autoSaveTimer = setInterval(() => {
      this.saveSRAM().catch(err => console.warn('AutoSave fallo:', err));
    }, this.autoSaveIntervalMs);
  }

  private stopAutoSave(): void {
    if (this.autoSaveTimer) {
      clearInterval(this.autoSaveTimer);
      this.autoSaveTimer = null;
    }
  }

  private handleBeforeUnload = () => {
    this.saveSRAM();
  };

  public destroy(): void {
    this.stopAutoSave();
    window.removeEventListener('beforeunload', this.handleBeforeUnload);
    this.saveSRAM();
    this.db = null;
    this.emulatorInstance = null;
  }
}
