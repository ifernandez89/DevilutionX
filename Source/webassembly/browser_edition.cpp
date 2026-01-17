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
    
    LogInfo("💀 BROWSER EDITION DEATH - Triggering instant restart");
    LogInfo("   🎲 Current seed was: {}", g_browserEditionSeed);
    
#ifdef __EMSCRIPTEN__
    // Show death message in browser console
    EM_ASM({
        console.log("💀 NIGHTMARE BROWSER EDITION - PERMADEATH!");
        console.log("🔄 Restarting with new seed...");
    });
#endif
    
    // Generate new seed for next run
    std::random_device rd;
    uint32_t newSeed = rd();
    
    LogInfo("   🎲 New seed for restart: {}", newSeed);
    
    // Trigger complete restart
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
    
    LogInfo("🔄 BROWSER EDITION RESTART");
    LogInfo("   🎲 New seed: {}", newSeed);
    
    // Set new seed
    SetBrowserEditionSeed(newSeed);
    
    // TODO: Implement actual restart logic
    // This will need to:
    // 1. Reset all game state
    // 2. Regenerate world with new seed
    // 3. Reset player to level 1
    // 4. Clear all progress
    // 5. Return to character creation or main menu
    
    LogInfo("   ⚡ Restart logic will be implemented in next phase");
    
#ifdef __EMSCRIPTEN__
    // For now, just reload the page in browser
    EM_ASM({
        console.log("🔄 Reloading page for fresh start...");
        window.location.reload();
    });
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