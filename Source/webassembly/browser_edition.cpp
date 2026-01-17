/**
 * @file browser_edition.cpp
 * @brief Implementation of Browser Edition functionality
 */

#include "browser_edition.h"

#include <random>
#include <ctime>

#include "utils/log.hpp"
#include "diablo.h"
#include "player.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace devilution {

namespace {
    uint32_t g_browserEditionSeed = 0;
    bool g_browserEditionInitialized = false;
}

void InitializeBrowserEdition()
{
    if (g_browserEditionInitialized) {
        return;
    }
    
    LogInfo("🌐 Initializing Nightmare Browser Edition");
    LogInfo("   💀 Permadeath Mode: ENABLED");
    LogInfo("   🚫 Save/Load: DISABLED");
    LogInfo("   🎲 Seed-based worlds: ENABLED");
    LogInfo("   ⚡ Instant restart: ENABLED");
    
    // Generate initial seed if not set
    if (g_browserEditionSeed == 0) {
        std::random_device rd;
        g_browserEditionSeed = rd();
        LogInfo("   🎲 Generated random seed: {}", g_browserEditionSeed);
    }
    
    g_browserEditionInitialized = true;
    
#ifdef __EMSCRIPTEN__
    // Set up browser-specific configurations
    LogInfo("   🌐 WebAssembly runtime detected");
    
    // Disable browser context menu on right-click for better game experience
    EM_ASM({
        document.addEventListener('contextmenu', function(e) {
            e.preventDefault();
        });
    });
#endif
}

void HandleBrowserEditionDeath()
{
    if (!IsBrowserEdition()) {
        return; // Not Browser Edition, use normal death handling
    }
    
    LogInfo("💀 BROWSER EDITION DEATH DETECTED");
    LogInfo("   🎲 Current seed was: {}", g_browserEditionSeed);
    LogInfo("   ⚡ Triggering permadeath restart...");
    
#ifdef __EMSCRIPTEN__
    // Show death message in browser console
    EM_ASM({
        console.log("💀 NIGHTMARE BROWSER EDITION - PERMADEATH!");
        console.log("🔄 Your journey ends here. Restarting with new seed...");
        
        // Optional: Show browser notification if supported
        if ('Notification' in window && Notification.permission === 'granted') {
            new Notification('💀 Nightmare Browser Edition', {
                body: 'Permadeath! Restarting with new seed...',
                icon: 'nightmare.ico'
            });
        }
    });
#endif
    
    // Generate new seed for next run
    std::random_device rd;
    uint32_t newSeed = rd();
    
    LogInfo("   🎲 New seed for restart: {}", newSeed);
    
    // Delay restart slightly to allow death animation to complete
    // In a real implementation, this would be handled by a timer or frame counter
    RestartBrowserEdition(newSeed);
}

uint32_t GetBrowserEditionSeed()
{
    return g_browserEditionSeed;
}

void SetBrowserEditionSeed(uint32_t seed)
{
    g_browserEditionSeed = seed;
    LogInfo("🎲 Browser Edition seed set to: {}", seed);
}

void RestartBrowserEdition(uint32_t newSeed)
{
    if (!IsBrowserEdition()) {
        LogWarn("RestartBrowserEdition called but not in Browser Edition mode");
        return;
    }
    
    if (newSeed == 0) {
        std::random_device rd;
        newSeed = rd();
    }
    
    LogInfo("🔄 BROWSER EDITION RESTART INITIATED");
    LogInfo("   🎲 New seed: {}", newSeed);
    LogInfo("   💀 Permadeath mode: Session reset");
    
    // Set new seed
    SetBrowserEditionSeed(newSeed);
    
#ifdef __EMSCRIPTEN__
    // Show restart message in browser
    EM_ASM({
        console.log("🔄 NIGHTMARE BROWSER EDITION - RESTARTING");
        console.log("🎲 New seed: " + $0, newSeed);
        console.log("⚡ Fresh start loading...");
        
        // Optional: Update page title to show restart
        document.title = "💀 Nightmare Browser Edition - Restarting...";
        
        // Optional: Show loading overlay
        var canvas = document.getElementById('canvas');
        if (canvas) {
            var ctx = canvas.getContext('2d');
            ctx.fillStyle = 'black';
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            ctx.fillStyle = 'red';
            ctx.font = '24px monospace';
            ctx.textAlign = 'center';
            ctx.fillText('💀 PERMADEATH', canvas.width/2, canvas.height/2 - 20);
            ctx.fillStyle = 'yellow';
            ctx.font = '16px monospace';
            ctx.fillText('🔄 Restarting with new seed...', canvas.width/2, canvas.height/2 + 20);
        }
    }, newSeed);
    
    // Delay the actual restart to show the message
    EM_ASM({
        setTimeout(function() {
            console.log("🌐 Reloading page for fresh start...");
            window.location.reload();
        }, 2000); // 2 second delay
    });
#else
    // For desktop builds (testing), just log the restart
    LogInfo("   🖥️ Desktop build: Restart would reload game state");
    LogInfo("   ⚡ In WebAssembly, this would reload the page");
#endif
}

const char* GetBrowserEditionName()
{
    return "Nightmare - Browser Edition";
}

const char* GetBrowserEditionVersion()
{
    return "1.0.0-alpha (WebAssembly)";
}

} // namespace devilution