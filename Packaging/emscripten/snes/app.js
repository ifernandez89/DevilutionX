/**
 * SNES Web — Super Nintendo 16-Bit WebAssembly Controller
 * Powers 60 FPS cycle-accurate SNES emulation with Snes9x / Snes9x2010 WASM cores,
 * Sony SPC700 stereo audio, Mode 7, USB/Bluetooth Gamepads,
 * persistent offline IndexedDB saves, and CRT scanlines.
 */

(() => {
    'use strict';

    // State
    let currentEmulator = null;
    let isPaused = false;
    let crtFilterEnabled = false;
    let lastSavedState = null;
    let activeRomName = 'Juego SNES';

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

    // Screen Sizes (Exact replica of Sega Genesis & NES implementation)
    const screenSizes = [
        { name: 'Normal', className: 'size-normal', label: '🖥️ Tamaño: Normal (800p)' },
        { name: 'Grande', className: 'size-large', label: '🖥️ Tamaño: Grande (980p)' },
        { name: 'Cinema', className: 'size-cinema', label: '⚡ Tamaño: Cinema (1180p)' }
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
    function showToast(msg, icon = '💾', duration = 3500) {
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
        const canvas = document.getElementById('snes-screen');
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
                system: 'snes',
                data: arrayBuffer,
                label,
                timestamp: Date.now(),
                dateStr: new Date().toLocaleString()
            });
        } catch (e) {
            console.warn('[SNES Save] Error guardando en IndexedDB:', e);
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
            console.warn('[SNES Save] Error recuperando de IndexedDB:', e);
            return null;
        }
    }

    async function autoSaveCurrentGame() {
        if (!currentEmulator || isPaused) return;
        try {
            const stateObj = await currentEmulator.saveState();
            if (stateObj && stateObj.state) {
                lastSavedState = stateObj.state;
                await persistSaveState(`snes_latest_${activeRomName}`, stateObj.state, `Autosave - ${activeRomName}`);
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

        // Ensure extension is .sfc if none
        if (!baseName.includes('.')) {
            baseName += '.sfc';
        }

        return new File([arrayBuffer], baseName, { type: 'application/octet-stream' });
    }

    async function launchRom(romSource, romName) {
        activeRomName = romName;
        showLoading(`Iniciando ${romName}...`, 'Cargando motor Snes9x (WebAssembly)...');

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
            canvas.id = 'snes-screen';
            canvas.tabIndex = 0;
            canvasContainer.insertBefore(canvas, scanlinesOverlay);

            const romFile = await prepareRomData(romSource, romName);

            // RetroArch configuration for Super Nintendo
            const retroarchConfig = {
                video_vsync: 'true',
                audio_enable: 'true',
                input_autodetect_enable: 'true',

                // D-Pad
                input_player1_up: 'up',
                input_player1_down: 'down',
                input_player1_left: 'left',
                input_player1_right: 'right',

                // SNES Controller Layout:
                // B -> 'x' or 's' (RetroPad B)
                input_player1_b: 'x',
                // A -> 'c' or 'd' (RetroPad A)
                input_player1_a: 'c',
                // Y -> 'z' or 'a' (RetroPad Y)
                input_player1_y: 'z',
                // X -> 'v' or 'w' (RetroPad X)
                input_player1_x: 'v',

                // Shoulder buttons
                input_player1_l: 'q',
                input_player1_r: 'e',

                // Start & Select
                input_player1_start: 'enter',
                input_player1_select: 'shift'
            };

            // Launch with local Snes9x core first (100% Offline)
            try {
                currentEmulator = await Nostalgist.launch({
                    core: 'snes9x',
                    rom: romFile,
                    element: canvas,
                    resolveCoreJs() {
                        return 'core/snes9x_libretro.js';
                    },
                    resolveCoreWasm() {
                        return 'core/snes9x_libretro.wasm';
                    },
                    retroarchConfig
                });
            } catch (snes9xErr) {
                console.warn('[SNES WASM] Error con Snes9x local, intentando Snes9x2010:', snes9xErr);
                try {
                    currentEmulator = await Nostalgist.launch({
                        core: 'snes9x2010',
                        rom: romFile,
                        element: canvas,
                        resolveCoreJs() {
                            return 'core/snes9x2010_libretro.js';
                        },
                        resolveCoreWasm() {
                            return 'core/snes9x2010_libretro.wasm';
                        },
                        retroarchConfig
                    });
                } catch (backupErr) {
                    console.warn('[SNES WASM] Error con Snes9x2010 local, intentando CDN:', backupErr);
                    currentEmulator = await Nostalgist.launch({
                        core: 'snes9x',
                        rom: romFile,
                        element: canvas,
                        retroarchConfig
                    });
                }
            }

            hideLoading();
            isPaused = false;
            if (pauseResumeBtn) pauseResumeBtn.textContent = '⏸️ Pausar';
            showToast(`¡${romName} iniciado a 60 FPS!`, '🎮');
            focusGameCanvas();

            // Check if there is an autosaved state from previous session
            const savedStateBlob = await retrieveSaveState(`snes_latest_${activeRomName}`);
            if (savedStateBlob) {
                showToast('Partida anterior detectada en IndexedDB. Usa F7 o el botón Cargar.', '💾', 5000);
            }

        } catch (err) {
            hideLoading();
            console.error('[SNES Hub] Error iniciando emulador:', err);
            alert(`No se pudo iniciar el juego "${romName}".\nDetalle: ${err.message || err}`);
            exitToHub();
        }
    }

    function exitToHub() {
        if (currentEmulator) {
            autoSaveCurrentGame();
            try {
                currentEmulator.exit();
            } catch (e) {}
            currentEmulator = null;
        }
        emulatorSection.style.display = 'none';
        hubSection.style.display = 'flex';
        showToast('Juego finalizado. Volviendo al menú.', '⏹');
    }

    // ==========================================
    // File Handlers: Drag & Drop and Browse
    // ==========================================
    function handleFile(file) {
        if (!file) return;
        const cleanName = file.name.replace(/\.[^/.]+$/, '');
        launchRom(file, cleanName);
    }

    if (fileInput) {
        fileInput.addEventListener('change', (e) => {
            if (e.target.files && e.target.files[0]) {
                handleFile(e.target.files[0]);
            }
        });
    }

    if (topUploadBtn) {
        topUploadBtn.addEventListener('click', () => {
            if (fileInput) fileInput.click();
        });
    }

    if (hubBrowseBtn) {
        hubBrowseBtn.addEventListener('click', () => {
            if (fileInput) fileInput.click();
        });
    }

    if (dropzone) {
        dropzone.addEventListener('click', () => {
            if (fileInput) fileInput.click();
        });

        ['dragenter', 'dragover'].forEach(eventName => {
            dropzone.addEventListener(eventName, (e) => {
                e.preventDefault();
                e.stopPropagation();
                dropzone.classList.add('dragover');
            }, false);
        });

        ['dragleave', 'drop'].forEach(eventName => {
            dropzone.addEventListener(eventName, (e) => {
                e.preventDefault();
                e.stopPropagation();
                dropzone.classList.remove('dragover');
            }, false);
        });

        dropzone.addEventListener('drop', (e) => {
            const dt = e.dataTransfer;
            const files = dt.files;
            if (files && files.length > 0) {
                handleFile(files[0]);
            }
        });
    }

    // Global drag & drop on window
    window.addEventListener('dragover', (e) => e.preventDefault(), false);
    window.addEventListener('drop', (e) => {
        e.preventDefault();
        if (e.dataTransfer && e.dataTransfer.files && e.dataTransfer.files.length > 0) {
            handleFile(e.dataTransfer.files[0]);
        }
    }, false);

    // ==========================================
    // Controls Toolbar Events
    // ==========================================
    if (pauseResumeBtn) {
        pauseResumeBtn.addEventListener('click', () => {
            if (!currentEmulator) return;
            if (isPaused) {
                currentEmulator.resume();
                isPaused = false;
                pauseResumeBtn.textContent = '⏸️ Pausar';
                showToast('Juego reanudado', '▶️');
                focusGameCanvas();
            } else {
                currentEmulator.pause();
                isPaused = true;
                pauseResumeBtn.textContent = '▶️ Reanudar';
                showToast('Juego en pausa', '⏸️');
            }
        });
    }

    if (resetBtn) {
        resetBtn.addEventListener('click', () => {
            if (!currentEmulator) return;
            currentEmulator.restart();
            showToast('Juego reiniciado', '🔄');
            focusGameCanvas();
        });
    }

    if (crtToggleBtn) {
        crtToggleBtn.addEventListener('click', () => {
            crtFilterEnabled = !crtFilterEnabled;
            if (scanlinesOverlay) {
                scanlinesOverlay.classList.toggle('active', crtFilterEnabled);
            }
            crtToggleBtn.textContent = crtFilterEnabled ? '📺 Scanlines: ON' : '📺 Scanlines: OFF';
            showToast(crtFilterEnabled ? 'Filtro CRT activado' : 'Filtro CRT desactivado', '📺');
            focusGameCanvas();
        });
    }

    if (screenSizeBtn) {
        screenSizeBtn.addEventListener('click', () => {
            currentSizeIndex = (currentSizeIndex + 1) % screenSizes.length;
            updateScreenSizeUI();
            const size = screenSizes[currentSizeIndex];
            showToast(`Tamaño cambiado a: ${size.name}`, '🖥️');
            focusGameCanvas();
        });
    }

    if (saveStateBtn) {
        saveStateBtn.addEventListener('click', async () => {
            if (!currentEmulator) return;
            try {
                const stateObj = await currentEmulator.saveState();
                if (stateObj && stateObj.state) {
                    lastSavedState = stateObj.state;
                    await persistSaveState(`snes_manual_${activeRomName}`, stateObj.state, `Manual - ${activeRomName}`);
                    showToast('¡Estado guardado en IndexedDB! (F5)', '💾');
                }
            } catch (err) {
                console.error('[SNES Save] Error:', err);
                showToast('Error al guardar estado', '❌');
            }
            focusGameCanvas();
        });
    }

    if (loadStateBtn) {
        loadStateBtn.addEventListener('click', async () => {
            if (!currentEmulator) return;
            try {
                let stateToLoad = lastSavedState;
                if (!stateToLoad) {
                    stateToLoad = await retrieveSaveState(`snes_manual_${activeRomName}`);
                }
                if (!stateToLoad) {
                    stateToLoad = await retrieveSaveState(`snes_latest_${activeRomName}`);
                }

                if (stateToLoad) {
                    await currentEmulator.loadState(stateToLoad);
                    showToast('¡Partida cargada exitosamente! (F7)', '📂');
                } else {
                    showToast('No hay partida guardada para este juego', '⚠️');
                }
            } catch (err) {
                console.error('[SNES Load] Error:', err);
                showToast('Error al cargar estado', '❌');
            }
            focusGameCanvas();
        });
    }

    if (screenshotBtn) {
        screenshotBtn.addEventListener('click', async () => {
            if (!currentEmulator) return;
            try {
                const blob = await currentEmulator.screenshot();
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = `${activeRomName}_capture_${Date.now()}.png`;
                a.click();
                URL.revokeObjectURL(url);
                showToast('Captura descargada', '📸');
            } catch (e) {
                showToast('Error al capturar pantalla', '❌');
            }
            focusGameCanvas();
        });
    }

    if (fullscreenBtn) {
        fullscreenBtn.addEventListener('click', () => {
            const el = canvasContainer || document.documentElement;
            if (!document.fullscreenElement) {
                el.requestFullscreen().catch(err => {
                    alert(`Error de pantalla completa: ${err.message}`);
                });
            } else {
                document.exitFullscreen();
            }
            focusGameCanvas();
        });
    }

    if (exitGameBtn) {
        exitGameBtn.addEventListener('click', () => {
            if (confirm(`¿Deseas cerrar "${activeRomName}" y volver al menú principal?`)) {
                exitToHub();
            }
        });
    }

    // Keyboard Shortcuts (F5 = Save, F7 = Load, P = Pause, R = Reset)
    window.addEventListener('keydown', (e) => {
        if (!currentEmulator) return;

        if (e.key === 'F5') {
            e.preventDefault();
            if (saveStateBtn) saveStateBtn.click();
        } else if (e.key === 'F7') {
            e.preventDefault();
            if (loadStateBtn) loadStateBtn.click();
        } else if ((e.key === 'p' || e.key === 'P') && e.ctrlKey) {
            e.preventDefault();
            if (pauseResumeBtn) pauseResumeBtn.click();
        }
    });

    // ==========================================
    // Gamepad API Support
    // ==========================================
    function updateGamepadStatus() {
        const gamepads = navigator.getGamepads ? navigator.getGamepads() : [];
        let connected = false;
        let gpName = '';

        for (let i = 0; i < gamepads.length; i++) {
            if (gamepads[i]) {
                connected = true;
                gpName = gamepads[i].id;
                break;
            }
        }

        if (gamepadDot && gamepadText) {
            if (connected) {
                gamepadDot.classList.add('connected');
                gamepadText.textContent = `Mando Conectado: ${gpName}`;
            } else {
                gamepadDot.classList.remove('connected');
                gamepadText.textContent = 'Mando USB/Bluetooth: Esperando conexión (conecta y presiona cualquier botón)';
            }
        }
    }

    window.addEventListener('gamepadconnected', (e) => {
        updateGamepadStatus();
        showToast(`Mando detectado: ${e.gamepad.id}`, '🎮');
    });

    window.addEventListener('gamepaddisconnected', () => {
        updateGamepadStatus();
        showToast('Mando desconectado', '⚠️');
    });

    // Check once on startup
    setTimeout(updateGamepadStatus, 1000);

})();
