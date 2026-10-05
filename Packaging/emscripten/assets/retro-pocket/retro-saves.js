/**
 * RetroHub Save Manager — Sega Genesis / NES / SNES (Nostalgist + RetroArch WASM)
 *
 * Why this exists (see RETRO_EMULATORS_SRAM_FIX.md, "V2"):
 *  - Nostalgist uses ONE virtual file (<rom>.state) for both saveState() and loadState().
 *    Overlapping operations (periodic autosave + manual save/load) deleted that file before
 *    RetroArch could read it, so "Cargar" silently did nothing (e.g. Road Rash 3).
 *  - saveSRAM() polls the FS for ~60s when a game has no battery RAM (Road Rash 3 uses
 *    passwords), blocking every save flow behind it.
 *
 * This module serializes every emulator FS operation through a single queue, applies short
 * timeouts, remembers per-session whether the cartridge has SRAM, and exposes
 * download / import of real, emulator-compatible save files (.srm battery / .state snapshot).
 */
(() => {
    'use strict';

    const ROM_EXT_RE = /\.(bin|smd|gen|md|zip|7z|nes|fds|unf|sfc|smc|fig|swc)$/i;
    const SRAM_EXT_RE = /\.(srm|sav|sram)$/i;

    const delay = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

    function withTimeout(promise, ms, label) {
        let timer;
        return Promise.race([
            Promise.resolve(promise).finally(() => clearTimeout(timer)),
            new Promise((_, reject) => {
                timer = setTimeout(() => reject(new Error(`timeout:${label}`)), ms);
            })
        ]);
    }

    function romBaseName(name) {
        return String(name || 'juego')
            .replace(ROM_EXT_RE, '')
            .replace(/[\\/:*?"<>|]+/g, '_')
            .trim() || 'juego';
    }

    function triggerDownload(blob, fileName) {
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = fileName;
        a.rel = 'noopener';
        a.style.display = 'none';
        document.body.appendChild(a);
        a.click();
        setTimeout(() => {
            URL.revokeObjectURL(url);
            a.remove();
        }, 4000);
    }

    /**
     * @param {object} opts
     * @param {string} opts.system            'gens' | 'nes' | 'snes'
     * @param {() => any} opts.getEmulator    returns the live Nostalgist instance (or null)
     * @param {() => string} opts.getRomName  returns the active ROM name (IndexedDB key suffix)
     * @param {() => boolean} opts.isPaused   returns whether the user paused the game
     * @param {() => void} [opts.onResumed]   called when a load forces the game to resume
     * @param {(id:string, blob:Blob, label:string) => Promise<void>} opts.persist
     * @param {(id:string) => Promise<Blob|null>} opts.retrieve
     * @param {(msg:string, icon?:string, ms?:number) => void} opts.toast
     * @param {() => Promise<void>} [opts.relaunch]  relaunches the current ROM (used after .srm import)
     * @param {string} [opts.manualPrefix]    IndexedDB prefix for manual states (default `${system}_state_`)
     * @param {() => void} [opts.afterAction] e.g. refocus the canvas
     */
    function create(opts) {
        const system = opts.system;
        const keys = {
            manual: (rom) => `${opts.manualPrefix || `${system}_state_`}${rom}`,
            latest: (rom) => `${system}_latest_${rom}`,
            sram: (rom) => `${system}_sram_${rom}`
        };

        let queue = Promise.resolve();
        let busy = false;
        let sramSupported = null; // null = unknown, true / false after first probe
        let autosaveBlocked = false;
        let lastState = null;

        const toast = (msg, icon, ms) => { try { opts.toast(msg, icon, ms); } catch (_) {} };
        const after = () => { try { opts.afterAction && opts.afterAction(); } catch (_) {} };

        /** Serialize every FS-touching operation (save / load / SRAM). */
        function run(task) {
            const next = queue.then(async () => {
                busy = true;
                try {
                    return await task();
                } finally {
                    busy = false;
                }
            });
            queue = next.catch(() => {});
            return next;
        }

        /** RetroArch only processes commands while its main loop runs: unpause temporarily. */
        async function whileRunning(emu, fn) {
            const wasPaused = typeof opts.isPaused === 'function' && opts.isPaused();
            if (wasPaused) {
                try { emu.resume(); } catch (_) {}
                await delay(50);
            }
            try {
                return await fn();
            } finally {
                if (wasPaused) {
                    try { emu.pause(); } catch (_) {}
                }
            }
        }

        async function captureSRAM(emu) {
            if (sramSupported === false) return null;
            try {
                const blob = await withTimeout(emu.saveSRAM(), 3000, 'sram');
                if (blob && blob.size > 0) {
                    sramSupported = true;
                    return blob;
                }
            } catch (_) {
                // No battery RAM on this cartridge (e.g. Road Rash 3 uses passwords).
            }
            if (sramSupported === null) sramSupported = false;
            return null;
        }

        async function captureState(emu) {
            const result = await withTimeout(emu.saveState(), 10000, 'state');
            if (!result || !result.state || result.state.size === 0) {
                throw new Error('estado vacío');
            }
            return result.state;
        }

        function resetSession() {
            sramSupported = null;
            autosaveBlocked = false;
            lastState = null;
        }

        function setLastState(blob) {
            if (blob && blob.size > 0) lastState = blob;
        }

        async function saveNow() {
            const emu = opts.getEmulator();
            if (!emu) return false;
            const rom = opts.getRomName();
            toast('Guardando partida...', '💾', 1500);
            try {
                const { state, sram } = await run(() => whileRunning(emu, async () => {
                    const s = await captureState(emu);
                    const b = await captureSRAM(emu);
                    return { state: s, sram: b };
                }));
                lastState = state;
                await opts.persist(keys.manual(rom), state, `Manual - ${rom}`);
                await opts.persist(keys.latest(rom), state, `Último - ${rom}`);
                if (sram) await opts.persist(keys.sram(rom), sram, `Batería/SRAM - ${rom}`);
                try { localStorage.setItem(`${system}_save_${rom}`, new Date().toISOString()); } catch (_) {}
                toast(sram ? '¡Partida y batería guardadas!' : '¡Partida guardada!', '✅');
                after();
                return true;
            } catch (err) {
                console.error(`[${system} Save] Error guardando estado:`, err);
                toast('Error al guardar la partida. Intentá de nuevo.', '❌', 4500);
                after();
                return false;
            }
        }

        async function loadState(blob, { silent = false } = {}) {
            const emu = opts.getEmulator();
            if (!emu || !blob) return false;
            if (!silent) toast('Cargando partida...', '📂', 1500);
            try {
                await run(async () => {
                    // Always load on a running core: the state file is written to the virtual FS
                    // and RetroArch reads it on its next main-loop iteration.
                    if (typeof opts.isPaused === 'function' && opts.isPaused()) {
                        try { emu.resume(); } catch (_) {}
                        if (opts.onResumed) opts.onResumed();
                    }
                    await emu.loadState(blob);
                    // Keep the lock until RetroArch has consumed the file, so no other
                    // operation can delete <rom>.state underneath it.
                    await delay(450);
                });
                lastState = blob;
                if (!silent) toast('¡Partida restaurada!', '✅');
                after();
                return true;
            } catch (err) {
                console.error(`[${system} Load] Error cargando estado:`, err);
                toast('Error al restaurar la partida', '❌', 4500);
                after();
                return false;
            }
        }

        async function loadLatest() {
            if (!opts.getEmulator()) return false;
            const rom = opts.getRomName();
            const state = lastState ||
                await opts.retrieve(keys.manual(rom)) ||
                await opts.retrieve(keys.latest(rom));
            if (!state) {
                toast('No hay partida guardada para este juego', '⚠️');
                after();
                return false;
            }
            return loadState(state);
        }

        /**
         * Background autosave (SRAM + snapshot). Skips when another operation is running unless
         * `wait` is true (used on exit so nothing is lost).
         */
        async function autoSave({ includeState = true, wait = false } = {}) {
            const emu = opts.getEmulator();
            if (!emu || autosaveBlocked) return;
            if (busy && !wait) return;
            const rom = opts.getRomName();
            try {
                await withTimeout(run(async () => {
                    const sram = await captureSRAM(emu);
                    if (sram) await opts.persist(keys.sram(rom), sram, `Batería/SRAM - ${rom}`);
                    if (includeState) {
                        const state = await whileRunning(emu, () => captureState(emu));
                        lastState = state;
                        await opts.persist(keys.latest(rom), state, `Autosave - ${rom}`);
                    }
                }), 15000, 'autosave');
            } catch (err) {
                console.warn(`[${system} AutoSave]`, err);
            }
        }

        async function downloadSRAM() {
            const rom = opts.getRomName();
            const emu = opts.getEmulator();
            let blob = null;
            if (emu) {
                blob = await run(() => captureSRAM(emu)).catch(() => null);
                if (blob) await opts.persist(keys.sram(rom), blob, `Batería/SRAM - ${rom}`);
            }
            if (!blob) blob = await opts.retrieve(keys.sram(rom));
            if (!blob || blob.size === 0) {
                toast('Este juego no usa batería (SRAM). Descargá el estado rápido (.state).', 'ℹ️', 5500);
                return false;
            }
            triggerDownload(new Blob([blob], { type: 'application/octet-stream' }), `${romBaseName(rom)}.srm`);
            toast(`Batería descargada: ${romBaseName(rom)}.srm`, '⬇️');
            return true;
        }

        async function downloadState() {
            const rom = opts.getRomName();
            const emu = opts.getEmulator();
            let blob = null;
            if (emu) {
                try {
                    blob = await run(() => whileRunning(emu, () => captureState(emu)));
                    lastState = blob;
                    await opts.persist(keys.latest(rom), blob, `Último - ${rom}`);
                } catch (err) {
                    console.warn(`[${system} Download] captura en vivo falló:`, err);
                }
            }
            if (!blob) blob = lastState || await opts.retrieve(keys.manual(rom)) || await opts.retrieve(keys.latest(rom));
            if (!blob || blob.size === 0) {
                toast('No hay estado para descargar todavía', '⚠️');
                return false;
            }
            triggerDownload(new Blob([blob], { type: 'application/octet-stream' }), `${romBaseName(rom)}.state`);
            toast(`Estado descargado: ${romBaseName(rom)}.state`, '⬇️');
            return true;
        }

        async function importFile(file) {
            if (!file || file.size === 0) {
                toast('Archivo vacío o inválido', '⚠️');
                return false;
            }
            const rom = opts.getRomName();
            const blob = new Blob([await file.arrayBuffer()], { type: 'application/octet-stream' });

            if (SRAM_EXT_RE.test(file.name)) {
                // Battery saves are injected at boot: persist, block autosave so the old SRAM
                // can't overwrite it, then reboot the cartridge.
                autosaveBlocked = true;
                await opts.persist(keys.sram(rom), blob, `Importado - ${rom}`);
                if (opts.relaunch && opts.getEmulator()) {
                    toast('Batería importada. Reiniciando el cartucho...', '⬆️', 2500);
                    await opts.relaunch();
                } else {
                    autosaveBlocked = false;
                    toast('Batería importada. Se usará al iniciar el juego.', '⬆️');
                }
                return true;
            }

            // Anything else is treated as a RetroArch save state for the same core.
            if (!opts.getEmulator()) {
                toast('Iniciá el juego antes de importar un estado (.state)', '⚠️');
                return false;
            }
            const ok = await loadState(blob, { silent: true });
            if (ok) {
                await opts.persist(keys.manual(rom), blob, `Importado - ${rom}`);
                await opts.persist(keys.latest(rom), blob, `Importado - ${rom}`);
                toast('¡Estado importado y restaurado!', '✅');
            }
            return ok;
        }

        /** Wires the shared "Saves" popover if present in the page. */
        function bindDefaultUI() {
            const menuBtn = document.getElementById('saveMenuBtn');
            const menu = document.getElementById('saveMenu');
            const closeBtn = document.getElementById('saveMenuClose');
            const dlSram = document.getElementById('downloadSramBtn');
            const dlState = document.getElementById('downloadStateBtn');
            const importBtn = document.getElementById('importSaveBtn');
            const importInput = document.getElementById('importSaveInput');
            const romLabel = document.getElementById('saveMenuRom');
            if (!menuBtn || !menu) return;

            const open = () => {
                if (romLabel) romLabel.textContent = romBaseName(opts.getRomName());
                menu.hidden = false;
                menu.classList.add('open');
            };
            const close = () => {
                menu.classList.remove('open');
                menu.hidden = true;
                after();
            };

            menuBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                if (menu.hidden) open(); else close();
            });
            if (closeBtn) closeBtn.addEventListener('click', close);
            menu.addEventListener('click', (e) => {
                if (e.target === menu) close();
            });
            if (dlSram) dlSram.addEventListener('click', async () => { await downloadSRAM(); close(); });
            if (dlState) dlState.addEventListener('click', async () => { await downloadState(); close(); });
            if (importBtn && importInput) {
                importBtn.addEventListener('click', () => importInput.click());
                importInput.addEventListener('change', async () => {
                    const file = importInput.files && importInput.files[0];
                    importInput.value = '';
                    close();
                    if (file) await importFile(file);
                });
            }
        }

        // Save when the app goes to background (mobile tab switch / screen lock).
        document.addEventListener('visibilitychange', () => {
            if (document.visibilityState === 'hidden') autoSave({ includeState: true });
        });
        window.addEventListener('pagehide', () => autoSave({ includeState: false }));

        return {
            run,
            resetSession,
            setLastState,
            saveNow,
            loadState,
            loadLatest,
            autoSave,
            downloadSRAM,
            downloadState,
            importFile,
            bindDefaultUI,
            get busy() { return busy; },
            get sramSupported() { return sramSupported; }
        };
    }

    window.RetroSaves = { create, romBaseName };
})();
