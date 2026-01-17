/**
 * @file browser_edition_simple.cpp
 * @brief Simplified Browser Edition for WebAssembly testing
 */

#include <random>
#include <ctime>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace devilution {

// Mock logging functions
void LogInfo(const char* format, ...) {
    std::cout << "[INFO] " << format << std::endl;
}

void LogVerbose(const char* format, ...) {
    std::cout << "[VERBOSE] " << format << std::endl;
}

void LogWarn(const char* format, ...) {
    std::cout << "[WARN] " << format << std::endl;
}

namespace {
    uint32_t g_browserEditionSeed = 0;
    bool g_browserEditionInitialized = false;
}

bool IsBrowserEdition() {
#ifdef __EMSCRIPTEN__
    return true;
#else
    return false;
#endif
}

void InitializeBrowserEdition() {
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
        LogInfo("   🎲 Generated random seed: %u", g_browserEditionSeed);
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

void HandleBrowserEditionDeath() {
    if (!IsBrowserEdition()) {
        return; // Not Browser Edition, use normal death handling
    }
    
    LogInfo("💀 BROWSER EDITION DEATH DETECTED");
    LogInfo("   🎲 Current seed was: %u", g_browserEditionSeed);
    LogInfo("   ⚡ Triggering permadeath restart...");
    
#ifdef __EMSCRIPTEN__
    // Show death message in browser console
    EM_ASM({
        console.log("💀 NIGHTMARE BROWSER EDITION - PERMADEATH!");
        console.log("🔄 Your journey ends here. Restarting with new seed...");
    });
#endif
    
    // Generate new seed for next run
    std::random_device rd;
    uint32_t newSeed = rd();
    
    LogInfo("   🎲 New seed for restart: %u", newSeed);
    
    // Set new seed
    SetBrowserEditionSeed(newSeed);
    
#ifdef __EMSCRIPTEN__
    // Show restart message
    EM_ASM({
        console.log("🔄 NIGHTMARE BROWSER EDITION - RESTARTING");
        console.log("⚡ Fresh start loading...");
        
        // In a real implementation, this would restart the game
        setTimeout(function() {
            console.log("🌐 Restart complete (simulated)");
        }, 2000);
    });
#endif
}

uint32_t GetBrowserEditionSeed() {
    return g_browserEditionSeed;
}

void SetBrowserEditionSeed(uint32_t seed) {
    g_browserEditionSeed = seed;
    LogInfo("🎲 Browser Edition seed set to: %u", seed);
}

const char* GetBrowserEditionName() {
    return "Nightmare - Browser Edition";
}

const char* GetBrowserEditionVersion() {
    return "1.0.0-webassembly-test";
}

} // namespace devilution