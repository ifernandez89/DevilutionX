/**
 * N64 Web — Nintendo 64 3D WebAssembly Controller
 * Features:
 * - Automatic in-memory byte-swapping (converts .v64 and .n64 to native .z64 in RAM)
 * - Mupen64Plus 3D hardware-accelerated WebGL execution
 * - Screen scaling: Normal (800p), Grande (980p, default), Cinema (1180p)
 * - USB/Bluetooth Gamepads & Keyboard mapping
 * - IndexedDB save persistence
 */

(() => {
    'use strict';

    // State
    let activeRomName = 'Juego N64';
    let currentBlobUrl = null;

    // DOM Elements
    const hubSection = document.getElementById('hub-section');
    const emulatorSection = document.getElementById('emulator-section');
    const screenCard = document.getElementById('screenCard');
    const screenToolbar = document.getElementById('screenToolbar');
    const gamePlayer = document.getElementById('game-player');
    const dropzone = document.getElementById('dropzone');
    const fileInput = document.getElementById('fileInput');
    const topUploadBtn = document.getElementById('topUploadBtn');
    const hubBrowseBtn = document.getElementById('hubBrowseBtn');
    const screenSizeBtn = document.getElementById('screenSizeBtn');
    const fullscreenBtn = document.getElementById('fullscreenBtn');
    const exitGameBtn = document.getElementById('exitGameBtn');
    const activeGameLabel = document.getElementById('activeGameLabel');

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
    function showToast(msg, icon = '🎮', duration = 3500) {
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

    // =========================================================
    // Automatic In-Memory Endianness Converter (.v64/.n64 -> .z64)
    // =========================================================
    function convertRomToZ64(arrayBuffer, fileName) {
        const u8 = new Uint8Array(arrayBuffer);
        if (u8.length < 4) return arrayBuffer;

        const b0 = u8[0], b1 = u8[1], b2 = u8[2], b3 = u8[3];

        // 1. Big Endian (.z64): 80 37 12 40 -> Already native!
        if (b0 === 0x80 && b1 === 0x37 && b2 === 0x12 && b3 === 0x40) {
            console.log(`[N64 Endianness] '${fileName}' es Big-Endian nativo (.z64). No requiere conversión.`);
            return arrayBuffer;
        }

        // 2. Byte-Swapped (.v64): 37 80 40 12 -> Swap every 2 bytes
        if (b0 === 0x37 && b1 === 0x80 && b2 === 0x40 && b3 === 0x12) {
            console.log(`[N64 Endianness] '${fileName}' es Byte-Swapped (.v64). Permutando bytes en RAM...`);
            const len = u8.length - (u8.length % 2);
            for (let i = 0; i < len; i += 2) {
                const tmp = u8[i];
                u8[i] = u8[i + 1];
                u8[i + 1] = tmp;
            }
            console.log(`[N64 Endianness] Conversión completada a .z64 nativo.`);
            return arrayBuffer;
        }

        // 3. Little Endian (.n64): 40 12 37 80 -> Reverse every 4 bytes
        if (b0 === 0x40 && b1 === 0x12 && b2 === 0x37 && b3 === 0x80) {
            console.log(`[N64 Endianness] '${fileName}' es Little-Endian (.n64). Invirtiendo palabras de 32 bits en RAM...`);
            const len = u8.length - (u8.length % 4);
            for (let i = 0; i < len; i += 4) {
                const t0 = u8[i];
                const t1 = u8[i + 1];
                const t2 = u8[i + 2];
                const t3 = u8[i + 3];
                u8[i] = t3;
                u8[i + 1] = t2;
                u8[i + 2] = t1;
                u8[i + 3] = t0;
            }
            console.log(`[N64 Endianness] Conversión completada a .z64 nativo.`);
            return arrayBuffer;
        }

        console.log(`[N64 Endianness] Cabecera no estándar en '${fileName}'. Enviando al motor tal cual.`);
        return arrayBuffer;
    }

    // ==========================================
    // ROM Preparation & Launching
    // ==========================================
    async function launchRom(file) {
        if (!file) return;

        const cleanName = file.name.replace(/\.[^/.]+$/, '');
        activeRomName = cleanName;

        showLoading(`Preparando ${cleanName}...`, 'Leyendo búfer binario y alineando Endianness...');

        try {
            const rawBuffer = await file.arrayBuffer();
            const z64Buffer = convertRomToZ64(rawBuffer, file.name);

            // Create fresh Blob with .z64 extension
            if (currentBlobUrl) {
                URL.revokeObjectURL(currentBlobUrl);
            }
            const blob = new Blob([z64Buffer], { type: 'application/octet-stream' });
            currentBlobUrl = URL.createObjectURL(blob);

            // Switch to game screen
            hubSection.style.display = 'none';
            emulatorSection.style.display = 'flex';
            updateScreenSizeUI();

            if (activeGameLabel) {
                activeGameLabel.textContent = `🎮 ${cleanName} (64-Bit WASM)`;
            }

            // Clear previous player element
            gamePlayer.innerHTML = '';
            const playerContainer = document.createElement('div');
            playerContainer.id = 'ejs-game-container';
            playerContainer.style.width = '100%';
            playerContainer.style.height = '100%';
            gamePlayer.appendChild(playerContainer);

            // Configure EmulatorJS globals
            window.EJS_player = '#ejs-game-container';
            window.EJS_core = 'n64';
            window.EJS_threads = (typeof SharedArrayBuffer !== 'undefined');
            window.EJS_gameUrl = currentBlobUrl;
            window.EJS_gameName = cleanName;
            window.EJS_pathtodata = 'https://cdn.emulatorjs.org/stable/data/';
            window.EJS_startOnLoaded = true;
            window.EJS_language = 'es-ES';
            window.EJS_align = 'center';
            window.EJS_color = '#f59e0b';

            // Remove existing loader script if any, and reinject
            const oldScript = document.getElementById('ejs-loader-script');
            if (oldScript) oldScript.remove();

            const script = document.createElement('script');
            script.id = 'ejs-loader-script';
            script.src = 'data/loader.js';
            script.onload = () => {
                hideLoading();
                showToast(`¡${cleanName} cargado en Nintendo 64!`, '🎮');
            };
            script.onerror = () => {
                // Fallback to CDN loader if local fails
                console.warn('[N64 Loader] Error con loader local, intentando CDN...');
                const cdnScript = document.createElement('script');
                cdnScript.id = 'ejs-loader-script-cdn';
                cdnScript.src = 'https://cdn.emulatorjs.org/stable/data/loader.js';
                cdnScript.onload = () => {
                    hideLoading();
                    showToast(`¡${cleanName} cargado en Nintendo 64!`, '🎮');
                };
                cdnScript.onerror = (e) => {
                    hideLoading();
                    alert(`Error cargando el motor N64: ${e.message || 'No se pudo conectar'}`);
                    exitToHub();
                };
                document.body.appendChild(cdnScript);
            };

            document.body.appendChild(script);

        } catch (err) {
            hideLoading();
            console.error('[N64 Launcher] Error:', err);
            alert(`Error al procesar la ROM: ${err.message || err}`);
            exitToHub();
        }
    }

    function exitToHub() {
        if (currentBlobUrl) {
            URL.revokeObjectURL(currentBlobUrl);
            currentBlobUrl = null;
        }
        gamePlayer.innerHTML = '';
        emulatorSection.style.display = 'none';
        hubSection.style.display = 'flex';
        showToast('Juego finalizado. Volviendo al menú.', '⏹');
    }

    // ==========================================
    // File Handlers: Drag & Drop and Browse
    // ==========================================
    if (fileInput) {
        fileInput.addEventListener('change', (e) => {
            if (e.target.files && e.target.files[0]) {
                launchRom(e.target.files[0]);
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
                launchRom(files[0]);
            }
        });
    }

    // Global drag & drop on window
    window.addEventListener('dragover', (e) => e.preventDefault(), false);
    window.addEventListener('drop', (e) => {
        e.preventDefault();
        if (e.dataTransfer && e.dataTransfer.files && e.dataTransfer.files.length > 0) {
            launchRom(e.dataTransfer.files[0]);
        }
    }, false);

    // ==========================================
    // Controls Toolbar Events
    // ==========================================
    if (screenSizeBtn) {
        screenSizeBtn.addEventListener('click', () => {
            currentSizeIndex = (currentSizeIndex + 1) % screenSizes.length;
            updateScreenSizeUI();
            const size = screenSizes[currentSizeIndex];
            showToast(`Tamaño cambiado a: ${size.name}`, '🖥️');
        });
    }

    if (fullscreenBtn) {
        fullscreenBtn.addEventListener('click', () => {
            const el = screenCard || document.documentElement;
            if (!document.fullscreenElement) {
                el.requestFullscreen().catch(err => {
                    alert(`Error de pantalla completa: ${err.message}`);
                });
            } else {
                document.exitFullscreen();
            }
        });
    }

    if (exitGameBtn) {
        exitGameBtn.addEventListener('click', () => {
            if (confirm(`¿Deseas cerrar "${activeRomName}" y volver al menú principal?`)) {
                exitToHub();
            }
        });
    }

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
                gamepadText.textContent = 'Mando USB/Bluetooth: Compatible automáticamente (Xbox, PlayStation, 8BitDo o réplica N64)';
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

    setTimeout(updateGamepadStatus, 1000);

})();
