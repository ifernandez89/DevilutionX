/**
 * Gens Web — Sega Genesis / Mega Drive WebAssembly Controller
 * Powers 60 FPS Genesis emulation with WebAudio, Gamepads, Save States & SMD Deinterleaver
 */

(() => {
    'use strict';

    // State
    let currentEmulator = null;
    let isPaused = false;
    let crtFilterEnabled = false;
    let lastSavedState = null;
    let activeRomName = 'Juego';

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
    const gamepadStatus = document.getElementById('gamepadStatus');
    const gamepadDot = document.getElementById('gamepadDot');
    const gamepadText = document.getElementById('gamepadText');

    // Overlay & Toast
    const loadingOverlay = document.getElementById('loading-overlay');
    const loadingText = document.getElementById('loadingText');
    const loadingSubtext = document.getElementById('loadingSubtext');
    const toast = document.getElementById('toast');
    const toastIcon = document.getElementById('toastIcon');
    const toastMessage = document.getElementById('toastMessage');

    // Screen Sizes
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
        if (loadingOverlay) loadingOverlay.style.display = 'none';
    }

    // Focus canvas to prevent key stealing by buttons
    function focusGameCanvas() {
        if (document.activeElement && typeof document.activeElement.blur === 'function') {
            document.activeElement.blur();
        }
        const canvas = document.getElementById('gens-screen');
        if (canvas) {
            canvas.tabIndex = 0;
            canvas.focus();
        }
    }

    /**
     * Deinterleave Super Magic Drive (.smd) ROM files into standard flat binary.
     * SMD format has a 512-byte header followed by 16KB blocks where
     * even and odd bytes are split into 8KB halves.
     */
    function deinterleaveSMD(uint8Arr) {
        if (!uint8Arr || uint8Arr.length <= 512) return uint8Arr;

        const isSmdSize = (uint8Arr.length - 512) % 16384 === 0;
        const hasSmdSig = uint8Arr[8] === 0xaa && uint8Arr[9] === 0xbb;

        // Verify if offset 0x100 is NOT "SEGA" (meaning it is still interleaved or headered)
        const isStandardSega = uint8Arr.length > 0x104 &&
            uint8Arr[0x100] === 0x53 && uint8Arr[0x101] === 0x45 &&
            uint8Arr[0x102] === 0x47 && uint8Arr[0x103] === 0x41;

        if ((hasSmdSig || isSmdSize) && !isStandardSega) {
            console.log('[Gens WASM] ROM SMD entrelazada detectada. Convirtiendo a binario plano estándar...');
            const body = uint8Arr.subarray(512);
            const numBlocks = Math.floor(body.length / 16384);
            const result = new Uint8Array(numBlocks * 16384);
            let outIdx = 0;

            for (let b = 0; b < numBlocks; b++) {
                const blkOffset = b * 16384;
                const half = 8192;
                for (let i = 0; i < half; i++) {
                    result[outIdx++] = body[blkOffset + half + i]; // Even byte
                    result[outIdx++] = body[blkOffset + i];        // Odd byte
                }
            }
            return result;
        }

        // Also check if it has a 512-byte header without interleaving
        if (uint8Arr.length > 512 && uint8Arr[0x100 + 512] === 0x53 && uint8Arr[0x101 + 512] === 0x45) {
            console.log('[Gens WASM] Encabezado de 512 bytes detectado. Recortando a binario...');
            return uint8Arr.slice(512);
        }

        return uint8Arr;
    }

    /**
     * Prepares ROM data: fetches or reads file, deinterleaves SMD if needed,
     * and returns a clean File object with .bin extension.
     */
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

        const rawBytes = new Uint8Array(arrayBuffer);
        const cleanBytes = deinterleaveSMD(rawBytes);
        const cleanName = baseName.replace(/\.[^/.]+$/, '') + '.bin';

        return new File([cleanBytes], cleanName, { type: 'application/octet-stream' });
    }

    // Launch ROM function
    async function launchRom(romSource, romName) {
        activeRomName = romName;
        showLoading(`Iniciando ${romName}...`, 'Cargando motor Genesis Plus GX (WebAssembly)...');

        try {
            if (currentEmulator) {
                try { await currentEmulator.exit(); } catch (e) {}
                currentEmulator = null;
            }

            // Switch UI views FIRST so canvas container has real dimensions (980x735)
            hubSection.style.display = 'none';
            emulatorSection.style.display = 'flex';
            updateScreenSizeUI();

            // Prepare fresh canvas
            const oldCanvas = canvasContainer.querySelector('canvas');
            if (oldCanvas) oldCanvas.remove();

            const canvas = document.createElement('canvas');
            canvas.id = 'gens-screen';
            canvas.tabIndex = 0;
            canvasContainer.insertBefore(canvas, scanlinesOverlay);

            // Prepare ROM with SMD deinterleave
            const romFile = await prepareRomData(romSource, romName);

            // Sega 6-Button Arcade Pad mappings & Mobile Performance Config for RetroArch
            const retroarchConfig = {
                video_vsync: 'true',
                video_threaded: 'true',
                video_smooth: 'false',
                video_max_swapchain_images: '2',
                video_frame_delay: '0',
                audio_enable: 'true',
                audio_sync: 'true',
                audio_latency: '96',
                input_autodetect_enable: 'true',

                // D-Pad
                input_player1_up: 'up',
                input_player1_down: 'down',
                input_player1_left: 'left',
                input_player1_right: 'right',

                // Sega 6-Button Pad Mappings:
                // Genesis A -> RetroPad Y (key 'a')
                input_player1_y: 'a',
                // Genesis B -> RetroPad B (key 's')
                input_player1_b: 's',
                // Genesis C -> RetroPad A (key 'd')
                input_player1_a: 'd',
                // Genesis X -> RetroPad L (key 'q')
                input_player1_l: 'q',
                // Genesis Y -> RetroPad X (key 'w')
                input_player1_x: 'w',
                // Genesis Z -> RetroPad R (key 'e')
                input_player1_r: 'e',

                // Start & Mode
                input_player1_start: 'enter',
                input_player1_select: 'shift'
            };

            // Attempt launch using local core first for offline capability
            try {
                currentEmulator = await Nostalgist.launch({
                    core: 'genesis_plus_gx',
                    rom: romFile,
                    element: canvas,
                    resolveCoreJs() {
                        return 'core/genesis_plus_gx_libretro.js';
                    },
                    resolveCoreWasm() {
                        return 'core/genesis_plus_gx_libretro.wasm';
                    },
                    retroarchConfig
                });
            } catch (localErr) {
                console.warn('[Gens WASM] Error con núcleo local, intentando CDN:', localErr);
                currentEmulator = await Nostalgist.launch({
                    core: 'genesis_plus_gx',
                    rom: romFile,
                    element: canvas,
                    retroarchConfig
                });
            }

            isPaused = false;
            updatePauseBtnUI();
            hideLoading();
            showToast(`🎮 ${romName} en ejecución (60 FPS)`, '▶');

            // Set focus to the game canvas
            setTimeout(() => {
                focusGameCanvas();
            }, 100);

            // Check for existing save state in IndexedDB (100% Offline)
            setTimeout(async () => {
                const existingState = await retrieveSaveState(`gens_state_${romName}`) ||
                                      await retrieveSaveState(`gens_latest_${romName}`);
                if (existingState) {
                    lastSavedState = existingState;
                    showToast(`💾 Partida previa en IndexedDB detectada. Presioná F7 para continuar.`, 'ℹ️', 5000);
                }
            }, 800);

            // Scroll smoothly to screen
            emulatorSection.scrollIntoView({ behavior: 'smooth' });

        } catch (err) {
            console.error('[Gens WASM] Error crítico al cargar ROM:', err);
            hideLoading();
            showToast(`Error al iniciar ROM: ${err.message || err}`, '❌', 6000);
        }
    }

    // Stop emulator and return to HUB
    async function exitToHub() {
        if (currentEmulator) {
            showLoading('Cerrando emulador...', '');
            try {
                await currentEmulator.exit();
            } catch (e) {}
            currentEmulator = null;
            hideLoading();
        }

        const oldCanvas = canvasContainer.querySelector('canvas');
        if (oldCanvas) oldCanvas.remove();

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

    // Toolbar Event Listeners
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

    // IndexedDB Persistent Storage for Save States (100% Offline)
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
                system: 'gens',
                data: arrayBuffer,
                label,
                timestamp: Date.now(),
                dateStr: new Date().toLocaleString()
            });
        } catch (e) {
            console.warn('[Gens Save] Error guardando en IndexedDB:', e);
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
            console.warn('[Gens Save] Error cargando de IndexedDB:', e);
            return null;
        }
    }

    async function autoSaveCurrentGame() {
        if (!currentEmulator || isPaused) return;
        try {
            const stateObj = await currentEmulator.saveState();
            if (stateObj && stateObj.state) {
                lastSavedState = stateObj.state;
                await persistSaveState(`gens_latest_${activeRomName}`, stateObj.state, `Autosave - ${activeRomName}`);
            }
        } catch (e) {}
    }

    // Auto-save on page exit/close
    window.addEventListener('beforeunload', () => {
        autoSaveCurrentGame();
    });
    window.addEventListener('pagehide', () => {
        autoSaveCurrentGame();
    });

    saveStateBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        try {
            showToast('Guardando estado en IndexedDB...', '💾', 1500);
            const stateObj = await currentEmulator.saveState();
            lastSavedState = stateObj.state;
            await persistSaveState(`gens_state_${activeRomName}`, lastSavedState, `Manual F5 - ${activeRomName}`);
            await persistSaveState(`gens_latest_${activeRomName}`, lastSavedState, `Último - ${activeRomName}`);
            localStorage.setItem(`gens_save_${activeRomName}`, new Date().toISOString());
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
                stateToLoad = await retrieveSaveState(`gens_state_${activeRomName}`) ||
                              await retrieveSaveState(`gens_latest_${activeRomName}`);
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
            a.download = `gens_${activeRomName}_${Date.now()}.png`;
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
            focusGameCanvas();
            showToast(`Tamaño de pantalla: ${screenSizes[currentSizeIndex].name}`, '📐', 2000);
        });
    }

    fullscreenBtn.addEventListener('click', () => {
        const target = screenCard || canvasContainer || document.documentElement;
        if (!document.fullscreenElement) {
            target.requestFullscreen().catch(err => {
                showToast(`No se pudo activar pantalla completa: ${err.message}`, '⚠️');
            });
        } else {
            document.exitFullscreen();
        }
        focusGameCanvas();
    });

    exitGameBtn.addEventListener('click', () => {
        if (confirm('¿Deseas salir del juego actual?')) {
            exitToHub();
        }
    });

    // Clicking anywhere on the canvas or card focuses the game
    canvasContainer.addEventListener('click', focusGameCanvas);
    if (screenCard) screenCard.addEventListener('click', focusGameCanvas);

    // Hub Browse Button
    if (hubBrowseBtn) {
        hubBrowseBtn.addEventListener('click', () => fileInput.click());
    }

    // File Input & Drag and Drop Handlers
    topUploadBtn.addEventListener('click', () => fileInput.click());
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
        if (fileNameLower.endsWith('.nes') || fileNameLower.endsWith('.unf')) {
            if (confirm(`Has seleccionado una ROM de NES / Famicom (${file.name}).\n¿Deseas abrir el Emulador de NES (8-Bit)?`)) {
                window.location.href = '../nes/index.html';
            }
            return;
        }

        const validExtensions = ['.smd', '.bin', '.gen', '.md', '.zip'];
        const isValid = validExtensions.some(ext => fileNameLower.endsWith(ext));

        if (!isValid) {
            showToast('Formato no compatible. Usa .smd, .bin, .gen, .md o .zip', '⚠️', 4000);
            return;
        }

        launchRom(file, file.name);
    }

    /**
     * Direct Dual-Keyboard Dispatcher:
     * Guarantees that keyboard inputs are intercepted and forwarded to Nostalgist
     * regardless of browser element focus or button states.
     * Supports both A/S/D and Z/X/C setups out-of-the-box.
     */
    const KEY_ACTIONS = {
        // D-Pad
        'ArrowUp': 'up',
        'ArrowDown': 'down',
        'ArrowLeft': 'left',
        'ArrowRight': 'right',

        // Sega Button A: 'A' or 'Z' -> RetroPad Y
        'KeyA': 'y',
        'KeyZ': 'y',

        // Sega Button B: 'S' or 'X' -> RetroPad B
        'KeyS': 'b',
        'KeyX': 'b',

        // Sega Button C: 'D' or 'C' -> RetroPad A
        'KeyD': 'a',
        'KeyC': 'a',

        // Sega 6-Button Row: X, Y, Z -> RetroPad L, X, R
        'KeyQ': 'l',
        'KeyW': 'x',
        'KeyE': 'r',

        // Start & Mode
        'Enter': 'start',
        'Space': 'start',
        'ShiftLeft': 'select',
        'ShiftRight': 'select'
    };

    window.addEventListener('keydown', (e) => {
        if (!currentEmulator) return;

        // Hotkeys
        if (e.key === 'F5') {
            e.preventDefault();
            saveStateBtn.click();
            return;
        } else if (e.key === 'F7') {
            e.preventDefault();
            loadStateBtn.click();
            return;
        } else if (e.key === 'p' || e.key === 'P') {
            if (document.activeElement?.tagName !== 'INPUT') {
                e.preventDefault();
                pauseResumeBtn.click();
                return;
            }
        }

        // Gameplay keys
        const action = KEY_ACTIONS[e.code];
        if (action) {
            // Prevent scrolling on arrows/space
            e.preventDefault();
            try {
                currentEmulator.pressDown(action);
            } catch (err) {
                // Fallback to Nostalgist keyboardDown
                try { currentEmulator.keyboardDown(e.code); } catch (_) {}
            }
        }
    }, { passive: false });

    window.addEventListener('keyup', (e) => {
        if (!currentEmulator) return;

        const action = KEY_ACTIONS[e.code];
        if (action) {
            e.preventDefault();
            try {
                currentEmulator.pressUp(action);
            } catch (err) {
                try { currentEmulator.keyboardUp(e.code); } catch (_) {}
            }
        }
    }, { passive: false });

    // Gamepad API Detection
    window.addEventListener('gamepadconnected', (e) => {
        const gp = e.gamepad;
        if (gamepadDot) gamepadDot.classList.add('active');
        if (gamepadText) gamepadText.textContent = `🎮 Mando conectado: ${gp.id.split('(')[0].trim() || 'Gamepad USB'}`;
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
