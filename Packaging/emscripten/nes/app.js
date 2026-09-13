/**
 * NES Web — Nintendo 8-Bit & Famicom WebAssembly Controller
 * Powers 60 FPS cycle-accurate NES emulation with Nestopia / FCEUmm WASM cores,
 * WebAudio, USB/Bluetooth Gamepads, persistent offline IndexedDB saves, and CRT scanlines.
 */

(() => {
    'use strict';

    // State
    let currentEmulator = null;
    let isPaused = false;
    let crtFilterEnabled = false;
    let lastSavedState = null;
    let activeRomName = 'Juego NES';

    // DOM Elements
    const hubSection = document.getElementById('hub-section');
    const emulatorSection = document.getElementById('emulator-section');
    const screenCard = document.querySelector('.screen-card');
    const screenToolbar = document.querySelector('.screen-toolbar');
    const canvasContainer = document.getElementById('canvas-container');
    const scanlinesOverlay = document.getElementById('scanlines');
    const dropzone = document.getElementById('dropzone');
    const fileInput = document.getElementById('fileInput');
    const topUploadBtn = document.getElementById('topUploadBtn');
    const hubBrowseBtn = document.getElementById('hubBrowseBtn');

    // Controls
    const pauseResumeBtn = document.getElementById('pauseResumeBtn');
    const resetBtn = document.getElementById('resetBtn');
    const crtToggleBtn = document.getElementById('crtToggleBtn');
    const saveStateBtn = document.getElementById('saveStateBtn');
    const loadStateBtn = document.getElementById('loadStateBtn');
    const screenshotBtn = document.getElementById('screenshotBtn');
    const screenSizeBtn = document.getElementById('screenSizeBtn');
    const fullscreenBtn = document.getElementById('fullscreenBtn');
    const exitGameBtn = document.getElementById('exitGameBtn');

    // Gamepad indicator
    const gamepadDot = document.getElementById('gamepadDot');
    const gamepadText = document.getElementById('gamepadText');

    // Overlay & Toast
    const loadingOverlay = document.getElementById('loading-overlay');
    const loadingText = document.getElementById('loadingText');
    const loadingSubtext = document.getElementById('loadingSubtext');
    const toast = document.getElementById('toast');
    const toastIcon = document.getElementById('toastIcon');
    const toastMessage = document.getElementById('toastMessage');

    // Screen Sizes (Exact replica of Sega Genesis implementation)
    const screenSizes = [
        { name: 'Normal', className: 'size-normal', label: '📐 Tamaño: Normal (800p)' },
        { name: 'Grande', className: 'size-large', label: '📺 Tamaño: Grande (980p)' },
        { name: 'Cinema', className: 'size-cinema', label: '🎬 Tamaño: Cinema (1180p)' }
    ];
    let currentSizeIndex = 1; // Default: Grande (980px)

    function updateScreenSizeUI() {
        const size = screenSizes[currentSizeIndex];
        if (screenSizeBtn) screenSizeBtn.textContent = size.label;
        if (screenCard) {
            screenCard.classList.remove('size-normal', 'size-large', 'size-cinema');
            screenCard.classList.add(size.className);
        }
        if (screenToolbar) {
            screenToolbar.classList.remove('size-normal', 'size-large', 'size-cinema');
            screenToolbar.classList.add(size.className);
        }
    }

    // Helpers: Toast & Loading
    function showToast(msg, icon = 'ℹ️', duration = 3500) {
        if (!toast) return;
        toastIcon.textContent = icon;
        toastMessage.textContent = msg;
        toast.style.display = 'flex';
        clearTimeout(toast._timeout);
        toast._timeout = setTimeout(() => {
            toast.style.display = 'none';
        }, duration);
    }

    function showLoading(title, subtitle = '') {
        if (!loadingOverlay) return;
        loadingText.textContent = title;
        loadingSubtext.textContent = subtitle;
        loadingOverlay.style.display = 'flex';
    }

    function hideLoading() {
        if (!loadingOverlay) return;
        loadingOverlay.style.display = 'none';
    }

    // Focus canvas to prevent key stealing
    function focusGameCanvas() {
        if (document.activeElement && typeof document.activeElement.blur === 'function') {
            document.activeElement.blur();
        }
        const canvas = document.getElementById('nes-screen');
        if (canvas) {
            canvas.tabIndex = 0;
            canvas.focus();
        }
    }

    // ==========================================
    // IndexedDB Persistent Storage (100% Offline)
    // ==========================================
    const DB_NAME = 'RetroHub_Saves_v1';
    const DB_VERSION = 1;
    const STORE_NAME = 'saves';

    function openSavesDB() {
        return new Promise((resolve) => {
            if (!window.indexedDB) {
                resolve(null);
                return;
            }
            const req = indexedDB.open(DB_NAME, DB_VERSION);
            req.onupgradeneeded = (e) => {
                const db = e.target.result;
                if (!db.objectStoreNames.contains(STORE_NAME)) {
                    db.createObjectStore(STORE_NAME, { keyPath: 'id' });
                }
            };
            req.onsuccess = () => resolve(req.result);
            req.onerror = () => resolve(null);
        });
    }

    async function persistSaveState(id, blob, label = '') {
        try {
            const db = await openSavesDB();
            if (!db) return;
            const tx = db.transaction(STORE_NAME, 'readwrite');
            const store = tx.objectStore(STORE_NAME);
            const arrayBuffer = await blob.arrayBuffer();
            store.put({
                id,
                system: 'nes',
                data: arrayBuffer,
                label,
                timestamp: Date.now(),
                dateStr: new Date().toLocaleString()
            });
        } catch (e) {
            console.warn('[NES Save] Error guardando en IndexedDB:', e);
        }
    }

    async function retrieveSaveState(id) {
        try {
            const db = await openSavesDB();
            if (!db) return null;
            return new Promise((resolve) => {
                const tx = db.transaction(STORE_NAME, 'readonly');
                const store = tx.objectStore(STORE_NAME);
                const req = store.get(id);
                req.onsuccess = () => {
                    if (req.result && req.result.data) {
                        resolve(new Blob([req.result.data]));
                    } else {
                        resolve(null);
                    }
                };
                req.onerror = () => resolve(null);
            });
        } catch (e) {
            console.warn('[NES Save] Error recuperando de IndexedDB:', e);
            return null;
        }
    }

    async function autoSaveCurrentGame() {
        if (!currentEmulator || isPaused) return;
        try {
            const stateObj = await currentEmulator.saveState();
            if (stateObj && stateObj.state) {
                lastSavedState = stateObj.state;
                await persistSaveState(`nes_latest_${activeRomName}`, stateObj.state, `Autosave - ${activeRomName}`);
            }
        } catch (e) {}
    }

    // Auto-save on tab close / browser exit
    window.addEventListener('beforeunload', () => {
        autoSaveCurrentGame();
    });
    window.addEventListener('pagehide', () => {
        autoSaveCurrentGame();
    });

    // Periodic auto-save every 60 seconds
    setInterval(() => {
        autoSaveCurrentGame();
    }, 60000);

    // ==========================================
    // ROM Preparation & Launching
    // ==========================================
    async function prepareRomData(romSource, defaultName) {
        let arrayBuffer;
        let baseName = defaultName || 'game';

        if (typeof romSource === 'string') {
            baseName = romSource.split('/').pop() || defaultName;
            const res = await fetch(romSource);
            if (!res.ok) throw new Error(`No se pudo descargar la ROM (${res.status} ${res.statusText})`);
            arrayBuffer = await res.arrayBuffer();
        } else if (romSource instanceof File || romSource instanceof Blob) {
            baseName = romSource.name || defaultName;
            arrayBuffer = await romSource.arrayBuffer();
        } else if (romSource instanceof ArrayBuffer) {
            arrayBuffer = romSource;
        } else if (ArrayBuffer.isView(romSource)) {
            arrayBuffer = romSource.buffer;
        } else {
            throw new Error('Tipo de ROM desconocido');
        }

        return new File([arrayBuffer], baseName, { type: 'application/octet-stream' });
    }

    async function launchRom(romSource, romName) {
        activeRomName = romName;
        showLoading(`Iniciando ${romName}...`, 'Cargando motor Nestopia (WebAssembly)...');

        try {
            if (currentEmulator) {
                try {
                    await autoSaveCurrentGame();
                    await currentEmulator.exit();
                } catch (e) {}
                currentEmulator = null;
            }

            // Switch UI views
            hubSection.style.display = 'none';
            emulatorSection.style.display = 'flex';
            updateScreenSizeUI();

            // Prepare fresh canvas
            const oldCanvas = canvasContainer.querySelector('canvas');
            if (oldCanvas) oldCanvas.remove();

            const canvas = document.createElement('canvas');
            canvas.id = 'nes-screen';
            canvas.tabIndex = 0;
            canvasContainer.insertBefore(canvas, scanlinesOverlay);

            const romFile = await prepareRomData(romSource, romName);

            // RetroArch configuration for NES
            const retroarchConfig = {
                video_vsync: 'true',
                audio_enable: 'true',
                input_autodetect_enable: 'true',

                // D-Pad
                input_player1_up: 'up',
                input_player1_down: 'down',
                input_player1_left: 'left',
                input_player1_right: 'right',

                // NES Buttons: B -> 'z' / 'a', A -> 'x' / 's'
                // RetroPad Y/B and B/A mappings
                input_player1_y: 'z',
                input_player1_b: 'z',
                input_player1_a: 'x',
                input_player1_x: 'x',

                // Start & Select
                input_player1_start: 'enter',
                input_player1_select: 'shift'
            };

            // Launch with local Nestopia core first (100% Offline)
            try {
                currentEmulator = await Nostalgist.launch({
                    core: 'nestopia',
                    rom: romFile,
                    element: canvas,
                    resolveCoreJs() {
                        return 'core/nestopia_libretro.js';
                    },
                    resolveCoreWasm() {
                        return 'core/nestopia_libretro.wasm';
                    },
                    retroarchConfig
                });
            } catch (nestopiaErr) {
                console.warn('[NES WASM] Error con Nestopia local, intentando FCEUmm local:', nestopiaErr);
                try {
                    currentEmulator = await Nostalgist.launch({
                        core: 'fceumm',
                        rom: romFile,
                        element: canvas,
                        resolveCoreJs() {
                            return 'core/fceumm_libretro.js';
                        },
                        resolveCoreWasm() {
                            return 'core/fceumm_libretro.wasm';
                        },
                        retroarchConfig
                    });
                } catch (fceuErr) {
                    console.warn('[NES WASM] Error con núcleos locales, intentando fallback en línea:', fceuErr);
                    currentEmulator = await Nostalgist.launch({
                        core: 'fceumm',
                        rom: romFile,
                        element: canvas,
                        retroarchConfig
                    });
                }
            }

            isPaused = false;
            updatePauseBtnUI();
            hideLoading();
            showToast(`🔴 ${romName} en ejecución (60 FPS)`, '▶');

            // Focus canvas
            setTimeout(() => {
                focusGameCanvas();
            }, 100);

            // Check for persistent save in IndexedDB
            setTimeout(async () => {
                const existingSave = await retrieveSaveState(`nes_state_${romName}`) ||
                                     await retrieveSaveState(`nes_latest_${romName}`);
                if (existingSave) {
                    lastSavedState = existingSave;
                    showToast(`💾 Partida previa en IndexedDB detectada. Presioná F7 para continuar.`, 'ℹ️', 5000);
                }
            }, 800);

            emulatorSection.scrollIntoView({ behavior: 'smooth' });

        } catch (err) {
            console.error('[NES WASM] Error fatal lanzando emulación:', err);
            hideLoading();
            showToast(`Error al iniciar ${romName}: ${err.message}`, '❌', 5000);
            exitToHub();
        }
    }

    // Exit emulation to hub
    async function exitToHub() {
        if (currentEmulator) {
            showLoading('Guardando y saliendo...', 'Sincronizando estado en IndexedDB...');
            try {
                await autoSaveCurrentGame();
                await currentEmulator.exit();
            } catch (e) {}
            currentEmulator = null;
            hideLoading();
        }

        const canvas = canvasContainer.querySelector('canvas');
        if (canvas) canvas.remove();

        emulatorSection.style.display = 'none';
        hubSection.style.display = 'grid';
        showToast('Volviste a la selección de ROMs', '📂');
    }

    function updatePauseBtnUI() {
        if (isPaused) {
            pauseResumeBtn.textContent = '▶️ Continuar';
            pauseResumeBtn.classList.add('active');
        } else {
            pauseResumeBtn.textContent = '⏸️ Pausar';
            pauseResumeBtn.classList.remove('active');
        }
    }

    // ==========================================
    // Toolbar Event Listeners
    // ==========================================
    pauseResumeBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        if (isPaused) {
            await currentEmulator.resume();
            isPaused = false;
            showToast('Juego reanudado', '▶️');
        } else {
            await currentEmulator.pause();
            isPaused = true;
            showToast('Juego en pausa', '⏸️');
        }
        updatePauseBtnUI();
        focusGameCanvas();
    });

    resetBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        if (confirm(`¿Reiniciar ${activeRomName}?`)) {
            await currentEmulator.restart();
            isPaused = false;
            updatePauseBtnUI();
            showToast('Juego reiniciado', '🔄');
            focusGameCanvas();
        }
    });

    crtToggleBtn.addEventListener('click', () => {
        crtFilterEnabled = !crtFilterEnabled;
        if (crtFilterEnabled) {
            scanlinesOverlay.classList.add('active');
            crtToggleBtn.textContent = '📺 Scanlines: ON';
            crtToggleBtn.classList.add('active');
            showToast('Filtro CRT Scanlines activado', '📺');
        } else {
            scanlinesOverlay.classList.remove('active');
            crtToggleBtn.textContent = '📺 Scanlines: OFF';
            crtToggleBtn.classList.remove('active');
            showToast('Filtro CRT Scanlines desactivado', '📺');
        }
        focusGameCanvas();
    });

    saveStateBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        try {
            showToast('Guardando estado en IndexedDB...', '💾', 1500);
            const stateObj = await currentEmulator.saveState();
            lastSavedState = stateObj.state;
            await persistSaveState(`nes_state_${activeRomName}`, lastSavedState, `Manual F5 - ${activeRomName}`);
            await persistSaveState(`nes_latest_${activeRomName}`, lastSavedState, `Último - ${activeRomName}`);
            showToast('¡Estado guardado permanentemente! (F5)', '✅');
        } catch (err) {
            console.error('Error guardando estado:', err);
            showToast('Error al guardar estado', '❌');
        }
        focusGameCanvas();
    });

    loadStateBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        try {
            let stateToLoad = lastSavedState;
            if (!stateToLoad) {
                stateToLoad = await retrieveSaveState(`nes_state_${activeRomName}`) ||
                              await retrieveSaveState(`nes_latest_${activeRomName}`);
            }
            if (!stateToLoad) {
                showToast('No hay partida guardada en IndexedDB para este juego', '⚠️');
                focusGameCanvas();
                return;
            }
            showToast('Cargando estado desde IndexedDB...', '📂', 1500);
            await currentEmulator.loadState(stateToLoad);
            lastSavedState = stateToLoad;
            isPaused = false;
            updatePauseBtnUI();
            showToast('¡Estado restaurado con éxito!', '✅');
        } catch (err) {
            console.error('Error cargando estado:', err);
            showToast('Error al restaurar estado', '❌');
        }
        focusGameCanvas();
    });

    screenshotBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        try {
            const shot = await currentEmulator.screenshot();
            const a = document.createElement('a');
            a.href = shot;
            a.download = `nes_${activeRomName}_${Date.now()}.png`;
            a.click();
            showToast('Captura descargada', '📸');
        } catch (err) {
            console.error('Error tomando captura:', err);
            showToast('Error al tomar captura', '❌');
        }
        focusGameCanvas();
    });

    if (screenSizeBtn) {
        screenSizeBtn.addEventListener('click', () => {
            currentSizeIndex = (currentSizeIndex + 1) % screenSizes.length;
            updateScreenSizeUI();
            const size = screenSizes[currentSizeIndex];
            showToast(`Tamaño de pantalla: ${size.name}`, '📐');
            focusGameCanvas();
        });
    }

    fullscreenBtn.addEventListener('click', () => {
        if (!document.fullscreenElement) {
            if (canvasContainer.requestFullscreen) {
                canvasContainer.requestFullscreen();
            } else if (screenCard.requestFullscreen) {
                screenCard.requestFullscreen();
            }
        } else {
            if (document.exitFullscreen) document.exitFullscreen();
        }
        focusGameCanvas();
    });

    exitGameBtn.addEventListener('click', () => {
        if (confirm('¿Deseas salir del juego actual?')) {
            exitToHub();
        }
    });

    // Canvas click focus
    canvasContainer.addEventListener('click', focusGameCanvas);
    if (screenCard) screenCard.addEventListener('click', focusGameCanvas);

    // File selection & drop handlers
    topUploadBtn.addEventListener('click', () => fileInput.click());
    if (hubBrowseBtn) hubBrowseBtn.addEventListener('click', () => fileInput.click());
    dropzone.addEventListener('click', () => fileInput.click());

    fileInput.addEventListener('change', (e) => {
        const file = e.target.files[0];
        if (!file) return;
        handleSelectedRomFile(file);
    });

    dropzone.addEventListener('dragover', (e) => {
        e.preventDefault();
        e.stopPropagation();
        dropzone.classList.add('dragover');
    });

    dropzone.addEventListener('dragleave', (e) => {
        e.preventDefault();
        e.stopPropagation();
        dropzone.classList.remove('dragover');
    });

    dropzone.addEventListener('drop', (e) => {
        e.preventDefault();
        e.stopPropagation();
        dropzone.classList.remove('dragover');

        const files = e.dataTransfer.files;
        if (files && files.length > 0) {
            handleSelectedRomFile(files[0]);
        }
    });

    function handleSelectedRomFile(file) {
        const fileNameLower = file.name.toLowerCase();

        // Check if user accidentally dropped a Sega Genesis ROM
        if (fileNameLower.endsWith('.smd') || fileNameLower.endsWith('.gen') || fileNameLower.endsWith('.md')) {
            if (confirm(`Has seleccionado una ROM de Sega Genesis (${file.name}).\n¿Deseas abrir el Emulador de Genesis (16-Bit)?`)) {
                window.location.href = '../gens/index.html';
            }
            return;
        }

        const validExtensions = ['.nes', '.fds', '.unf', '.zip'];
        const isValid = validExtensions.some(ext => fileNameLower.endsWith(ext));

        if (!isValid) {
            showToast('Formato no compatible. Usa archivos .nes, .fds, .unf o .zip', '⚠️', 4000);
            return;
        }

        launchRom(file, file.name);
    }

    // Direct Dual-Keyboard Dispatcher for NES
    const KEY_ACTIONS = {
        'ArrowUp': 'up',
        'ArrowDown': 'down',
        'ArrowLeft': 'left',
        'ArrowRight': 'right',

        // NES B button: Z or A
        'KeyZ': 'b',
        'KeyA': 'b',

        // NES A button: X or S
        'KeyX': 'a',
        'KeyS': 'a',

        // Turbo buttons
        'KeyQ': 'y',
        'KeyW': 'x',

        // Start & Select
        'Enter': 'start',
        'ShiftLeft': 'select',
        'ShiftRight': 'select',
        'Tab': 'select'
    };

    window.addEventListener('keydown', (e) => {
        // Global hotkeys
        if (e.key === 'F5') {
            e.preventDefault();
            if (saveStateBtn) saveStateBtn.click();
            return;
        }
        if (e.key === 'F7') {
            e.preventDefault();
            if (loadStateBtn) loadStateBtn.click();
            return;
        }
        if (e.key === 'p' || e.key === 'P') {
            if (currentEmulator && document.activeElement !== fileInput) {
                e.preventDefault();
                pauseResumeBtn.click();
                return;
            }
        }
        if (e.key === 'r' || e.key === 'R') {
            if (currentEmulator && document.activeElement !== fileInput) {
                e.preventDefault();
                resetBtn.click();
                return;
            }
        }

        // NES Controller Key forwarding
        if (currentEmulator && !isPaused && KEY_ACTIONS[e.code]) {
            const canvas = document.getElementById('nes-screen');
            if (canvas && document.activeElement !== canvas) {
                canvas.focus();
            }
        }
    });

    // Gamepad detection
    window.addEventListener('gamepadconnected', (e) => {
        const gp = e.gamepad;
        if (gamepadDot) gamepadDot.classList.add('active');
        if (gamepadText) gamepadText.textContent = `Gamepad conectado: ${gp.id.split('(')[0]} (Listo)`;
        showToast(`Gamepad conectado: ${gp.id.split('(')[0]}`, '🎮');
        focusGameCanvas();
    });

    window.addEventListener('gamepaddisconnected', () => {
        if (gamepadDot) gamepadDot.classList.remove('active');
        if (gamepadText) gamepadText.textContent = 'Sin gamepad detectado (conectá uno USB/Bluetooth)';
        showToast('Gamepad desconectado', '🔌');
    });

    // DOM Ready: Init UI
    window.addEventListener('DOMContentLoaded', () => {
        updateScreenSizeUI();
    });

})();
