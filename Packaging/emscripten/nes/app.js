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
        if (!currentEmulator) return;
        try {
            // 1. Auto-save SRAM Battery data (essential for Zelda, Final Fantasy, RPGs)
            try {
                const sramBlob = await currentEmulator.saveSRAM();
                if (sramBlob && sramBlob.size > 0) {
                    await persistSaveState(`nes_sram_${activeRomName}`, sramBlob, `Batería/SRAM - ${activeRomName}`);
                }
            } catch (_) {}

            // 2. Auto-save SaveState
            if (!isPaused) {
                const stateObj = await currentEmulator.saveState();
                if (stateObj && stateObj.state) {
                    lastSavedState = stateObj.state;
                    await persistSaveState(`nes_latest_${activeRomName}`, stateObj.state, `Autosave - ${activeRomName}`);
                }
            }
        } catch (e) {
            console.warn('[NES AutoSave] Error:', e);
        }
    }

    // Auto-save on tab close / browser exit
    window.addEventListener('beforeunload', () => {
        autoSaveCurrentGame();
    });
    window.addEventListener('pagehide', () => {
        autoSaveCurrentGame();
    });

    // Periodic auto-save every 20 seconds
    setInterval(() => {
        autoSaveCurrentGame();
    }, 20000);

    // ==========================================
    // ROM Preparation, Platform & Capabilities Inspector
    // ==========================================
    let isComputerModeActive = false;
    const computerModeBadge = document.getElementById('computerModeBadge');

    function isDesktopPC() {
        const isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent || '') || 
                         ((navigator.maxTouchPoints || 0) > 1 && window.innerWidth <= 768);
        return !isMobile && (window.matchMedia ? window.matchMedia('(pointer: fine)').matches : true);
    }

    function inspectNesRom(arrayBuffer, fileName) {
        const nameLower = (fileName || '').toLowerCase();
        const isNameMatch = nameLower.includes('pcvivaz') || nameLower.includes('vivaz') || nameLower.includes('edu2000') || nameLower.includes('educational');

        if (!arrayBuffer || arrayBuffer.byteLength < 16) {
            return { isComputerRom: isNameMatch, format: 'UNKNOWN', title: fileName };
        }

        const bytes = new Uint8Array(arrayBuffer);
        const header4 = String.fromCharCode(bytes[0], bytes[1], bytes[2], bytes[3]);

        if (header4 === 'UNIF') {
            let offset = 32;
            let mapr = '';
            let name = '';
            while (offset + 8 <= bytes.length) {
                const chunkId = String.fromCharCode(bytes[offset], bytes[offset+1], bytes[offset+2], bytes[offset+3]);
                const chunkLen = bytes[offset+4] | (bytes[offset+5] << 8) | (bytes[offset+6] << 16) | (bytes[offset+7] << 24);
                const dataStart = offset + 8;
                const dataEnd = Math.min(dataStart + chunkLen, bytes.length);

                if (chunkId === 'MAPR') {
                    mapr = new TextDecoder().decode(bytes.slice(dataStart, dataEnd)).trim().replace(/\0/g, '');
                } else if (chunkId === 'NAME') {
                    name = new TextDecoder().decode(bytes.slice(dataStart, dataEnd)).trim().replace(/\0/g, '');
                }
                offset += 8 + chunkLen;
            }

            const isEdu = mapr.includes('UNL-EDU2000') || mapr.includes('EDU2000') || name.toLowerCase().includes('educational') || name.toLowerCase().includes('vivaz') || isNameMatch;
            return { isComputerRom: isEdu, format: 'UNIF', mapper: mapr || 'UNL-EDU2000', title: name || fileName };
        }

        if (header4 === 'NES\x1A') {
            const mapperLow = (bytes[6] >> 4) | (bytes[7] & 0xF0);
            const isNes2 = (bytes[7] & 0x0C) === 0x08;
            let mapper = mapperLow;
            if (isNes2) {
                mapper |= (bytes[8] & 0x0F) << 8;
            }
            const isComputerMapper = (mapper === 329 || mapper === 177 || mapper === 35 || isNameMatch);
            return { isComputerRom: isComputerMapper, format: isNes2 ? 'NES 2.0' : 'iNES', mapper: `Mapper ${mapper}`, title: fileName };
        }

        return { isComputerRom: isNameMatch, format: 'RAW', mapper: '0', title: fileName };
    }

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

        const romInfo = inspectNesRom(arrayBuffer, baseName);
        const file = new File([arrayBuffer], baseName, { type: 'application/octet-stream' });
        return { file, romInfo };
    }

    async function launchRom(romSource, romName) {
        activeRomName = romName;
        showLoading(`Iniciando ${romName}...`, 'Cargando motor NES (WebAssembly)...');

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

            const { file: romFile, romInfo } = await prepareRomData(romSource, romName);
            const onDesktopPC = isDesktopPC();

            // Check if this ROM is PC Vivaz / Computer Mode and running on PC
            if (romInfo.isComputerRom && onDesktopPC) {
                isComputerModeActive = true;
                if (computerModeBadge) computerModeBadge.style.display = 'flex';
            } else {
                isComputerModeActive = false;
                if (computerModeBadge) computerModeBadge.style.display = 'none';
            }

            // Configure RetroArch & Core specifically for the detected ROM & Platform
            let retroarchConfig;
            let retroarchCoreConfig;
            let primaryCore = 'nestopia';

            if (isComputerModeActive) {
                // ==========================================
                // EXCLUSIVE PC VIVAZ COMPUTER MODE (PC ONLY)
                // ==========================================
                primaryCore = 'nestopia'; // Nestopia has native cycle-accurate UNL-EDU2000 board + mouse
                retroarchConfig = {
                    video_vsync: 'true',
                    video_threaded: 'false',
                    video_smooth: 'false',
                    video_max_swapchain_images: '2',
                    video_frame_delay: '0',
                    audio_enable: 'true',
                    audio_sync: 'true',
                    audio_latency: '64',
                    autosave_interval: '10',
                    savestate_auto_load: 'false',
                    input_autodetect_enable: 'true',

                    // Port 1: RetroPad Controller (Arrows for menu navigation, Enter for Start/Open)
                    input_libretro_device_p1: '1',
                    input_player1_up: 'up',
                    input_player1_down: 'down',
                    input_player1_left: 'left',
                    input_player1_right: 'right',
                    input_player1_a: 'x',
                    input_player1_b: 'z',
                    input_player1_y: 'a',
                    input_player1_x: 's',
                    input_player1_start: 'enter',
                    input_player1_select: 'rshift',
                    input_player1_l: 'q',
                    input_player1_r: 'w',

                    // Port 2: Mouse Peripheral for Educational Computer 2000
                    input_libretro_device_p2: '2',
                    input_player1_mouse_index: '0',
                    input_player2_mouse_index: '0'
                };

                retroarchCoreConfig = {
                    nestopia_ram_power_state: '0x00',
                    nestopia_genie_distortion: 'disabled',
                    fceumm_ram_power_state: '0x00',
                    fceumm_nospritelimit: 'disabled',
                    fceumm_zapper_mode: 'mouse',
                    fceumm_mouse_sensitivity: '100'
                };

                // Enable pointer lock and active canvas focus on click
                canvas.addEventListener('click', () => {
                    if (canvas.requestPointerLock) {
                        try { canvas.requestPointerLock(); } catch (_) {}
                    }
                    focusGameCanvas();
                });

            } else {
                // ==========================================
                // STANDARD NES GAMEPAD MODE (GAMES & MOBILE)
                // ==========================================
                primaryCore = 'nestopia';
                retroarchConfig = {
                    video_vsync: 'true',
                    video_threaded: 'false',
                    video_smooth: 'false',
                    video_max_swapchain_images: '2',
                    video_frame_delay: '0',
                    audio_enable: 'true',
                    audio_sync: 'true',
                    audio_latency: '64',
                    autosave_interval: '10',
                    savestate_auto_load: 'false',
                    input_autodetect_enable: 'true',

                    // D-Pad
                    input_player1_up: 'up',
                    input_player1_down: 'down',
                    input_player1_left: 'left',
                    input_player1_right: 'right',

                    // NES Buttons: B -> 'z' / 'a', A -> 'x' / 's'
                    input_player1_b: 'z',
                    input_player1_a: 'x',
                    input_player1_y: 'a',
                    input_player1_x: 's',
                    input_player1_l: 'q',
                    input_player1_r: 'w',

                    // Start & Select
                    input_player1_start: 'enter',
                    input_player1_select: 'rshift'
                };

                retroarchCoreConfig = {
                    nestopia_ram_power_state: '0x00',
                    nestopia_genie_distortion: 'disabled',
                    fceumm_ram_power_state: '0x00',
                    fceumm_nospritelimit: 'disabled'
                };
            }

            // Check if saved SRAM battery exists in IndexedDB for this ROM
            const existingSram = await retrieveSaveState(`nes_sram_${romName}`);

            const launchOptions = {
                core: primaryCore,
                rom: romFile,
                element: canvas,
                retroarchConfig,
                retroarchCoreConfig,
                ...(existingSram ? { sram: existingSram } : {})
            };

            // Launch emulator with appropriate core resolution
            try {
                if (primaryCore === 'fceumm') {
                    currentEmulator = await Nostalgist.launch({
                        ...launchOptions,
                        resolveCoreJs() { return 'core/fceumm_libretro.js'; },
                        resolveCoreWasm() { return 'core/fceumm_libretro.wasm'; }
                    });
                } else {
                    currentEmulator = await Nostalgist.launch({
                        ...launchOptions,
                        resolveCoreJs() { return 'core/nestopia_libretro.js'; },
                        resolveCoreWasm() { return 'core/nestopia_libretro.wasm'; }
                    });
                }
            } catch (primaryErr) {
                console.warn(`[NES WASM] Fallo al iniciar con ${primaryCore}, intentando fallback FCEUmm:`, primaryErr);
                try {
                    currentEmulator = await Nostalgist.launch({
                        ...launchOptions,
                        core: 'fceumm',
                        resolveCoreJs() { return 'core/fceumm_libretro.js'; },
                        resolveCoreWasm() { return 'core/fceumm_libretro.wasm'; }
                    });
                } catch (fceuErr) {
                    console.warn('[NES WASM] Intentando fallback online:', fceuErr);
                    currentEmulator = await Nostalgist.launch({
                        ...launchOptions,
                        core: 'fceumm'
                    });
                }
            }

            isPaused = false;
            updatePauseBtnUI();
            hideLoading();

            if (isComputerModeActive) {
                showToast(`💻 Mini PC Vivaz iniciada: Teclado QWERTY y Ratón activos`, '⌨️', 5000);
            } else {
                showToast(`🔴 ${romName} en ejecución (60 FPS)`, '▶');
            }

            // Focus canvas
            setTimeout(() => {
                focusGameCanvas();
            }, 100);

            // Check for persistent save in IndexedDB
            setTimeout(async () => {
                const existingState = await retrieveSaveState(`nes_state_${romName}`) ||
                                     await retrieveSaveState(`nes_latest_${romName}`);
                if (existingSram && existingState) {
                    lastSavedState = existingState;
                    showToast(`💾 Batería SRAM cargada y Estado rápido disponible (F7).`, 'ℹ️', 5000);
                } else if (existingSram) {
                    showToast(`💾 Partida guardada (Batería SRAM) cargada con éxito.`, 'ℹ️', 4500);
                } else if (existingState) {
                    lastSavedState = existingState;
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

    // ==========================================
    // RetroSaves Manager Integration (100% Offline & Non-Blocking)
    // ==========================================
    const saves = window.RetroSaves.create({
        system: 'nes',
        getEmulator: () => currentEmulator,
        getRomName: () => activeRomName,
        isPaused: () => isPaused,
        onResumed: () => {
            isPaused = false;
            updatePauseBtnUI();
        },
        persist: persistSaveState,
        retrieve: retrieveSaveState,
        toast: showToast,
        relaunch: async () => {
            if (activeRomName) await launchRom(activeRomName, activeRomName);
        },
        afterAction: focusGameCanvas
    });

    async function autoSaveCurrentGame() {
        if (!currentEmulator) return;
        await saves.autoSave({ includeState: !isPaused });
    }

    // Periodic auto-save every 25 seconds
    setInterval(() => {
        if (currentEmulator && !isPaused) {
            saves.autoSave({ includeState: true });
        }
    }, 25000);

    saveStateBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        await saves.saveNow();
    });

    loadStateBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        await saves.loadLatest();
    });

    // Wire Save/Download/Import popover
    saves.bindDefaultUI();

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

    // Direct Dual-Keyboard & Virtual Gamepad Dispatcher for NES
    const KEY_ACTIONS = {
        'ArrowUp': 'up',
        'KeyW': 'up',
        'ArrowDown': 'down',
        'KeyS': 'down',
        'ArrowLeft': 'left',
        'KeyA': 'left',
        'ArrowRight': 'right',
        'KeyD': 'right',

        // NES B button: Z, J
        'KeyZ': 'b',
        'KeyJ': 'b',

        // NES A button: X, K, Space
        'KeyX': 'a',
        'KeyK': 'a',
        'Space': 'a',

        // Turbos / Shoulders
        'KeyQ': 'y',
        'KeyE': 'x',

        // Start & Select
        'Enter': 'start',
        'NumpadEnter': 'start',
        'ShiftLeft': 'select',
        'ShiftRight': 'select',
        'Tab': 'select',
        'KeyC': 'select'
    };

    window.addEventListener('keydown', (e) => {
        if (!currentEmulator) return;

        // Global hotkeys (Save / Load / Pause / Reset)
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
            if (document.activeElement !== fileInput) {
                e.preventDefault();
                pauseResumeBtn.click();
                return;
            }
        }
        if (e.key === 'r' || e.key === 'R') {
            if (document.activeElement !== fileInput) {
                e.preventDefault();
                resetBtn.click();
                return;
            }
        }

        if (isPaused) return;

        // Focus canvas if needed
        const canvas = document.getElementById('nes-screen');
        if (canvas && document.activeElement !== canvas && document.activeElement !== fileInput) {
            canvas.focus();
        }

        const action = KEY_ACTIONS[e.code];
        if (action) {
            e.preventDefault();
            try {
                currentEmulator.pressDown(action);
            } catch (_) {
                try { currentEmulator.keyboardDown(e.code); } catch (_) {}
            }
        } else {
            // Forward raw key code for typing in educational / computer software (A-Z, 0-9, Backspace, etc.)
            try {
                currentEmulator.keyboardDown(e.code);
            } catch (_) {}
        }
    }, { passive: false });

    window.addEventListener('keyup', (e) => {
        if (!currentEmulator) return;

        const action = KEY_ACTIONS[e.code];
        if (action) {
            e.preventDefault();
            try {
                currentEmulator.pressUp(action);
            } catch (_) {
                try { currentEmulator.keyboardUp(e.code); } catch (_) {}
            }
        } else {
            try {
                currentEmulator.keyboardUp(e.code);
            } catch (_) {}
        }
    }, { passive: false });

    // Virtual Gamepad integration (On-screen mobile touch HUD & mouse-clickable buttons)
    window.addEventListener('virtual-gamepad-action', (e) => {
        if (!currentEmulator || isPaused) return;
        const { action, isDown } = e.detail || {};
        if (!action) return;

        let mappedAction = action;
        if (action === 'c') mappedAction = 'b';
        if (action === 'x') mappedAction = 'a';
        if (action === 'l') mappedAction = 'b';
        if (action === 'r') mappedAction = 'a';

        try {
            if (isDown) {
                currentEmulator.pressDown(mappedAction);
            } else {
                currentEmulator.pressUp(mappedAction);
            }
        } catch (_) {
            try {
                if (isDown) currentEmulator.keyboardDown(action);
                else currentEmulator.keyboardUp(action);
            } catch (_) {}
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
