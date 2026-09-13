/**
 * Cave Story WASM — Doukutsu Monogatari WebAssembly Controller
 * Powers 60 FPS Cave Story emulation with NXEngine, WebAudio, Save States & Gamepads
 */

(() => {
    'use strict';

    // State
    let currentEmulator = null;
    let isPaused = false;
    let crtFilterEnabled = false;
    let lastSavedState = null;
    let activeRomName = 'Cave Story (English Deluxe)';

    // DOM Elements
    const hubSection = document.getElementById('hub-section');
    const emulatorSection = document.getElementById('emulator-section');
    const screenCard = document.querySelector('.screen-card');
    const screenToolbar = document.querySelector('.screen-toolbar');
    const canvasContainer = document.getElementById('canvas-container');
    const scanlinesOverlay = document.getElementById('scanlines');
    const playCaveBtn = document.getElementById('playCaveBtn');
    const activeGameTitle = document.getElementById('activeGameTitle');

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

    // Custom Mod Dropzone
    const csDropzone = document.getElementById('csDropzone');
    const csFileInput = document.getElementById('csFileInput');
    const browseCsBtn = document.getElementById('browseCsBtn');

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
        const canvas = document.getElementById('cave-screen');
        if (canvas) {
            canvas.tabIndex = 0;
            canvas.focus();
        }
    }

    // Launch Cave Story with NXEngine
    async function launchCaveStory(customRom = null, romLabel = 'Cave Story (English Deluxe)') {
        showLoading('Entrando a la Caverna...', 'Cargando motor NXEngine WebAssembly & Doukutsu...');

        try {
            if (currentEmulator) {
                try { await currentEmulator.exit(); } catch (e) {}
                currentEmulator = null;
            }

            activeRomName = romLabel;
            if (activeGameTitle) activeGameTitle.textContent = activeRomName;

            hubSection.style.display = 'none';
            emulatorSection.style.display = 'flex';
            updateScreenSizeUI();

            const oldCanvas = canvasContainer.querySelector('canvas');
            if (oldCanvas) oldCanvas.remove();

            const canvas = document.createElement('canvas');
            canvas.id = 'cave-screen';
            canvas.tabIndex = 0;
            canvasContainer.insertBefore(canvas, scanlinesOverlay);

            const retroarchConfig = {
                video_vsync: 'true',
                audio_enable: 'true',
                input_autodetect_enable: 'true',

                input_player1_up: 'up',
                input_player1_down: 'down',
                input_player1_left: 'left',
                input_player1_right: 'right',

                input_player1_a: 'z',         // Jump
                input_player1_b: 'x',         // Shoot
                input_player1_l: 'a',         // Prev Weapon
                input_player1_r: 's',         // Next Weapon
                input_player1_select: 'q',    // Inventory
                input_player1_start: 'enter'  // Map / Menu
            };

            const romTarget = customRom || 'data/cavestory.zip';

            try {
                currentEmulator = await Nostalgist.launch({
                    core: 'nxengine',
                    rom: romTarget,
                    element: canvas,
                    resolveCoreJs() {
                        return 'core/nxengine_libretro.js';
                    },
                    resolveCoreWasm() {
                        return 'core/nxengine_libretro.wasm';
                    },
                    retroarchConfig
                });
            } catch (localErr) {
                console.warn('[Cave Story WASM] Fallback a CDN:', localErr);
                currentEmulator = await Nostalgist.launch({
                    core: 'nxengine',
                    rom: romTarget,
                    element: canvas,
                    retroarchConfig
                });
            }

            isPaused = false;
            updatePauseBtnUI();
            hideLoading();
            showToast(`🐰 ${activeRomName} en ejecución (60 FPS)`, '▶');

            setTimeout(() => {
                focusGameCanvas();
            }, 100);

            emulatorSection.scrollIntoView({ behavior: 'smooth' });

        } catch (err) {
            console.error('[Cave Story WASM] Error crítico:', err);
            hideLoading();
            showToast(`Error al iniciar: ${err.message || err}`, '❌', 6000);
        }
    }

    async function exitToHub() {
        if (currentEmulator) {
            showLoading('Saliendo de la caverna...', '');
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
        showToast('Volviste a la selección de juegos', '📂');
    }

    function updatePauseBtnUI() {
        if (!pauseResumeBtn) return;
        if (isPaused) {
            pauseResumeBtn.textContent = '▶ Continuar';
            pauseResumeBtn.style.background = 'rgba(56, 189, 248, 0.3)';
        } else {
            pauseResumeBtn.textContent = '⏸ Pausa';
            pauseResumeBtn.style.background = '';
        }
    }

    // Event Listeners
    if (playCaveBtn) {
        playCaveBtn.addEventListener('click', () => {
            launchCaveStory(null, 'Cave Story (English Deluxe)');
        });
    }

    if (exitGameBtn) {
        exitGameBtn.addEventListener('click', exitToHub);
    }

    if (pauseResumeBtn) {
        pauseResumeBtn.addEventListener('click', () => {
            if (!currentEmulator) return;
            if (isPaused) {
                currentEmulator.resume();
                isPaused = false;
                showToast('Juego reanudado', '▶');
            } else {
                currentEmulator.pause();
                isPaused = true;
                showToast('Juego en pausa', '⏸');
            }
            updatePauseBtnUI();
            focusGameCanvas();
        });
    }

    if (resetBtn) {
        resetBtn.addEventListener('click', () => {
            if (!currentEmulator) return;
            currentEmulator.restart();
            showToast('Cave Story reiniciado', '🔄');
            focusGameCanvas();
        });
    }

    if (crtToggleBtn) {
        crtToggleBtn.addEventListener('click', () => {
            crtFilterEnabled = !crtFilterEnabled;
            if (scanlinesOverlay) {
                scanlinesOverlay.style.display = crtFilterEnabled ? 'block' : 'none';
            }
            crtToggleBtn.textContent = crtFilterEnabled ? '📺 Filtro CRT: ON' : '📺 Filtro CRT: OFF';
            crtToggleBtn.style.background = crtFilterEnabled ? 'rgba(56, 189, 248, 0.25)' : '';
            showToast(crtFilterEnabled ? 'Filtro CRT activado' : 'Filtro CRT desactivado', '📺');
            focusGameCanvas();
        });
    }

    if (saveStateBtn) {
        saveStateBtn.addEventListener('click', async () => {
            if (!currentEmulator) return;
            try {
                showToast('Guardando estado...', '💾');
                lastSavedState = await currentEmulator.saveState();
                showToast('Estado guardado con éxito', '✅');
            } catch (err) {
                showToast('Error al guardar estado', '❌');
            }
            focusGameCanvas();
        });
    }

    if (loadStateBtn) {
        loadStateBtn.addEventListener('click', async () => {
            if (!currentEmulator) return;
            if (!lastSavedState) {
                showToast('No hay estados guardados aún', '⚠️');
                focusGameCanvas();
                return;
            }
            try {
                showToast('Cargando estado guardado...', '📂');
                await currentEmulator.loadState(lastSavedState.state);
                showToast('Estado cargado con éxito', '✅');
            } catch (err) {
                showToast('Error al cargar estado', '❌');
            }
            focusGameCanvas();
        });
    }

    if (screenshotBtn) {
        screenshotBtn.addEventListener('click', async () => {
            if (!currentEmulator) return;
            try {
                const { screenshot } = await currentEmulator.screenshot();
                const url = URL.createObjectURL(screenshot);
                const a = document.createElement('a');
                a.href = url;
                a.download = `cavestory-${Date.now()}.png`;
                a.click();
                URL.revokeObjectURL(url);
                showToast('Captura descargada', '📷');
            } catch (err) {
                showToast('Error al capturar pantalla', '❌');
            }
            focusGameCanvas();
        });
    }

    if (screenSizeBtn) {
        screenSizeBtn.addEventListener('click', () => {
            currentSizeIndex = (currentSizeIndex + 1) % screenSizes.length;
            updateScreenSizeUI();
            showToast(`Tamaño: ${screenSizes[currentSizeIndex].name}`, '📐');
            focusGameCanvas();
        });
    }

    if (fullscreenBtn) {
        fullscreenBtn.addEventListener('click', () => {
            if (!document.fullscreenElement) {
                if (canvasContainer.requestFullscreen) {
                    canvasContainer.requestFullscreen();
                } else if (canvasContainer.webkitRequestFullscreen) {
                    canvasContainer.webkitRequestFullscreen();
                }
            } else {
                if (document.exitFullscreen) {
                    document.exitFullscreen();
                }
            }
            focusGameCanvas();
        });
    }

    // Drag and Drop Custom Mod Zip
    if (csDropzone) {
        csDropzone.addEventListener('dragover', (e) => {
            e.preventDefault();
            csDropzone.classList.add('dragover');
        });
        csDropzone.addEventListener('dragleave', () => {
            csDropzone.classList.remove('dragover');
        });
        csDropzone.addEventListener('drop', (e) => {
            e.preventDefault();
            csDropzone.classList.remove('dragover');
            if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
                handleZipFile(e.dataTransfer.files[0]);
            }
        });
    }

    if (browseCsBtn && csFileInput) {
        browseCsBtn.addEventListener('click', () => csFileInput.click());
        csFileInput.addEventListener('change', (e) => {
            if (e.target.files && e.target.files.length > 0) {
                handleZipFile(e.target.files[0]);
            }
        });
    }

    function handleZipFile(file) {
        if (!file.name.toLowerCase().endsWith('.zip')) {
            showToast('Por favor selecciona un archivo .zip', '⚠️');
            return;
        }
        showToast(`Cargando mod ${file.name}...`, '📦');
        launchCaveStory(file, file.name);
    }

    // Gamepad Detection
    window.addEventListener('gamepadconnected', (e) => {
        if (gamepadDot) gamepadDot.style.background = '#4ade80';
        if (gamepadText) {
            gamepadText.textContent = `PAD: ${e.gamepad.id.slice(0, 15)}...`;
            gamepadText.style.color = '#4ade80';
        }
        showToast(`Gamepad conectado: ${e.gamepad.id}`, '🎮');
    });

    window.addEventListener('gamepaddisconnected', () => {
        if (gamepadDot) gamepadDot.style.background = '#64748b';
        if (gamepadText) {
            gamepadText.textContent = 'GAMEPAD DESCONECTADO';
            gamepadText.style.color = '';
        }
        showToast('Gamepad desconectado', '⚠️');
    });

    // Keyboard Hotkeys
    window.addEventListener('keydown', (e) => {
        if (!currentEmulator) return;
        if (e.key === 'F5') {
            e.preventDefault();
            if (saveStateBtn) saveStateBtn.click();
        } else if (e.key === 'F7') {
            e.preventDefault();
            if (loadStateBtn) loadStateBtn.click();
        }
    });

    // Init Screen Size
    updateScreenSizeUI();

})();
