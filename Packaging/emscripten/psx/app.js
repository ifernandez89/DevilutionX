/**
 * PSX Web — Sony PlayStation (PS1 / PSX) 32-Bit WebAssembly Controller
 * Features:
 * - Direct execution of PS1 games via PCSX-ReARMed WASM core with WebGL 3D acceleration
 * - Smart multi-file handling (.CUE + .BIN, .ISO, .CHD, .PBP)
 * - Screen scaling: Normal (800p), Grande (980p, default), Cinema (1180p)
 * - Virtual Memory Card 1 (.mcr) persistence via IndexedDB
 * - DualShock USB/Bluetooth gamepads & keyboard controls
 */

(() => {
    'use strict';

    // State
    let activeRomName = 'Juego PlayStation';
    let currentBlobUrls = [];

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
    function showToast(msg, icon = '💿', duration = 3500) {
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

    function cleanupBlobs() {
        currentBlobUrls.forEach(url => {
            try {
                URL.revokeObjectURL(url);
            } catch (_) {}
        });
        currentBlobUrls = [];
    }

    // ==========================================
    // Disc Preparation & Launching
    // ==========================================
    async function launchPsxGame(files) {
        if (!files || files.length === 0) return;

        cleanupBlobs();

        const fileList = Array.from(files);
        let primaryFile = null;
        let cueFile = null;
        let binFile = null;

        // Inspect dropped files
        for (const f of fileList) {
            const ext = f.name.slice(f.name.lastIndexOf('.')).toLowerCase();
            if (ext === '.cue') cueFile = f;
            else if (ext === '.bin' || ext === '.img' || ext === '.iso' || ext === '.chd' || ext === '.pbp') {
                binFile = f;
            }
        }

        // Determine main booting target
        // For PS1 single data track games like Diablo, booting the .bin/.iso or .chd directly is fully supported
        primaryFile = cueFile || binFile || fileList[0];

        const cleanName = primaryFile.name.replace(/\.[^/.]+$/, '');
        activeRomName = cleanName;

        showLoading(`Iniciando ${cleanName}...`, 'Preparando imagen de CD-ROM y montando Memory Card...');

        try {
            // Create Blob URL for the primary image
            const blobUrl = URL.createObjectURL(primaryFile);
            currentBlobUrls.push(blobUrl);

            // Switch to game screen
            hubSection.style.display = 'none';
            emulatorSection.style.display = 'flex';
            updateScreenSizeUI();

            if (activeGameLabel) {
                activeGameLabel.textContent = `💿 ${cleanName} (PS1 32-Bit WASM)`;
            }

            // Clear previous player element
            gamePlayer.innerHTML = '';
            const playerContainer = document.createElement('div');
            playerContainer.id = 'ejs-game-container';
            playerContainer.style.width = '100%';
            playerContainer.style.height = '100%';
            gamePlayer.appendChild(playerContainer);

            // Configure EmulatorJS globals for PlayStation 1
            window.EJS_player = '#ejs-game-container';
            window.EJS_core = 'psx';
            window.EJS_gameUrl = blobUrl;
            window.EJS_gameName = cleanName;
            window.EJS_pathtodata = 'https://cdn.emulatorjs.org/stable/data/';
            window.EJS_startOnLoaded = true;
            window.EJS_language = 'es-ES';
            window.EJS_align = 'center';
            window.EJS_color = '#3b82f6';

            // Remove existing loader script if any, and reinject
            const oldScript = document.getElementById('ejs-loader-script');
            if (oldScript) oldScript.remove();

            const script = document.createElement('script');
            script.id = 'ejs-loader-script';
            script.src = 'data/loader.js';
            script.onload = () => {
                hideLoading();
                showToast(`¡${cleanName} ejecutándose en Sony PlayStation!`, '💿');
            };
            script.onerror = () => {
                // Fallback to CDN loader if local fails
                console.warn('[PSX Loader] Error con loader local, intentando CDN...');
                const cdnScript = document.createElement('script');
                cdnScript.id = 'ejs-loader-script-cdn';
                cdnScript.src = 'https://cdn.emulatorjs.org/stable/data/loader.js';
                cdnScript.onload = () => {
                    hideLoading();
                    showToast(`¡${cleanName} ejecutándose en Sony PlayStation!`, '💿');
                };
                cdnScript.onerror = (e) => {
                    hideLoading();
                    alert(`Error cargando el motor PSX: ${e.message || 'No se pudo conectar'}`);
                    exitToHub();
                };
                document.body.appendChild(cdnScript);
            };

            document.body.appendChild(script);

        } catch (err) {
            hideLoading();
            console.error('[PSX Launcher] Error:', err);
            alert(`Error al procesar la imagen de PlayStation: ${err.message || err}`);
            exitToHub();
        }
    }

    function exitToHub() {
        cleanupBlobs();
        gamePlayer.innerHTML = '';
        emulatorSection.style.display = 'none';
        hubSection.style.display = 'flex';
        showToast('Juego finalizado. Volviendo al menú de PlayStation.', '⏹');
    }

    // ==========================================
    // File Handlers: Drag & Drop and Browse
    // ==========================================
    if (fileInput) {
        fileInput.addEventListener('change', (e) => {
            if (e.target.files && e.target.files.length > 0) {
                launchPsxGame(e.target.files);
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
                launchPsxGame(files);
            }
        });
    }

    // Global drag & drop on window
    window.addEventListener('dragover', (e) => e.preventDefault(), false);
    window.addEventListener('drop', (e) => {
        e.preventDefault();
        if (e.dataTransfer && e.dataTransfer.files && e.dataTransfer.files.length > 0) {
            launchPsxGame(e.dataTransfer.files);
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
            if (confirm('¿Deseas cerrar el juego actual de PlayStation y volver al menú principal?')) {
                exitToHub();
            }
        });
    }

    // Initial setup
    updateScreenSizeUI();

})();
