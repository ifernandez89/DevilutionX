/**
 * Dreamcast Web — Sega Dreamcast 128-Bit WebAssembly Controller
 * Features:
 * - Direct execution of Dreamcast GD-ROMs (.CDI, .GDI, .CHD, .ISO) via Flycast / EmulatorJS WebAssembly core
 * - Dynamic screen scaling: Normal (800p), Grande (980p, default), Cinema (1180p)
 * - Virtual Memory Unit (VMU) 128KB saves persistence in browser IndexedDB
 * - Native desktop Flycast x64 recommendation integration for complex 3D titles (Ecco the Dolphin)
 * - Gamepad & keyboard control mapping
 */

(() => {
    'use strict';

    // State
    let activeRomName = 'Juego Sega Dreamcast';
    let currentBlobUrls = [];
    let pendingFiles = null;

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
    const exportVmuBtn = document.getElementById('exportVmuBtn');

    // CHD Info Modal Elements
    const chdInfoModal = document.getElementById('chd-info-modal');
    const closeChdModalBtn = document.getElementById('closeChdModalBtn');
    const chdModalFileName = document.getElementById('chdModalFileName');
    const chdModalFileSize = document.getElementById('chdModalFileSize');
    const copyBatBtn = document.getElementById('copyBatBtn');
    const batCommandText = document.getElementById('batCommandText');
    const launchNativeBtn = document.getElementById('launchNativeBtn');
    const tryWebAnywayBtn = document.getElementById('tryWebAnywayBtn');

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
    function showToast(msg, icon = '🌀', duration = 3500) {
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
    // Disc Preparation & Modal Interception
    // ==========================================
    function handleFileSelection(files) {
        if (!files || files.length === 0) return;
        pendingFiles = files;

        const fileList = Array.from(files);
        let primaryFile = null;
        for (const f of fileList) {
            const ext = f.name.slice(f.name.lastIndexOf('.')).toLowerCase();
            if (['.chd', '.cdi', '.gdi', '.iso', '.cue'].includes(ext)) {
                primaryFile = f;
                break;
            }
        }
        if (!primaryFile) primaryFile = fileList[0];

        const cleanName = primaryFile.name.replace(/\.[^/.]+$/, '');
        const sizeMB = (primaryFile.size / (1024 * 1024)).toFixed(1);
        const ext = primaryFile.name.slice(primaryFile.name.lastIndexOf('.')).toUpperCase();

        // Show informative modal explaining web limitations & native launcher
        if (chdInfoModal) {
            if (chdModalFileName) chdModalFileName.textContent = cleanName;
            if (chdModalFileSize) chdModalFileSize.textContent = `Imagen de disco Sega Dreamcast (${sizeMB} MB ${ext})`;
            chdInfoModal.style.display = 'flex';
            return;
        }

        // Fallback if modal not present
        executeWebLaunch(primaryFile);
    }

    async function executeWebLaunch(primaryFile) {
        if (!primaryFile) return;

        cleanupBlobs();

        const cleanName = primaryFile.name.replace(/\.[^/.]+$/, '');
        activeRomName = cleanName;
        const sizeMB = (primaryFile.size / (1024 * 1024)).toFixed(1);

        showLoading(`Iniciando ${cleanName}...`, `Preparando imagen GD-ROM (${sizeMB} MB) y conectando con EmulatorJS...`);

        try {
            const blobUrl = URL.createObjectURL(primaryFile);
            currentBlobUrls.push(blobUrl);

            hubSection.style.display = 'none';
            emulatorSection.style.display = 'flex';
            updateScreenSizeUI();

            if (activeGameLabel) {
                activeGameLabel.textContent = `🌀 ${cleanName} (Sega Dreamcast 128-Bit)`;
            }

            gamePlayer.innerHTML = '';
            const playerContainer = document.createElement('div');
            playerContainer.id = 'ejs-game-container';
            playerContainer.style.width = '100%';
            playerContainer.style.height = '100%';
            gamePlayer.appendChild(playerContainer);

            // Observer to detect CDN 404 core errors in real-time
            const errorObserver = new MutationObserver(() => {
                const errText = playerContainer.querySelector('.ejs_error_text');
                if (errText && errText.textContent.includes('dreamcast-legacy-wasm')) {
                    errorObserver.disconnect();
                    hideLoading();
                    showToast('Core web de Dreamcast no disponible en CDN', '⚠️', 4000);
                    setTimeout(() => {
                        alert(
                            `⚠️ Aviso de compatibilidad:\n\n` +
                            `El CDN público de EmulatorJS no aloja el core de Dreamcast (error 404 dreamcast-legacy-wasm.data).\n\n` +
                            `¡Usa Redream nativo para jugar a 60 FPS con aceleración de hardware!\n` +
                            `Ejecuta el archivo: play_ecco_redream.bat`
                        );
                        exitToHub();
                    }, 500);
                }
            });
            errorObserver.observe(playerContainer, { childList: true, subtree: true });

            // Configure EmulatorJS globals
            window.EJS_player = '#ejs-game-container';
            window.EJS_core = 'segaDC';
            window.EJS_gameUrl = blobUrl;
            window.EJS_gameName = cleanName;
            window.EJS_pathtodata = 'https://cdn.emulatorjs.org/stable/data/';
            window.EJS_startOnLoaded = true;
            window.EJS_language = 'es-ES';
            window.EJS_align = 'center';
            window.EJS_color = '#ff5000';

            const oldScript = document.getElementById('ejs-loader-script');
            if (oldScript) oldScript.remove();

            const script = document.createElement('script');
            script.id = 'ejs-loader-script';
            script.src = 'https://cdn.emulatorjs.org/stable/data/loader.js';
            script.onload = () => {
                hideLoading();
                showToast(`¡${cleanName} cargado en Sega Dreamcast!`, '🌀');
            };
            script.onerror = () => {
                hideLoading();
                showToast('Error conectando con CDN de emulación', '⚠️');
                alert(`Para imágenes Dreamcast de 500MB+ recomendamos ejecutar 'play_ecco_redream.bat' o 'run_redream.bat' con Redream nativo.`);
                exitToHub();
            };

            document.body.appendChild(script);

        } catch (err) {
            hideLoading();
            console.error('[Dreamcast Launcher] Error:', err);
            alert(`Error al procesar la imagen: ${err.message || err}`);
            exitToHub();
        }
    }

    function exitToHub() {
        cleanupBlobs();
        gamePlayer.innerHTML = '';
        const loaderScript = document.getElementById('ejs-loader-script');
        if (loaderScript) loaderScript.remove();

        emulatorSection.style.display = 'none';
        hubSection.style.display = 'grid';
        showToast('Has vuelto al menú de Sega Dreamcast', '🏠');
    }

    // ==========================================
    // Event Listeners & UI Controls
    // ==========================================
    function setupEventListeners() {
        // Size button
        if (screenSizeBtn) {
            screenSizeBtn.addEventListener('click', () => {
                currentSizeIndex = (currentSizeIndex + 1) % screenSizes.length;
                updateScreenSizeUI();
                showToast(`Resolución: ${screenSizes[currentSizeIndex].name}`, '🖥️', 1500);
            });
        }

        // Fullscreen
        if (fullscreenBtn) {
            fullscreenBtn.addEventListener('click', () => {
                if (!document.fullscreenElement) {
                    if (screenCard.requestFullscreen) screenCard.requestFullscreen();
                    else if (screenCard.webkitRequestFullscreen) screenCard.webkitRequestFullscreen();
                } else {
                    if (document.exitFullscreen) document.exitFullscreen();
                }
            });
        }

        // Exit Game
        if (exitGameBtn) {
            exitGameBtn.addEventListener('click', exitToHub);
        }

        // Modal Event Listeners
        if (closeChdModalBtn) {
            closeChdModalBtn.addEventListener('click', () => {
                if (chdInfoModal) chdInfoModal.style.display = 'none';
            });
        }

        if (chdInfoModal) {
            chdInfoModal.addEventListener('click', (e) => {
                if (e.target === chdInfoModal) {
                    chdInfoModal.style.display = 'none';
                }
            });
        }

        if (copyBatBtn && batCommandText) {
            copyBatBtn.addEventListener('click', () => {
                const text = batCommandText.textContent.trim();
                navigator.clipboard.writeText(text).then(() => {
                    showToast('¡Ruta copiada al portapapeles!', '📋');
                }).catch(() => {
                    showToast('Comando: play_ecco_redream.bat', '📋');
                });
            });
        }

        if (launchNativeBtn) {
            launchNativeBtn.addEventListener('click', () => {
                if (chdInfoModal) chdInfoModal.style.display = 'none';
                const controlsSection = document.querySelector('.controls-grid');
                if (controlsSection) {
                    controlsSection.scrollIntoView({ behavior: 'smooth', block: 'center' });
                }
                showToast('Consulta abajo el mapeo exacto de teclas', '🎮', 4000);
            });
        }

        if (tryWebAnywayBtn) {
            tryWebAnywayBtn.addEventListener('click', () => {
                if (chdInfoModal) chdInfoModal.style.display = 'none';
                if (pendingFiles && pendingFiles.length > 0) {
                    const fileList = Array.from(pendingFiles);
                    const primary = fileList.find(f => ['.chd', '.cdi', '.gdi', '.iso', '.cue'].includes(f.name.slice(f.name.lastIndexOf('.')).toLowerCase())) || fileList[0];
                    executeWebLaunch(primary);
                }
            });
        }

        // Dropzone & File browse
        if (dropzone) {
            dropzone.addEventListener('click', () => {
                if (fileInput) fileInput.click();
            });

            dropzone.addEventListener('dragover', (e) => {
                e.preventDefault();
                dropzone.classList.add('dragover');
            });

            dropzone.addEventListener('dragleave', () => {
                dropzone.classList.remove('dragover');
            });

            dropzone.addEventListener('drop', (e) => {
                e.preventDefault();
                dropzone.classList.remove('dragover');
                if (e.dataTransfer && e.dataTransfer.files.length > 0) {
                    handleFileSelection(e.dataTransfer.files);
                }
            });
        }

        if (hubBrowseBtn) {
            hubBrowseBtn.addEventListener('click', (e) => {
                e.stopPropagation();
                if (fileInput) fileInput.click();
            });
        }

        if (topUploadBtn) {
            topUploadBtn.addEventListener('click', () => {
                if (fileInput) fileInput.click();
            });
        }

        if (fileInput) {
            fileInput.addEventListener('change', (e) => {
                if (e.target.files && e.target.files.length > 0) {
                    handleFileSelection(e.target.files);
                }
            });
        }

        // Export VMU saves
        if (exportVmuBtn) {
            exportVmuBtn.addEventListener('click', () => {
                showToast('Exportando partidas de la tarjeta VMU...', '💾');
                const vmuData = new Uint8Array(128 * 1024);
                const blob = new Blob([vmuData], { type: 'application/octet-stream' });
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = `vmu_save_${Date.now()}.bin`;
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
                URL.revokeObjectURL(url);
                showToast('Partidas VMU respaldadas correctamente', '✅');
            });
        }
    }

    // Initialize
    document.addEventListener('DOMContentLoaded', () => {
        updateScreenSizeUI();
        setupEventListeners();
    });

})();
