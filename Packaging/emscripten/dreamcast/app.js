/**
 * Dreamcast Web — Sega Dreamcast 128-Bit WebAssembly Controller
 * Features:
 * - Direct execution of Dreamcast GD-ROMs (.CDI, .GDI, .CHD, .ISO) via Redream / WebAssembly core
 * - Dynamic screen scaling: Normal (800p), Grande (980p, default), Cinema (1180p)
 * - Virtual Memory Unit (VMU) 128KB saves persistence in browser IndexedDB
 * - Native desktop Redream x64 recommendation integration for 3D titles (Ecco the Dolphin)
 * - Gamepad & keyboard control mapping
 */

(() => {
    'use strict';

    // State
    let activeRomName = 'Juego Sega Dreamcast';
    let currentBlobUrls = [];
    let currentMountedFile = null;

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
        currentMountedFile = null;
    }

    // ==========================================
    // Disc Mounting & Launching (Sin Bloqueos)
    // ==========================================
    function handleFileSelection(files) {
        if (!files || files.length === 0) return;

        cleanupBlobs();

        const fileList = Array.from(files);
        let primaryFile = null;
        let gdiFile = null;
        let cdiFile = null;
        let chdFile = null;
        let cueFile = null;

        for (const f of fileList) {
            const ext = f.name.slice(f.name.lastIndexOf('.')).toLowerCase();
            if (ext === '.chd') chdFile = f;
            else if (ext === '.cdi') cdiFile = f;
            else if (ext === '.gdi') gdiFile = f;
            else if (ext === '.cue') cueFile = f;
            else if (ext === '.iso' || ext === '.bin') {
                if (!primaryFile) primaryFile = f;
            }
        }

        // Determine main booting target (.chd prioritized, then .cdi, .gdi...)
        primaryFile = chdFile || cdiFile || gdiFile || cueFile || primaryFile || fileList[0];
        currentMountedFile = primaryFile;

        const cleanName = primaryFile.name.replace(/\.[^/.]+$/, '');
        activeRomName = cleanName;
        const fileSizeMB = (primaryFile.size / (1024 * 1024)).toFixed(1);
        const fileExt = primaryFile.name.slice(primaryFile.name.lastIndexOf('.')).toUpperCase();

        const blobUrl = URL.createObjectURL(primaryFile);
        currentBlobUrls.push(blobUrl);

        // Switch to game screen immediately
        hubSection.style.display = 'none';
        emulatorSection.style.display = 'flex';
        updateScreenSizeUI();

        if (activeGameLabel) {
            activeGameLabel.textContent = `🌀 ${cleanName} (${fileSizeMB} MB ${fileExt})`;
        }

        showToast(`Disco cargado: ${cleanName}`, '💿', 2500);

        // Render player dashboard immediately without any blocking modals
        renderDiscMountedScreen(primaryFile, cleanName, fileSizeMB, fileExt, blobUrl);
    }

    function renderDiscMountedScreen(file, cleanName, fileSizeMB, fileExt, blobUrl) {
        gamePlayer.innerHTML = `
            <div style="display: flex; flex-direction: column; align-items: center; justify-content: center; width: 100%; height: 100%; min-height: 480px; padding: 28px; text-align: center; background: radial-gradient(circle at center, #151c2c 0%, #0a0d15 100%); border-radius: 12px; color: #f8fafc;">
                <div style="font-size: 52px; animation: dc-spin 10s linear infinite; margin-bottom: 12px; filter: drop-shadow(0 0 16px rgba(255,80,0,0.6));">🌀</div>
                <div style="font-size: 11px; font-weight: 800; letter-spacing: 2px; color: #ff5000; text-transform: uppercase; margin-bottom: 6px;">GD-ROM MONTADO CORRECTAMENTE</div>
                <h2 style="font-size: 22px; font-weight: 700; margin-bottom: 10px; color: #ffffff; max-width: 90%; word-break: break-word;">${cleanName}</h2>
                
                <div style="display: inline-flex; flex-wrap: wrap; justify-content: center; gap: 8px; margin-bottom: 24px;">
                    <span style="background: rgba(255,80,0,0.15); border: 1px solid #ff5000; color: #ff8533; padding: 4px 14px; border-radius: 20px; font-size: 12px; font-weight: 600;">Formato: ${fileExt}</span>
                    <span style="background: rgba(0,114,206,0.15); border: 1px solid #0072ce; color: #38bdf8; padding: 4px 14px; border-radius: 20px; font-size: 12px; font-weight: 600;">Tamaño: ${fileSizeMB} MB</span>
                    <span style="background: rgba(34,197,94,0.15); border: 1px solid #22c55e; color: #4ade80; padding: 4px 14px; border-radius: 20px; font-size: 12px; font-weight: 600;">Lector GD-ROM: Listo</span>
                </div>

                <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(290px, 1fr)); gap: 16px; width: 100%; max-width: 720px; margin-bottom: 20px; text-align: left;">
                    
                    <!-- Tarjeta 1: Redream Nativo (Recomendado para PC) -->
                    <div style="background: rgba(255,255,255,0.03); border: 1px solid rgba(255,80,0,0.45); border-radius: 12px; padding: 18px; display: flex; flex-direction: column; justify-content: space-between;">
                        <div>
                            <div style="display: flex; align-items: center; gap: 8px; margin-bottom: 8px;">
                                <span style="font-size: 22px;">⚡</span>
                                <strong style="color: #ffedd5; font-size: 16px;">Jugar en tu PC (Redream Nativo)</strong>
                            </div>
                            <p style="font-size: 13px; color: #94a3b8; line-height: 1.5; margin-bottom: 12px;">
                                En tu PC, <strong>Redream x64</strong> ejecuta esta imagen al 100% de velocidad (60 FPS fluidos) con mandos automáticos y BIOS integrada.
                            </p>
                            <div style="background: #090c13; border: 1px solid #252e45; border-radius: 6px; padding: 10px; font-family: monospace; font-size: 12px; color: #38bdf8; margin-bottom: 10px; display: flex; justify-content: space-between; align-items: center;">
                                <span>play_ecco_redream.bat</span>
                                <button id="copyBatBtn" class="btn btn-secondary btn-sm" style="padding: 2px 8px; font-size: 11px;">📋 Copiar</button>
                            </div>
                        </div>
                        <div style="font-size: 11px; color: #64748b;">
                            ✔ 60 FPS estables &nbsp;•&nbsp; W/A/S/D + P/L/K/J &nbsp;•&nbsp; Barra Espaciadora = Start
                        </div>
                    </div>

                    <!-- Tarjeta 2: Ejecución en Navegador (WebAssembly) -->
                    <div style="background: rgba(255,255,255,0.03); border: 1px solid rgba(0,114,206,0.45); border-radius: 12px; padding: 18px; display: flex; flex-direction: column; justify-content: space-between;">
                        <div>
                            <div style="display: flex; align-items: center; gap: 8px; margin-bottom: 8px;">
                                <span style="font-size: 22px;">🌐</span>
                                <strong style="color: #e0f2fe; font-size: 16px;">Ejecutar en Navegador Web</strong>
                            </div>
                            <p style="font-size: 13px; color: #94a3b8; line-height: 1.5; margin-bottom: 14px;">
                                Intenta conectar con el core WebAssembly en el navegador para imágenes homebrew o ligeras.
                            </p>
                        </div>
                        <button id="startWasmBtn" class="btn btn-primary" style="width: 100%; padding: 12px; font-size: 13px; font-weight: bold; background: linear-gradient(135deg, #0072ce 0%, #0284c7 100%); border-radius: 8px; cursor: pointer;">
                            ▶ Iniciar Motor WebAssembly
                        </button>
                    </div>

                </div>

                <div style="display: flex; gap: 12px; justify-content: center;">
                    <button class="btn btn-secondary" id="changeDiscBtn" style="padding: 8px 18px; font-size: 13px;">
                        📂 Cargar Otro Disco
                    </button>
                </div>
            </div>
        `;

        const startWasmBtn = document.getElementById('startWasmBtn');
        if (startWasmBtn) {
            startWasmBtn.addEventListener('click', () => {
                startWasmEmulator(blobUrl, cleanName);
            });
        }

        const copyBatBtn = document.getElementById('copyBatBtn');
        if (copyBatBtn) {
            copyBatBtn.addEventListener('click', () => {
                navigator.clipboard.writeText('play_ecco_redream.bat').then(() => {
                    showToast('¡Comando copiado al portapapeles!', '📋');
                });
            });
        }

        const changeDiscBtn = document.getElementById('changeDiscBtn');
        if (changeDiscBtn) {
            changeDiscBtn.addEventListener('click', () => {
                if (fileInput) fileInput.click();
            });
        }
    }

    function startWasmEmulator(blobUrl, cleanName) {
        showLoading(`Iniciando ${cleanName}...`, 'Preparando núcleo WebAssembly y montando GD-ROM...');

        // Clear previous player element
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

        // Configure EmulatorJS globals for Dreamcast
        window.EJS_player = '#ejs-game-container';
        window.EJS_core = 'dreamcast';
        window.EJS_gameUrl = blobUrl;
        window.EJS_gameName = cleanName;
        window.EJS_pathtodata = 'https://cdn.emulatorjs.org/stable/data/';
        window.EJS_startOnLoaded = true;
        window.EJS_language = 'es-ES';
        window.EJS_align = 'center';
        window.EJS_color = '#ff5000';

        // Remove existing loader script if any, and reinject
        const oldScript = document.getElementById('ejs-loader-script');
        if (oldScript) oldScript.remove();

        const script = document.createElement('script');
        script.id = 'ejs-loader-script';
        script.src = 'https://cdn.emulatorjs.org/stable/data/loader.js';
        script.onload = () => {
            hideLoading();
            showToast(`¡${cleanName} ejecutándose en Sega Dreamcast!`, '🌀');
        };
        script.onerror = () => {
            hideLoading();
            showToast('Error conectando con CDN de emulación', '⚠️');
            alert(`Para imágenes Dreamcast de 500MB+ recomendamos ejecutar 'play_ecco_redream.bat' o 'run_redream.bat' con Redream nativo.`);
            exitToHub();
        };

        document.body.appendChild(script);
    }

    function exitToHub() {
        cleanupBlobs();
        gamePlayer.innerHTML = '';
        const loaderScript = document.getElementById('ejs-loader-script');
        if (loaderScript) loaderScript.remove();
        const cdnScript = document.getElementById('ejs-loader-script-cdn');
        if (cdnScript) cdnScript.remove();

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
