/**
 * Virtual GameBoy Modern On-Screen Controller & Mobile Device Interceptor
 * Provides responsive touch controls with tactile haptic feedback,
 * native keyboard event dispatching for WASM emulators, and mobile-friendly onboarding.
 */

(() => {
    'use strict';

    // Calculate relative path to root Packaging/emscripten/ directory
    function getRootRelativePath() {
        const path = window.location.pathname.toLowerCase();
        if (path.includes('/doom/') || path.includes('/wolf3d/') || path.includes('/gens/') || path.includes('/minixp/') || path.includes('/tinycore/') || path.includes('/quake/') || path.includes('/cavestory/') || path.includes('/nes/') || path.includes('/snes/') || path.includes('/n64/') || path.includes('/psx/') || path.includes('/psp/') || path.includes('/flash/') || path.includes('/gunblood/')) {
            return '../';
        }
        return './';
    }

    const BASE_PATH = getRootRelativePath();

    // Check if device is mobile or tablet
    function isMobileDevice() {
        const ua = navigator.userAgent || navigator.vendor || window.opera || '';
        const isMobileUA = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini|Mobile|mobile|CriOS/i.test(ua);
        const hasTouch = ('ontouchstart' in window) || (navigator.maxTouchPoints > 0) || (navigator.msMaxTouchPoints > 0);
        const isSmallScreen = window.innerWidth <= 1024;
        return (isMobileUA || (hasTouch && isSmallScreen));
    }

    // Check if the current page is Diablo / DevilutionX
    function isDiabloPage() {
        const path = window.location.pathname.toLowerCase();
        return !path.includes('/doom/') && 
               !path.includes('/wolf3d/') && 
               !path.includes('/gens/') && 
               !path.includes('/minixp/') && 
               !path.includes('/tinycore/') && 
               !path.includes('/quake/') && 
               !path.includes('/cavestory/') && 
               !path.includes('/nes/') && 
               !path.includes('/snes/') && 
               !path.includes('/n64/') && 
               !path.includes('/psx/') && 
               !path.includes('/psp/') && 
               !path.includes('/flash/') && 
               !path.includes('/gunblood/');
    }

    // Key mapping for retro consoles (NES, SNES, Genesis, DOOM, Wolf3D, Cave Story)
    const KEY_DEFINITIONS = {
        'up':     { key: 'ArrowUp',    code: 'ArrowUp',    keyCode: 38 },
        'down':   { key: 'ArrowDown',  code: 'ArrowDown',  keyCode: 40 },
        'left':   { key: 'ArrowLeft',  code: 'ArrowLeft',  keyCode: 37 },
        'right':  { key: 'ArrowRight', code: 'ArrowRight', keyCode: 39 },
        'a':      { key: 'x',          code: 'KeyX',       keyCode: 88 }, // NES A / SNES B / Genesis B
        'b':      { key: 'z',          code: 'KeyZ',       keyCode: 90 }, // NES B / SNES Y / Genesis A
        'x':      { key: 's',          code: 'KeyS',       keyCode: 83 }, // SNES X / Genesis X / DOOM Open
        'y':      { key: 'a',          code: 'KeyA',       keyCode: 65 }, // SNES A / Genesis C
        'l':      { key: 'q',          code: 'KeyQ',       keyCode: 81 }, // SNES L
        'r':      { key: 'w',          code: 'KeyW',       keyCode: 87 }, // SNES R
        'select': { key: 'Shift',      code: 'ShiftRight', keyCode: 16 }, // Select / Coin / Mode
        'start':  { key: 'Enter',      code: 'Enter',      keyCode: 13 }  // Start / Enter
    };

    const activeKeysState = {};

    function hapticFeedback(ms = 12) {
        if (typeof navigator.vibrate === 'function') {
            try {
                navigator.vibrate(ms);
            } catch (e) {
                // Ignore vibration errors
            }
        }
    }

    // Dispatch native keyboard events to window, document and active canvas
    function dispatchKeyEvent(actionName, isDown) {
        const def = KEY_DEFINITIONS[actionName];
        if (!def) return;

        if (isDown) {
            if (activeKeysState[actionName]) return; // Already pressed
            activeKeysState[actionName] = true;
            hapticFeedback(12);
        } else {
            if (!activeKeysState[actionName]) return; // Already released
            activeKeysState[actionName] = false;
        }

        const eventType = isDown ? 'keydown' : 'keyup';
        const eventInit = {
            key: def.key,
            code: def.code,
            keyCode: def.keyCode,
            which: def.keyCode,
            bubbles: true,
            cancelable: true,
            composed: true
        };

        const keyEvent = new KeyboardEvent(eventType, eventInit);
        
        // Target active canvas or fallback to document
        const canvas = document.querySelector('canvas.emscripten') || document.querySelector('canvas') || document.body;
        if (canvas) {
            canvas.dispatchEvent(keyEvent);
        }
        document.dispatchEvent(keyEvent);
        window.dispatchEvent(keyEvent);

        // Custom hook event for game engines
        window.dispatchEvent(new CustomEvent('virtual-gamepad-input', {
            detail: { action: actionName, isDown, def }
        }));
    }

    // Create Virtual Gamepad Overlay HTML
    function buildGamepadHTML() {
        return `
            <!-- Floating Gamepad Toggle Button -->
            <button class="virtual-gamepad-toggle" id="vpadToggleBtn" title="Mostrar/Ocultar GamePad Táctil">
                <span>🎮</span>
                <span id="vpadToggleLabel">GAMEPAD</span>
            </button>

            <!-- Virtual Handheld HUD -->
            <div id="virtual-gamepad-hud">
                <!-- Top L / R Bumpers (Discreet) -->
                <div class="vpad-shoulder-container">
                    <button class="vpad-shoulder-btn" data-action="l" title="L Shoulder">L</button>
                    <button class="vpad-shoulder-btn" data-action="r" title="R Shoulder">R</button>
                </div>

                <!-- Left Zone: D-Pad & Select -->
                <div class="vpad-left-cluster">
                    <div class="vpad-dpad" id="vpadDpad">
                        <button class="vpad-dpad-btn up" data-action="up" aria-label="Arriba">▲</button>
                        <button class="vpad-dpad-btn left" data-action="left" aria-label="Izquierda">◀</button>
                        <div class="vpad-dpad-center"></div>
                        <button class="vpad-dpad-btn right" data-action="right" aria-label="Derecha">▶</button>
                        <button class="vpad-dpad-btn down" data-action="down" aria-label="Abajo">▼</button>
                    </div>
                    <div class="vpad-center-cluster">
                        <button class="vpad-pill-btn" data-action="select" title="Select / Shift">
                            <span>SELECT</span>
                        </button>
                    </div>
                </div>

                <!-- Right Zone: Action Buttons & Start -->
                <div class="vpad-right-cluster">
                    <div class="vpad-actions-container">
                        <button class="vpad-btn btn-x" data-action="x" title="Botón X">
                            <span>X</span>
                            <span class="btn-sublabel">TURBO</span>
                        </button>
                        <button class="vpad-btn btn-y" data-action="y" title="Botón Y">
                            <span>Y</span>
                        </button>
                        <button class="vpad-btn btn-b" data-action="b" title="Botón B">
                            <span>B</span>
                        </button>
                        <button class="vpad-btn btn-a" data-action="a" title="Botón A">
                            <span>A</span>
                        </button>
                    </div>
                    <div class="vpad-center-cluster">
                        <button class="vpad-pill-btn" data-action="start" title="Start / Enter">
                            <span>START</span>
                        </button>
                    </div>
                </div>
            </div>
        `;
    }

    // Attach Touch & Pointer Handlers
    function setupGamepadInteractions() {
        const hud = document.getElementById('virtual-gamepad-hud');
        const toggleBtn = document.getElementById('vpadToggleBtn');
        const toggleLabel = document.getElementById('vpadToggleLabel');

        if (!hud || !toggleBtn) return;

        // Auto show on mobile, auto-hide on desktop unless toggled
        const isMob = isMobileDevice();
        if (isMob) {
            hud.classList.remove('hidden');
            if (toggleLabel) toggleLabel.textContent = 'PAD: ON';
        } else {
            hud.classList.add('hidden');
            if (toggleLabel) toggleLabel.textContent = 'PAD: OFF';
        }

        toggleBtn.addEventListener('click', (e) => {
            e.stopPropagation();
            const isHidden = hud.classList.toggle('hidden');
            if (toggleLabel) toggleLabel.textContent = isHidden ? 'PAD: OFF' : 'PAD: ON';
            hapticFeedback(15);
        });

        // Bind interactive buttons
        const actionElements = hud.querySelectorAll('[data-action]');
        actionElements.forEach(el => {
            const action = el.getAttribute('data-action');

            const startPress = (e) => {
                e.preventDefault();
                el.classList.add('active');
                dispatchKeyEvent(action, true);
            };

            const endPress = (e) => {
                e.preventDefault();
                el.classList.remove('active');
                dispatchKeyEvent(action, false);
            };

            el.addEventListener('touchstart', startPress, { passive: false });
            el.addEventListener('touchend', endPress, { passive: false });
            el.addEventListener('touchcancel', endPress, { passive: false });

            el.addEventListener('mousedown', startPress);
            el.addEventListener('mouseup', endPress);
            el.addEventListener('mouseleave', endPress);
        });

        // Touch Drag support for D-Pad
        const dpadContainer = document.getElementById('vpadDpad');
        if (dpadContainer) {
            let activeTouchId = null;

            const handleDpadTouch = (e) => {
                e.preventDefault();
                const rect = dpadContainer.getBoundingClientRect();
                const centerX = rect.left + rect.width / 2;
                const centerY = rect.top + rect.height / 2;

                const touch = Array.from(e.touches).find(t => t.identifier === activeTouchId) || e.touches[0];
                if (!touch) return;

                const dx = touch.clientX - centerX;
                const dy = touch.clientY - centerY;
                const distance = Math.hypot(dx, dy);

                // Deadzone of 12px
                if (distance < 12) {
                    ['up', 'down', 'left', 'right'].forEach(dir => {
                        const btn = dpadContainer.querySelector(`.vpad-dpad-btn.${dir}`);
                        if (btn) btn.classList.remove('active');
                        dispatchKeyEvent(dir, false);
                    });
                    return;
                }

                const angle = Math.atan2(dy, dx) * (180 / Math.PI); // -180 to 180

                // Determine active directions based on angle
                const isRight = (angle >= -67.5 && angle <= 67.5);
                const isLeft = (angle >= 112.5 || angle <= -112.5);
                const isDown = (angle >= 22.5 && angle <= 157.5);
                const isUp = (angle <= -22.5 && angle >= -157.5);

                const dirs = { up: isUp, down: isDown, left: isLeft, right: isRight };

                Object.entries(dirs).forEach(([dir, active]) => {
                    const btn = dpadContainer.querySelector(`.vpad-dpad-btn.${dir}`);
                    if (btn) {
                        btn.classList.toggle('active', active);
                    }
                    dispatchKeyEvent(dir, active);
                });
            };

            dpadContainer.addEventListener('touchstart', (e) => {
                activeTouchId = e.changedTouches[0].identifier;
                handleDpadTouch(e);
            }, { passive: false });

            dpadContainer.addEventListener('touchmove', (e) => {
                handleDpadTouch(e);
            }, { passive: false });

            const releaseDpad = (e) => {
                ['up', 'down', 'left', 'right'].forEach(dir => {
                    const btn = dpadContainer.querySelector(`.vpad-dpad-btn.${dir}`);
                    if (btn) btn.classList.remove('active');
                    dispatchKeyEvent(dir, false);
                });
                activeTouchId = null;
            };

            dpadContainer.addEventListener('touchend', releaseDpad, { passive: false });
            dpadContainer.addEventListener('touchcancel', releaseDpad, { passive: false });
        }
    }

    // Show Mobile Onboarding Modal for Diablo / Hellfire
    function checkDiabloMobileOnboarding() {
        if (!isDiabloPage() || !isMobileDevice()) return;

        // Check if user already dismissed modal during session
        if (sessionStorage.getItem('diablo_mobile_dismissed')) return;

        const modalHTML = `
            <div class="mobile-diablo-modal-backdrop" id="diabloMobileModal">
                <div class="mobile-diablo-modal">
                    <h2>📱 DISPOSITIVO MÓVIL DETECTADO</h2>
                    <p>
                        <strong>Diablo I & Hellfire</strong> fueron concebidos para ratón/teclado de precisión y requieren alta potencia de renderizado en navegador.
                    </p>
                    <div class="mobile-diablo-badge-list">
                        <span class="mobile-diablo-badge">⭐ 100% 60 FPS Táctil</span>
                        <span class="mobile-diablo-badge">🔴 NES (8-Bit)</span>
                        <span class="mobile-diablo-badge">⚡ Genesis (16-Bit)</span>
                        <span class="mobile-diablo-badge">🎮 SNES (16-Bit)</span>
                    </div>
                    <p style="font-size: 13px; opacity: 0.9;">
                        Para una experiencia fluida y controles GameBoy táctiles perfectos, te recomendamos jugar a los clásicos 8/16-bit:
                    </p>
                    <div class="mobile-diablo-actions">
                        <a href="${BASE_PATH}gens/index.html" class="mobile-diablo-btn-primary">
                            🎮 Jugar Sega Genesis / Mega Drive (60 FPS)
                        </a>
                        <a href="${BASE_PATH}nes/index.html" class="mobile-diablo-btn-primary" style="background: linear-gradient(135deg, #b91c1c 0%, #991b1b 100%); border-color: #f87171;">
                            🔴 Jugar Nintendo NES & Famicom (8-Bit)
                        </a>
                        <button class="mobile-diablo-btn-secondary" id="dismissDiabloModalBtn">
                            ⚔️ Continuar en Diablo I (Modo Experimental)
                        </button>
                    </div>
                </div>
            </div>
        `;

        const modalDiv = document.createElement('div');
        modalDiv.innerHTML = modalHTML;
        document.body.appendChild(modalDiv);

        const dismissBtn = document.getElementById('dismissDiabloModalBtn');
        const modalBackdrop = document.getElementById('diabloMobileModal');
        if (dismissBtn && modalBackdrop) {
            dismissBtn.addEventListener('click', () => {
                sessionStorage.setItem('diablo_mobile_dismissed', '1');
                modalBackdrop.style.display = 'none';
                hapticFeedback(20);
            });
        }
    }

    // Initialize Virtual Gamepad & Mobile Controller
    function initVirtualGamepad() {
        if (isMobileDevice()) {
            document.body.classList.add('is-mobile-device');
        }

        // Inject Stylesheet if not present
        if (!document.getElementById('virtual-gamepad-css')) {
            const link = document.createElement('link');
            link.id = 'virtual-gamepad-css';
            link.rel = 'stylesheet';
            link.href = `${BASE_PATH}assets/mobile-controls/virtual-gamepad.css?v=gb-v1`;
            document.head.appendChild(link);
        }

        // Inject HTML
        const vpadContainer = document.createElement('div');
        vpadContainer.id = 'virtual-gamepad-root';
        vpadContainer.innerHTML = buildGamepadHTML();
        document.body.appendChild(vpadContainer);

        // Bind events
        setupGamepadInteractions();

        // Check Diablo Mobile modal
        checkDiabloMobileOnboarding();
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', initVirtualGamepad);
    } else {
        initVirtualGamepad();
    }
})();
