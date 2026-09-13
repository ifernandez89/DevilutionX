/**
 * Retro Hub Global Navigation & Quick Selector
 * Provides unified cross-game navigation across all WebAssembly retro experiences.
 */

(() => {
    'use strict';

    // Calculate relative path to root Packaging/emscripten/ directory
    function getRootRelativePath() {
        const path = window.location.pathname.toLowerCase();
        if (path.includes('/doom/') || path.includes('/wolf3d/') || path.includes('/gens/') || path.includes('/minixp/') || path.includes('/tinycore/') || path.includes('/quake/') || path.includes('/cavestory/') || path.includes('/nes/')) {
            return '../';
        }
        return './';
    }

    const BASE_PATH = getRootRelativePath();

    // Catalog definition organized by categories
    const RETRO_CATALOG = [
        {
            category: '💀 DOOM (Motor PrBoom WASM)',
            items: [
                {
                    id: 'doom-shareware',
                    title: 'DOOM 1 Shareware',
                    subtitle: 'Knee-Deep in the Dead (id Software 1993)',
                    badge: 'SHAREWARE',
                    badgeClass: 'badge-shareware',
                    icon: '🪖',
                    url: `${BASE_PATH}doom/index.html?game=doom1`,
                    activeMatcher: (p, q) => p.includes('/doom/') && (!q.get('game') || q.get('game') === 'doom1')
                },
                {
                    id: 'doom-freedoom1',
                    title: 'Freedoom: Phase 1',
                    subtitle: 'Campaña libre completa • 30 Niveles',
                    badge: 'OPEN SOURCE',
                    badgeClass: 'badge-open-source',
                    icon: '💀',
                    url: `${BASE_PATH}doom/index.html?game=freedoom1`,
                    activeMatcher: (p, q) => p.includes('/doom/') && q.get('game') === 'freedoom1'
                },
                {
                    id: 'doom-freedoom2',
                    title: 'Freedoom: Phase 2',
                    subtitle: 'Super Escopeta • 32 Niveles completos',
                    badge: 'OPEN SOURCE',
                    badgeClass: 'badge-open-source',
                    icon: '🔥',
                    url: `${BASE_PATH}doom/index.html?game=freedoom2`,
                    activeMatcher: (p, q) => p.includes('/doom/') && q.get('game') === 'freedoom2'
                }
            ]
        },
        {
            category: '🌋 3D FPS REVOLUTION (Motor TyrQuake WASM)',
            items: [
                {
                    id: 'quake1',
                    title: 'Quake 1: Episode 1',
                    subtitle: 'Dimension of the Doomed (id Software 1996)',
                    badge: 'SHAREWARE',
                    badgeClass: 'badge-shareware',
                    icon: '🌋',
                    url: `${BASE_PATH}quake/index.html`,
                    activeMatcher: (p) => p.includes('/quake/')
                }
            ]
        },
        {
            category: '🐺 FPS CLASSICS (Motor ECWolf WASM)',
            items: [
                {
                    id: 'wolf3d',
                    title: 'Wolfenstein 3D: Episodio 1',
                    subtitle: 'Escape from Castle Wolfenstein (1992)',
                    badge: 'SHAREWARE',
                    badgeClass: 'badge-shareware',
                    icon: '🐺',
                    url: `${BASE_PATH}wolf3d/index.html`,
                    activeMatcher: (p) => p.includes('/wolf3d/')
                }
            ]
        },
        {
            category: '🐰 INDIE ADVENTURE (Motor NXEngine WASM)',
            items: [
                {
                    id: 'cavestory',
                    title: 'Cave Story (Doukutsu)',
                    subtitle: 'Pixel Amaya • Aventura Completa en Inglés',
                    badge: 'FREEWARE',
                    badgeClass: 'badge-open-source',
                    icon: '🐰',
                    url: `${BASE_PATH}cavestory/index.html`,
                    activeMatcher: (p) => p.includes('/cavestory/')
                }
            ]
        },
        {
            category: '🎮 SEGA 16-BIT (Motor Genesis Plus GX WASM)',
            items: [
                {
                    id: 'gens',
                    title: 'Sega Genesis & Mega Drive',
                    subtitle: 'Lector Universal de ROMs 16-bit • 60 FPS WASM',
                    badge: '60 FPS WASM',
                    badgeClass: 'badge-wasm',
                    icon: '🎮',
                    url: `${BASE_PATH}gens/index.html`,
                    activeMatcher: (p) => p.includes('/gens/')
                }
            ]
        },
        {
            category: '🔴 NINTENDO 8-BIT (Motor Nestopia WASM)',
            items: [
                {
                    id: 'nes',
                    title: 'Nintendo NES & Famicom',
                    subtitle: 'Lector Universal de ROMs .NES • Ciclo Exacto 60 FPS',
                    badge: '8-BIT WASM',
                    badgeClass: 'badge-wasm',
                    icon: '🔴',
                    url: `${BASE_PATH}nes/index.html`,
                    activeMatcher: (p) => p.includes('/nes/')
                }
            ]
        },
        {
            category: '⚔️ ACTION RPG (Motor DevilutionX WASM)',
            items: [
                {
                    id: 'diablo1',
                    title: 'Diablo 1: Tristram',
                    subtitle: 'DevilutionX Source Port (1996 Blizzard)',
                    badge: 'NATIVE C++',
                    badgeClass: 'badge-wasm',
                    icon: '⚔️',
                    url: `${BASE_PATH}index.html`,
                    activeMatcher: (p) => !p.includes('/doom/') && !p.includes('/wolf3d/') && !p.includes('/gens/') && !p.includes('/minixp/') && !p.includes('/quake/') && !p.includes('/cavestory/') && !p.includes('/nes/')
                }
            ]
        },
        {
            category: '🐧 VIRTUAL COMPUTING (Motor v86 VM)',
            items: [
                {
                    id: 'tinycore',
                    title: 'Tiny Core Linux 15.0 Desktop',
                    subtitle: 'Máquina Virtual x86 en Navegador',
                    badge: 'VIRTUAL PC',
                    badgeClass: 'badge-open-source',
                    icon: '🐧',
                    url: `${BASE_PATH}minixp/index.html`,
                    activeMatcher: (p) => p.includes('/minixp/') || p.includes('/tinycore/')
                }
            ]
        }
    ];

    function createRetroNavHTML() {
        const currentPath = window.location.pathname.toLowerCase();
        const currentQuery = new URLSearchParams(window.location.search);

        let categoriesHTML = '';
        for (const cat of RETRO_CATALOG) {
            let itemsHTML = '';
            for (const item of cat.items) {
                const isCurrent = item.activeMatcher ? item.activeMatcher(currentPath, currentQuery) : false;
                itemsHTML += `
                    <a href="${item.url}" class="retro-nav-item ${isCurrent ? 'current-game' : ''}" title="${item.title}">
                        <div class="retro-nav-icon">${item.icon}</div>
                        <div class="retro-nav-info">
                            <div class="retro-nav-title">${item.title}</div>
                            <div class="retro-nav-subtitle">${item.subtitle}</div>
                        </div>
                        <span class="retro-nav-badge ${isCurrent ? 'badge-active' : item.badgeClass}">${isCurrent ? 'JUGANDO' : item.badge}</span>
                    </a>
                `;
            }

            categoriesHTML += `
                <div class="retro-nav-category">
                    <div class="retro-nav-category-header">${cat.category}</div>
                    ${itemsHTML}
                </div>
            `;
        }

        return `
            <div class="retro-nav-wrapper" id="retroNavWrapper">
                <button class="retro-nav-trigger" id="retroNavTrigger" type="button" aria-haspopup="true" aria-expanded="false" title="Catálogo de Juegos Nativos Retro">
                    <span>🎮 RETRO HUB</span>
                    <span class="nav-arrow">▼</span>
                </button>
                <div class="retro-nav-dropdown" id="retroNavDropdown" role="menu">
                    ${categoriesHTML}
                    <div class="retro-nav-footer">
                        🧪 Próxima expansión: Heretic • Hexen • Catacomb • OpenTyrian
                    </div>
                </div>
            </div>
        `;
    }

    function initRetroNav() {
        // Ensure stylesheet is loaded
        if (!document.getElementById('retro-nav-css')) {
            const link = document.createElement('link');
            link.id = 'retro-nav-css';
            link.rel = 'stylesheet';
            link.href = `${BASE_PATH}assets/retro-nav/retro-nav.css`;
            document.head.appendChild(link);
        }

        // Find injection container (preferred: element with id="retro-nav-slot", or header-actions)
        let slot = document.getElementById('retro-nav-slot');
        if (!slot) {
            const headerActions = document.querySelector('.header-actions') || document.querySelector('header');
            if (headerActions) {
                slot = document.createElement('div');
                slot.id = 'retro-nav-slot';
                headerActions.insertBefore(slot, headerActions.firstChild);
            }
        }

        if (!slot) return;
        slot.innerHTML = createRetroNavHTML();

        const wrapper = document.getElementById('retroNavWrapper');
        const trigger = document.getElementById('retroNavTrigger');

        if (!wrapper || !trigger) return;

        trigger.addEventListener('click', (e) => {
            e.stopPropagation();
            const isActive = wrapper.classList.toggle('active');
            trigger.setAttribute('aria-expanded', isActive ? 'true' : 'false');
        });

        // Close when clicking outside
        document.addEventListener('click', (e) => {
            if (!wrapper.contains(e.target)) {
                wrapper.classList.remove('active');
                trigger.setAttribute('aria-expanded', 'false');
            }
        });

        // Close on Escape key
        window.addEventListener('keydown', (e) => {
            if (e.key === 'Escape' && wrapper.classList.contains('active')) {
                wrapper.classList.remove('active');
                trigger.setAttribute('aria-expanded', 'false');
                trigger.focus();
            }
        });
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', initRetroNav);
    } else {
        initRetroNav();
    }
})();
