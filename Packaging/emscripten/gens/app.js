/**
 * Gens Web — Sega Genesis / Mega Drive WebAssembly Controller
 * Powers 60 FPS Genesis emulation with WebAudio, Gamepads & Save States
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
    const canvasContainer = document.getElementById('canvas-container');
    const scanlinesOverlay = document.getElementById('scanlines');
    const dropzone = document.getElementById('dropzone');
    const fileInput = document.getElementById('fileInput');
    const topUploadBtn = document.getElementById('topUploadBtn');
    const playRoadRashBtn = document.getElementById('playRoadRashBtn');

    // Controls
    const pauseResumeBtn = document.getElementById('pauseResumeBtn');
    const resetBtn = document.getElementById('resetBtn');
    const crtToggleBtn = document.getElementById('crtToggleBtn');
    const saveStateBtn = document.getElementById('saveStateBtn');
    const loadStateBtn = document.getElementById('loadStateBtn');
    const screenshotBtn = document.getElementById('screenshotBtn');
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

    // Launch ROM function
    async function launchRom(romSource, romName) {
        activeRomName = romName;
        showLoading(`Iniciando ${romName}...`, 'Cargando motor Genesis Plus GX (WebAssembly)...');

        try {
            if (currentEmulator) {
                try { await currentEmulator.exit(); } catch (e) {}
                currentEmulator = null;
            }

            // Prepare fresh canvas
            const oldCanvas = canvasContainer.querySelector('canvas');
            if (oldCanvas) oldCanvas.remove();

            const canvas = document.createElement('canvas');
            canvas.id = 'gens-screen';
            canvasContainer.insertBefore(canvas, scanlinesOverlay);

            // Attempt launch using local core first for offline capability
            try {
                currentEmulator = await Nostalgist.launch({
                    core: 'genesis_plus_gx',
                    rom: romSource,
                    element: canvas,
                    resolveCoreJs() {
                        return 'core/genesis_plus_gx_libretro.js';
                    },
                    resolveCoreWasm() {
                        return 'core/genesis_plus_gx_libretro.wasm';
                    },
                    retroarchConfig: {
                        video_vsync: 'true',
                        audio_enable: 'true',
                        input_autodetect_enable: 'true'
                    }
                });
            } catch (localErr) {
                console.warn('[Gens WASM] Error con núcleo local, intentando CDN:', localErr);
                currentEmulator = await Nostalgist.launch({
                    core: 'genesis_plus_gx',
                    rom: romSource,
                    element: canvas
                });
            }

            // Switch UI views
            hubSection.style.display = 'none';
            emulatorSection.style.display = 'flex';
            isPaused = false;
            updatePauseBtnUI();
            hideLoading();
            showToast(`🎮 ${romName} en ejecución (60 FPS)`, '▶');

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
    });

    resetBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        if (confirm(`¿Reiniciar ${activeRomName}?`)) {
            await currentEmulator.restart();
            isPaused = false;
            updatePauseBtnUI();
            showToast('Juego reiniciado', '🔄');
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
    });

    saveStateBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        try {
            showToast('Guardando estado...', '💾', 1500);
            const stateObj = await currentEmulator.saveState();
            lastSavedState = stateObj.state;
            // Also store in IndexedDB or localStorage timestamp
            localStorage.setItem(`gens_save_${activeRomName}`, new Date().toISOString());
            showToast('¡Estado guardado con éxito! (Slot Rápido)', '✅');
        } catch (err) {
            console.error('Error guardando estado:', err);
            showToast('Error al guardar estado', '❌');
        }
    });

    loadStateBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        if (!lastSavedState) {
            showToast('No hay ningún guardado rápido en esta sesión', '⚠️');
            return;
        }
        try {
            showToast('Cargando estado...', '📂', 1500);
            await currentEmulator.loadState(lastSavedState);
            isPaused = false;
            updatePauseBtnUI();
            showToast('¡Estado restaurado!', '✅');
        } catch (err) {
            console.error('Error cargando estado:', err);
            showToast('Error al restaurar estado', '❌');
        }
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
    });

    fullscreenBtn.addEventListener('click', () => {
        const target = canvasContainer || document.documentElement;
        if (!document.fullscreenElement) {
            target.requestFullscreen().catch(err => {
                showToast(`No se pudo activar pantalla completa: ${err.message}`, '⚠️');
            });
        } else {
            document.exitFullscreen();
        }
    });

    exitGameBtn.addEventListener('click', () => {
        if (confirm('¿Deseas salir del juego actual?')) {
            exitToHub();
        }
    });

    // Road Rash 3 Launch Button
    playRoadRashBtn.addEventListener('click', () => {
        launchRom('roms/RoadRash3.smd', 'Road Rash 3 (Tour de Force)');
    });

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
        const validExtensions = ['.smd', '.bin', '.gen', '.md', '.zip'];
        const fileNameLower = file.name.toLowerCase();
        const isValid = validExtensions.some(ext => fileNameLower.endsWith(ext));

        if (!isValid) {
            showToast('Formato no compatible. Usa .smd, .bin, .gen, .md o .zip', '⚠️', 4000);
            return;
        }

        launchRom(file, file.name);
    }

    // Keyboard Hotkeys
    window.addEventListener('keydown', (e) => {
        if (!currentEmulator) return;

        if (e.key === 'F5') {
            e.preventDefault();
            saveStateBtn.click();
        } else if (e.key === 'F7') {
            e.preventDefault();
            loadStateBtn.click();
        } else if (e.key === 'p' || e.key === 'P') {
            if (document.activeElement.tagName !== 'INPUT') {
                e.preventDefault();
                pauseResumeBtn.click();
            }
        }
    });

    // Gamepad API Detection
    window.addEventListener('gamepadconnected', (e) => {
        const gp = e.gamepad;
        gamepadDot.classList.add('active');
        gamepadText.textContent = `🎮 Mando conectado: ${gp.id.split('(')[0].trim() || 'Gamepad USB'}`;
        showToast(`Gamepad conectado: ${gp.id.split('(')[0]}`, '🎮');
    });

    window.addEventListener('gamepaddisconnected', () => {
        gamepadDot.classList.remove('active');
        gamepadText.textContent = 'Sin gamepad detectado (conectá uno USB/Bluetooth)';
        showToast('Gamepad desconectado', '🔌');
    });

    // Check URL parameters for auto-start (e.g. ?rom=roadrash3)
    window.addEventListener('DOMContentLoaded', () => {
        const urlParams = new URLSearchParams(window.location.search);
        const romParam = urlParams.get('rom');
        if (romParam === 'roadrash3') {
            setTimeout(() => {
                launchRom('roms/RoadRash3.smd', 'Road Rash 3 (Tour de Force)');
            }, 300);
        }
    });

})();
