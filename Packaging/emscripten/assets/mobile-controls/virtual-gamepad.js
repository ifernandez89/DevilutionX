/**
 * Virtual GameBoy & Sega 16-Bit Modern Touch Controller
 * Cross-platform WASM emulator controller with tactile haptics and multi-key dispatch.
 */

(() => {
    'use strict';

    function getRootRelativePath() {
        const path = window.location.pathname.toLowerCase();
        if (path.includes('/doom/') || path.includes('/wolf3d/') || path.includes('/gens/') || path.includes('/minixp/') || path.includes('/tinycore/') || path.includes('/quake/') || path.includes('/cavestory/') || path.includes('/nes/') || path.includes('/snes/') || path.includes('/n64/') || path.includes('/psx/') || path.includes('/psp/') || path.includes('/flash/') || path.includes('/gunblood/')) {
            return '../';
        }
        return './';
    }

    const BASE_PATH = getRootRelativePath();

    // Universal Mobile & Touch Detection
    function isMobileDevice() {
        const ua = (navigator.userAgent || navigator.vendor || window.opera || '').toLowerCase();
        const isMobileUA = /android|webos|iphone|ipad|ipod|blackberry|iemobile|opera mini|mobile|crios|touch|silk|kindle|samsung|pixel/i.test(ua);
        const hasTouch = ('ontouchstart' in window) || (navigator.maxTouchPoints > 0) || (navigator.msMaxTouchPoints > 0);
        const isCoarse = window.matchMedia && window.matchMedia('(pointer: coarse)').matches;
        const isSmallScreen = window.innerWidth <= 1024;
        return isMobileUA || hasTouch || isCoarse || isSmallScreen;
    }

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

    // Comprehensive Key Mapping (Supports NES, Sega Genesis Plus GX, Snes9x, NXEngine, PrBoom)
    const MULTI_KEY_MAPPINGS = {
        'up': [
            { key: 'ArrowUp', code: 'ArrowUp', keyCode: 38 }
        ],
        'down': [
            { key: 'ArrowDown', code: 'ArrowDown', keyCode: 40 }
        ],
        'left': [
            { key: 'ArrowLeft', code: 'ArrowLeft', keyCode: 37 }
        ],
        'right': [
            { key: 'ArrowRight', code: 'ArrowRight', keyCode: 39 }
        ],
        // Sega Button B / Nintendo Button A (Accelerate / Jump / Primary)
        'a': [
            { key: 'x', code: 'KeyX', keyCode: 88 },
            { key: 's', code: 'KeyS', keyCode: 83 }
        ],
        // Sega Button A / Nintendo Button B (Brake / Attack 1)
        'b': [
            { key: 'z', code: 'KeyZ', keyCode: 90 },
            { key: 'a', code: 'KeyA', keyCode: 65 }
        ],
        // Sega Button C / SNES Button Y (Turbo Nitro / Special Action)
        'c': [
            { key: 'c', code: 'KeyC', keyCode: 67 },
            { key: 'd', code: 'KeyD', keyCode: 68 },
            { key: 'a', code: 'KeyA', keyCode: 65 }
        ],
        // Sega Button X / Turbo Button
        'x': [
            { key: 'q', code: 'KeyQ', keyCode: 81 },
            { key: 's', code: 'KeyS', keyCode: 83 }
        ],
        // Shoulder L / R
        'l': [
            { key: 'q', code: 'KeyQ', keyCode: 81 }
        ],
        'r': [
            { key: 'w', code: 'KeyW', keyCode: 87 }
        ],
        // Mode / Select / Shift
        'select': [
            { key: 'Shift', code: 'ShiftRight', keyCode: 16 }
        ],
        // Start / Pause / Enter
        'start': [
            { key: 'Enter', code: 'Enter', keyCode: 13 },
            { key: ' ', code: 'Space', keyCode: 32 }
        ]
    };

    const activeKeysState = {};

    function hapticFeedback(ms = 14) {
        if (typeof navigator.vibrate === 'function') {
            try {
                navigator.vibrate(ms);
            } catch (e) {}
        }
    }

    function dispatchKey(def, eventType) {
        const keyEvent = new KeyboardEvent(eventType, {
            key: def.key,
            code: def.code,
            keyCode: def.keyCode,
            which: def.keyCode,
            charCode: def.keyCode,
            bubbles: true,
            cancelable: true,
            composed: true
        });

        const targets = [
            document.querySelector('canvas.emscripten'),
            document.querySelector('#canvas-container canvas'),
            document.querySelector('canvas'),
            document.activeElement,
            document.body,
            document,
            window
        ].filter(Boolean);

        for (const target of targets) {
            try {
                target.dispatchEvent(keyEvent);
            } catch (e) {}
        }
    }

    function dispatchAction(actionName, isDown) {
        const defs = MULTI_KEY_MAPPINGS[actionName];
        if (!defs) return;

        if (isDown) {
            if (activeKeysState[actionName]) return;
            activeKeysState[actionName] = true;
            hapticFeedback(14);
            defs.forEach(def => dispatchKey(def, 'keydown'));
        } else {
            if (!activeKeysState[actionName]) return;
            activeKeysState[actionName] = false;
            defs.forEach(def => dispatchKey(def, 'keyup'));
        }

        window.dispatchEvent(new CustomEvent('virtual-gamepad-action', {
            detail: { action: actionName, isDown }
        }));
    }

    function buildGamepadHTML() {
        return `
            <!-- Floating Gamepad Toggle Button -->
            <button class="virtual-gamepad-toggle" id="vpadToggleBtn" title="Activar / Ocultar Mandos Táctiles">
                <span class="vpad-dot"></span>
                <span>🎮</span>
                <span id="vpadToggleLabel">PAD TÁCTIL</span>
            </button>

            <!-- Virtual Handheld HUD -->
            <div id="virtual-gamepad-hud">
                <!-- Top L / R Shoulder Bumpers -->
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
                    <div class="vpad-pill-cluster">
                        <button class="vpad-pill-btn" data-action="select" title="Select / Modo">
                            <span>MODE / SELECT</span>
                        </button>
                    </div>
                </div>

                <!-- Right Zone: Action Buttons (Sega / Nintendo) & Start -->
                <div class="vpad-right-cluster">
                    <div class="vpad-actions-container">
                        <!-- Top: Turbo / X -->
                        <button class="vpad-btn btn-x" data-action="x" title="Botón X / Turbo">
                            <span>X</span>
                            <span class="btn-sublabel">TURBO</span>
                        </button>
                        <!-- Left: Sega C / Action 3 -->
                        <button class="vpad-btn btn-c" data-action="c" title="Botón C / Especial">
                            <span>C</span>
                            <span class="btn-sublabel">NITRO</span>
                        </button>
                        <!-- Bottom: Sega A / Nintendo B -->
                        <button class="vpad-btn btn-b" data-action="b" title="Botón B (Frenar / Golpear)">
                            <span>B</span>
                            <span class="btn-sublabel">GOLPE</span>
                        </button>
                        <!-- Right: Sega B / Nintendo A -->
                        <button class="vpad-btn btn-a" data-action="a" title="Botón A (Acelerar / Saltar)">
                            <span>A</span>
                            <span class="btn-sublabel">SALTAR</span>
                        </button>
                    </div>
                    <div class="vpad-pill-cluster">
                        <button class="vpad-pill-btn" data-action="start" title="Start / Comenzar">
                            <span>START ▶</span>
                        </button>
                    </div>
                </div>
            </div>
        `;
    }

    function setupGamepadInteractions() {
        const hud = document.getElementById('virtual-gamepad-hud');
        const toggleBtn = document.getElementById('vpadToggleBtn');
        const toggleLabel = document.getElementById('vpadToggleLabel');

        if (!hud || !toggleBtn) return;

        const isMobile = isMobileDevice();
        const savedState = localStorage.getItem('virtual_gamepad_visible');

        // Always show by default on mobile or if user explicitly enabled it
        let isVisible = true;
        if (savedState !== null) {
            isVisible = (savedState === 'true');
        } else {
            isVisible = isMobile || (window.innerWidth <= 1200);
        }

        if (isVisible) {
            hud.classList.remove('hidden');
            hud.style.display = 'flex';
            toggleBtn.classList.remove('is-off');
            if (toggleLabel) toggleLabel.textContent = 'PAD: ON';
        } else {
            hud.classList.add('hidden');
            hud.style.display = 'none';
            toggleBtn.classList.add('is-off');
            if (toggleLabel) toggleLabel.textContent = 'PAD: OFF';
        }

        toggleBtn.addEventListener('click', (e) => {
            e.stopPropagation();
            const nowHidden = hud.classList.toggle('hidden');
            hud.style.display = nowHidden ? 'none' : 'flex';
            toggleBtn.classList.toggle('is-off', nowHidden);
            if (toggleLabel) toggleLabel.textContent = nowHidden ? 'PAD: OFF' : 'PAD: ON';
            localStorage.setItem('virtual_gamepad_visible', (!nowHidden).toString());
            hapticFeedback(20);
        });

        // Touch & Click binding on all buttons
        const actionElements = hud.querySelectorAll('[data-action]');
        actionElements.forEach(el => {
            const action = el.getAttribute('data-action');

            const startPress = (e) => {
                e.preventDefault();
                el.classList.add('active');
                dispatchAction(action, true);
            };

            const endPress = (e) => {
                e.preventDefault();
                el.classList.remove('active');
                dispatchAction(action, false);
            };

            el.addEventListener('touchstart', startPress, { passive: false });
            el.addEventListener('touchend', endPress, { passive: false });
            el.addEventListener('touchcancel', endPress, { passive: false });

            el.addEventListener('mousedown', startPress);
            el.addEventListener('mouseup', endPress);
            el.addEventListener('mouseleave', endPress);
        });

        // Smooth D-Pad Multi-Touch Drag
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

                if (distance < 14) {
                    ['up', 'down', 'left', 'right'].forEach(dir => {
                        const btn = dpadContainer.querySelector(`.vpad-dpad-btn.${dir}`);
                        if (btn) btn.classList.remove('active');
                        dispatchAction(dir, false);
                    });
                    return;
                }

                const angle = Math.atan2(dy, dx) * (180 / Math.PI);
                const isRight = (angle >= -67.5 && angle <= 67.5);
                const isLeft = (angle >= 112.5 || angle <= -112.5);
                const isDown = (angle >= 22.5 && angle <= 157.5);
                const isUp = (angle <= -22.5 && angle >= -157.5);

                const dirs = { up: isUp, down: isDown, left: isLeft, right: isRight };

                Object.entries(dirs).forEach(([dir, active]) => {
                    const btn = dpadContainer.querySelector(`.vpad-dpad-btn.${dir}`);
                    if (btn) btn.classList.toggle('active', active);
                    dispatchAction(dir, active);
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
                    dispatchAction(dir, false);
                });
                activeTouchId = null;
            };

            dpadContainer.addEventListener('touchend', releaseDpad, { passive: false });
            dpadContainer.addEventListener('touchcancel', releaseDpad, { passive: false });
        }
    }

    function checkDiabloMobileOnboarding() {
        if (!isDiabloPage() || !isMobileDevice()) return;
        if (sessionStorage.getItem('diablo_mobile_dismissed')) return;

        const modalHTML = `
            <div class="mobile-diablo-modal-backdrop" id="diabloMobileModal">
                <div class="mobile-diablo-modal">
                    <h2>📱 DISPOSITIVO MÓVIL DETECTADO</h2>
                    <p>
                        <strong>Diablo I & Hellfire</strong> fueron concebidos para ratón/teclado de precisión y requieren alta potencia en navegador.
                    </p>
                    <div class="mobile-diablo-badge-list">
                        <span class="mobile-diablo-badge">⭐ 100% 60 FPS Táctil</span>
                        <span class="mobile-diablo-badge">⚡ Sega Genesis (16-Bit)</span>
                        <span class="mobile-diablo-badge">🔴 NES (8-Bit)</span>
                        <span class="mobile-diablo-badge">🎮 SNES (16-Bit)</span>
                    </div>
                    <p style="font-size: 13px; opacity: 0.9;">
                        Para una experiencia fluida con mandos táctiles nativos a 60 FPS, te recomendamos jugar a nuestra selección retro:
                    </p>
                    <div class="mobile-diablo-actions">
                        <a href="${BASE_PATH}gens/index.html" class="mobile-diablo-btn-primary">
                            ⚡ Jugar Sega Genesis & Mega Drive (60 FPS)
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

    function initVirtualGamepad() {
        if (isMobileDevice()) {
            document.body.classList.add('is-mobile-device');
        }

        if (!document.getElementById('virtual-gamepad-css')) {
            const link = document.createElement('link');
            link.id = 'virtual-gamepad-css';
            link.rel = 'stylesheet';
            link.href = `${BASE_PATH}assets/mobile-controls/virtual-gamepad.css?v=gb-v2`;
            document.head.appendChild(link);
        }

        if (!document.getElementById('virtual-gamepad-hud')) {
            const vpadContainer = document.createElement('div');
            vpadContainer.id = 'virtual-gamepad-root';
            vpadContainer.innerHTML = buildGamepadHTML();
            document.body.appendChild(vpadContainer);
            setupGamepadInteractions();
        }

        checkDiabloMobileOnboarding();
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', initVirtualGamepad);
    } else {
        initVirtualGamepad();
    }
})();
