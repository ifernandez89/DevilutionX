/**
 * RetroHub Service Worker (PWA Offline Shell & Asset Cache)
 * Enables PWA installability on mobile/desktop and ultra-fast UI caching.
 */

const CACHE_NAME = 'retrohub-v5-cache';

const CORE_ASSETS = [
    './manifest.json',
    './assets/icons/icon-192.png',
    './assets/icons/icon-512.png',
    './assets/icons/icon-maskable-192.png',
    './assets/icons/icon-maskable-512.png',
    './assets/icons/apple-touch-icon.png',
    './assets/icons/favicon.png',
    './assets/icons/retrohub.svg',
    './assets/retro-nav/retro-nav.css?v=retro-hub-v5',
    './assets/retro-nav/retro-nav.js?v=retro-hub-v5',
    './assets/mobile-controls/virtual-gamepad.css?v=gb-v5',
    './assets/mobile-controls/virtual-gamepad.js?v=gb-v5'
];

self.addEventListener('install', (event) => {
    self.skipWaiting();
    event.waitUntil(
        caches.open(CACHE_NAME).then((cache) => {
            return cache.addAll(CORE_ASSETS).catch((err) => {
                console.warn('[Service Worker] Non-fatal asset precache error:', err);
            });
        })
    );
});

self.addEventListener('activate', (event) => {
    event.waitUntil(
        caches.keys().then((cacheNames) => {
            return Promise.all(
                cacheNames.map((name) => {
                    if (name !== CACHE_NAME) {
                        return caches.delete(name);
                    }
                })
            );
        }).then(() => self.clients.claim())
    );
});

self.addEventListener('fetch', (event) => {
    const request = event.request;
    if (request.method !== 'GET') return;

    const url = new URL(request.url);

    // Bypass cross-origin or non-http requests
    if (!url.protocol.startsWith('http')) return;

    // Core Shell UI Assets: Stale-While-Revalidate
    if (url.pathname.includes('/assets/icons/') || 
        url.pathname.includes('/retro-nav.') || 
        url.pathname.includes('/virtual-gamepad.') ||
        url.pathname.endsWith('manifest.json')) {
        event.respondWith(
            caches.open(CACHE_NAME).then((cache) => {
                return cache.match(request).then((cachedResponse) => {
                    const fetchPromise = fetch(request).then((networkResponse) => {
                        if (networkResponse && networkResponse.status === 200) {
                            cache.put(request, networkResponse.clone());
                        }
                        return networkResponse;
                    }).catch(() => cachedResponse);
                    return cachedResponse || fetchPromise;
                });
            })
        );
        return;
    }

    // Default: Network with Cache Fallback
    event.respondWith(
        fetch(request).catch(() => {
            return caches.match(request);
        })
    );
});
