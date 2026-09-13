/**
 * DOOM WASM — UAC Phobos Terminal & Freedoom WebAssembly Controller
 * Powers 60 FPS DOOM emulation with WebAudio, PointerLock, Save States & Gamepad API
 */

(() => {
    'use strict';

    // State
    let currentEmulator = null;
    let isPaused = false;
    let crtFilterEnabled = false;
    let lastSavedState = null;
    let activeRomName = 'DOOM 1 Shareware';

    // DOM Elements
    const hubSection = document.getElementById('hub-section');
    const emulatorSection = document.getElementById('emulator-section');
    const screenCard = document.querySelector('.screen-card');
    const screenToolbar = document.querySelector('.screen-toolbar');
    const canvasContainer = document.getElementById('canvas-container');
    const scanlinesOverlay = document.getElementById('scanlines');

    // Cartridge buttons
    const playDoom1Btn = document.getElementById('playDoom1Btn');
    const playFreedoom1Btn = document.getElementById('playFreedoom1Btn');
    const playFreedoom2Btn = document.getElementById('playFreedoom2Btn');

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

    // Toast & Loading Helpers
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

    function focusGameCanvas() {
        if (document.activeElement && typeof document.activeElement.blur === 'function') {
            document.activeElement.blur();
        }
        const canvas = document.getElementById('doom-screen');
        if (canvas) {
            canvas.tabIndex = 0;
            canvas.focus();
        }
    }

    // Launch DOOM with PrBoom Core
    async function launchDoom(wadPath, gameTitle) {
        activeRomName = gameTitle;
        showLoading(`Iniciando ${gameTitle}...`, 'Cargando motor PrBoom WebAssembly & IWAD...');

        try {
            if (currentEmulator) {
                try { await currentEmulator.exit(); } catch (e) {}
                currentEmulator = null;
            }

            // Switch views FIRST so canvas has true dimensions
            hubSection.style.display = 'none';
            emulatorSection.style.display = 'flex';
            updateScreenSizeUI();

            // Prepare clean canvas
            const oldCanvas = canvasContainer.querySelector('canvas');
            if (oldCanvas) oldCanvas.remove();

            const canvas = document.createElement('canvas');
            canvas.id = 'doom-screen';
            canvas.tabIndex = 0;
            canvasContainer.insertBefore(canvas, scanlinesOverlay);

            // RetroArch Config for DOOM
            const retroarchConfig = {
                video_vsync: 'true',
                audio_enable: 'true',
                input_autodetect_enable: 'true',

                // Movement
                input_player1_up: 'w',
                input_player1_down: 's',
                input_player1_left: 'a',
                input_player1_right: 'd',

                // Actions
                input_player1_b: 'ctrl',      // Fire
                input_player1_a: 'space',     // Use / Open
                input_player1_y: 'e',         // Alternate use
                input_player1_l: 'shift',     // Run / Speed
                input_player1_select: 'tab',  // Map
                input_player1_start: 'enter'  // Enter / Menu
            };

            // Launch with Nostalgist (local core first, fallback to CDN)
            try {
                currentEmulator = await Nostalgist.launch({
                    core: 'prboom',
                    rom: wadPath,
                    bios: ['core/prboom.wad'],
                    element: canvas,
                    resolveCoreJs() {
                        return 'core/prboom_libretro.js';
                    },
                    resolveCoreWasm() {
                        return 'core/prboom_libretro.wasm';
                    },
                    retroarchConfig
                });
            } catch (localErr) {
                console.warn('[DOOM WASM] Fallback a CDN para PrBoom:', localErr);
                currentEmulator = await Nostalgist.launch({
                    core: 'prboom',
                    rom: wadPath,
                    bios: ['wads/prboom.wad'],
                    element: canvas,
                    retroarchConfig
                });
            }

            isPaused = false;
            updatePauseBtnUI();
            hideLoading();
            showToast(`💀 ${gameTitle} en ejecución (60 FPS)`, '▶');

            setTimeout(() => {
                focusGameCanvas();
            }, 100);

            emulatorSection.scrollIntoView({ behavior: 'smooth' });

        } catch (err) {
            console.error('[DOOM WASM] Error crítico:', err);
            hideLoading();
            showToast(`Error al iniciar: ${err.message || err}`, '❌', 6000);
        }
    }

    // Exit to Hub
    async function exitToHub() {
        if (currentEmulator) {
            showLoading('Cerrando DOOM...', '');
            try {
                await currentEmulator.exit();
            } catch (e) {}
            currentEmulator = null;
            hideLoading();
        }

        const oldCanvas = canvasContainer.querySelector('canvas');
        if (oldCanvas) oldCanvas.remove();

        emulatorSection.style.display = 'none';
        hubSection.style.display = 'flex';
        showToast('Volviste al menú de cartuchos', '📂');
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

    // Toolbar Listeners
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
            showToast('Partida reiniciada', '🔄');
            focusGameCanvas();
        }
    });

    crtToggleBtn.addEventListener('click', () => {
        crtFilterEnabled = !crtFilterEnabled;
        if (crtFilterEnabled) {
            scanlinesOverlay.classList.add('active');
            crtToggleBtn.textContent = '📺 Scanlines: ON';
            crtToggleBtn.classList.add('active');
            showToast('Scanlines activado', '📺');
        } else {
            scanlinesOverlay.classList.remove('active');
            crtToggleBtn.textContent = '📺 Scanlines: OFF';
            crtToggleBtn.classList.remove('active');
            showToast('Scanlines desactivado', '📺');
        }
        focusGameCanvas();
    });

    saveStateBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        try {
            showToast('Guardando estado...', '💾', 1500);
            const stateObj = await currentEmulator.saveState();
            lastSavedState = stateObj.state;
            localStorage.setItem(`doom_save_${activeRomName}`, new Date().toISOString());
            showToast('¡Estado guardado con éxito! (Slot Rápido)', '✅');
        } catch (err) {
            console.error('Error guardando:', err);
            showToast('Error al guardar estado', '❌');
        }
        focusGameCanvas();
    });

    loadStateBtn.addEventListener('click', async () => {
        if (!currentEmulator) return;
        if (!lastSavedState) {
            showToast('No hay ningún guardado en esta sesión', '⚠️');
            focusGameCanvas();
            return;
        }
        try {
            showToast('Cargando estado...', '📂', 1500);
            await currentEmulator.loadState(lastSavedState);
            isPaused = false;
            updatePauseBtnUI();
            showToast('¡Estado restaurado!', '✅');
        } catch (err) {
            console.error('Error cargando:', err);
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
            a.download = `doom_${Date.now()}.png`;
            a.click();
            showToast('Captura descargada', '📸');
        } catch (err) {
            showToast('Error al capturar', '❌');
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
        if (confirm('¿Deseas salir de la partida actual?')) {
            exitToHub();
        }
    });

    canvasContainer.addEventListener('click', focusGameCanvas);
    if (screenCard) screenCard.addEventListener('click', focusGameCanvas);

    // Cartridge Click Handlers
    playDoom1Btn.addEventListener('click', () => {
        launchDoom('wads/doom1.wad', 'DOOM 1 Shareware (Knee-Deep in the Dead)');
    });

    playFreedoom1Btn.addEventListener('click', () => {
        launchDoom('wads/freedoom1.wad', 'Freedoom: Phase 1 (30 Niveles)');
    });

    playFreedoom2Btn.addEventListener('click', () => {
        launchDoom('wads/freedoom2.wad', 'Freedoom: Phase 2 (Super Shotgun)');
    });

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
            if (document.activeElement?.tagName !== 'INPUT') {
                e.preventDefault();
                pauseResumeBtn.click();
            }
        }
    });

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

    // URL Query Param auto start
    window.addEventListener('DOMContentLoaded', () => {
        updateScreenSizeUI();
        const urlParams = new URLSearchParams(window.location.search);
        const gameParam = urlParams.get('game');
        if (gameParam === 'doom1') {
            setTimeout(() => playDoom1Btn.click(), 300);
        } else if (gameParam === 'freedoom1') {
            setTimeout(() => playFreedoom1Btn.click(), 300);
        } else if (gameParam === 'freedoom2') {
            setTimeout(() => playFreedoom2Btn.click(), 300);
        }
    });

})();
