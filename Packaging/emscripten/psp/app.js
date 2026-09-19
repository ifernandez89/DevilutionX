/**
 * PSP Web — Sony PlayStation Portable (PSP) WebAssembly Controller
 * Features:
 * - Direct execution of PSP games via PPSSPP WASM core with WebGL2 3D acceleration
 * - Smart file handling (.ISO, .CSO, .PBP, EBOOT.BIN, .ELF)
 * - 16:9 Screen scaling: Normal (800p), Grande (980p, default), Cinema (1180p)
 * - Virtual Memory Stick persistence via IndexedDB
 * - Full Gamepad (DualShock / Xbox) & Keyboard controls
 */

(() => {
    'use strict';

    // State
    let activeRomName = 'Juego PSP';
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

    // Screen Sizes (16:9 PSP Native Proportions)
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

    function cleanupBlobs() {
        currentBlobUrls.forEach(url => {
            try {
                URL.revokeObjectURL(url);
            } catch (_) {}
        });
        currentBlobUrls = [];
    }

    // ==========================================
    // PSP Disc & File Launcher
    // ==========================================
    async function launchPspGame(files) {
        if (!files || files.length === 0) return;

        cleanupBlobs();

        const fileList = Array.from(files);
        let primaryFile = null;

        // Inspect dropped files for PSP extensions
        for (const f of fileList) {
            const ext = f.name.slice(f.name.lastIndexOf('.')).toLowerCase();
            if (ext === '.iso' || ext === '.cso' || ext === '.pbp' || ext === '.elf' || f.name.toUpperCase() === 'EBOOT.BIN') {
                primaryFile = f;
                break;
            }
        }

        primaryFile = primaryFile || fileList[0];

        const cleanName = primaryFile.name.replace(/\.[^/.]+$/, '');
        activeRomName = cleanName;

        showLoading(`Iniciando ${cleanName}...`, 'Preparando imagen UMD / ejecutable en PPSSPP WebAssembly...');

        try {
            // Create Blob URL for the primary image
            const blobUrl = URL.createObjectURL(primaryFile);
            currentBlobUrls.push(blobUrl);

            // Switch to game screen
            hubSection.style.display = 'none';
            emulatorSection.style.display = 'flex';
            updateScreenSizeUI();

            if (activeGameLabel) {
                activeGameLabel.textContent = `🎮 ${cleanName} (Sony PSP WASM • PPSSPP)`;
            }

            // Clear previous player element
            gamePlayer.innerHTML = '';
            const playerContainer = document.createElement('div');
            playerContainer.id = 'ejs-game-container';
            playerContainer.style.width = '100%';
            playerContainer.style.height = '100%';
            gamePlayer.appendChild(playerContainer);

            // Configure EmulatorJS globals for PlayStation Portable
            window.EJS_player = '#ejs-game-container';
            window.EJS_core = 'psp';
            window.EJS_gameUrl = blobUrl;
            window.EJS_gameName = cleanName;
            window.EJS_pathtodata = 'https://cdn.emulatorjs.org/stable/data/';
            window.EJS_startOnLoaded = true;
            window.EJS_language = 'es-ES';
            window.EJS_align = 'center';
            window.EJS_color = '#00d2ff';

            // Remove existing loader script if any, and reinject
            const oldScript = document.getElementById('ejs-loader-script');
            if (oldScript) oldScript.remove();

            const script = document.createElement('script');
            script.id = 'ejs-loader-script';
            script.src = 'data/loader.js';
            script.onload = () => {
                hideLoading();
                showToast(`¡${cleanName} ejecutándose en Sony PSP (PPSSPP)!`, '🎮');
            };
            script.onerror = () => {
                // Fallback to CDN loader if local fails
                console.warn('[PSP Loader] Error con loader local, intentando CDN...');
                const cdnScript = document.createElement('script');
                cdnScript.id = 'ejs-loader-script-cdn';
                cdnScript.src = 'https://cdn.emulatorjs.org/stable/data/loader.js';
                cdnScript.onload = () => {
                    hideLoading();
                    showToast(`¡${cleanName} ejecutándose en Sony PSP (PPSSPP)!`, '🎮');
                };
                cdnScript.onerror = (e) => {
                    hideLoading();
                    alert(`Error cargando el motor PSP: ${e.message || 'No se pudo conectar'}`);
                    exitToHub();
                };
                document.body.appendChild(cdnScript);
            };

            document.body.appendChild(script);

        } catch (err) {
            hideLoading();
            console.error('[PSP Launcher] Error:', err);
            alert(`Error al procesar la imagen de PSP: ${err.message || err}`);
            exitToHub();
        }
    }

    function exitToHub() {
        cleanupBlobs();
        gamePlayer.innerHTML = '';
        emulatorSection.style.display = 'none';
        hubSection.style.display = 'flex';
        if (fileInput) fileInput.value = '';
        if (window.EJS_emulator && typeof window.EJS_emulator.stop === 'function') {
            try {
                window.EJS_emulator.stop();
            } catch (_) {}
        }
    }

    // ==========================================
    // Event Listeners & Drag and Drop
    // ==========================================
    document.addEventListener('DOMContentLoaded', () => {
        updateScreenSizeUI();

        // Screen Size Toggle Button
        if (screenSizeBtn) {
            screenSizeBtn.addEventListener('click', () => {
                currentSizeIndex = (currentSizeIndex + 1) % screenSizes.length;
                updateScreenSizeUI();
            });
        }

        // Fullscreen Button
        if (fullscreenBtn) {
            fullscreenBtn.addEventListener('click', () => {
                if (!document.fullscreenElement) {
                    if (screenCard && screenCard.requestFullscreen) {
                        screenCard.requestFullscreen().catch(err => {
                            console.warn('Fullscreen error:', err);
                        });
                    }
                } else {
                    if (document.exitFullscreen) {
                        document.exitFullscreen();
                    }
                }
            });
        }

        // Exit Game Button
        if (exitGameBtn) {
            exitGameBtn.addEventListener('click', () => {
                if (confirm('¿Deseas cerrar el juego actual y volver al menú principal?')) {
                    exitToHub();
                }
            });
        }

        // File Input Triggers
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

            // Drag and Drop Handling
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
                if (e.dataTransfer && e.dataTransfer.files && e.dataTransfer.files.length > 0) {
                    launchPspGame(e.dataTransfer.files);
                }
            });
        }

        // Window-level Drag and Drop
        window.addEventListener('dragover', (e) => {
            e.preventDefault();
        });

        window.addEventListener('drop', (e) => {
            if (hubSection.style.display !== 'none' && (!dropzone || !dropzone.contains(e.target))) {
                e.preventDefault();
                if (e.dataTransfer && e.dataTransfer.files && e.dataTransfer.files.length > 0) {
                    launchPspGame(e.dataTransfer.files);
                }
            }
        });

        // File Input Change
        if (fileInput) {
            fileInput.addEventListener('change', (e) => {
                if (e.target.files && e.target.files.length > 0) {
                    launchPspGame(e.target.files);
                }
            });
        }
    });

})();
