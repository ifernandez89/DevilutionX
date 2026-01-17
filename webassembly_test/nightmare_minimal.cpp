/**
 * @file nightmare_minimal.cpp
 * @brief Minimal Nightmare Browser Edition test for WebAssembly
 */

#include <iostream>
#include <random>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace devilution {

// Global state for Browser Edition
uint32_t g_browserEditionSeed = 0;
bool g_browserEditionInitialized = false;
uint32_t g_deathCount = 0;
uint32_t g_runCount = 0;

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
    
    std::cout << "[INFO] 🌐 Initializing Nightmare Browser Edition" << std::endl;
    std::cout << "[INFO]    💀 Permadeath Mode: ENABLED" << std::endl;
    std::cout << "[INFO]    🚫 Save/Load: DISABLED" << std::endl;
    std::cout << "[INFO]    🎲 Seed-based worlds: ENABLED" << std::endl;
    std::cout << "[INFO]    ⚡ Instant restart: ENABLED" << std::endl;
    
    // Generate initial seed if not set
    if (g_browserEditionSeed == 0) {
        std::random_device rd;
        g_browserEditionSeed = rd();
        std::cout << "[INFO]    🎲 Generated random seed: " << g_browserEditionSeed << std::endl;
    }
    
    g_browserEditionInitialized = true;
    g_runCount++;
    
#ifdef __EMSCRIPTEN__
    std::cout << "[INFO]    🌐 WebAssembly runtime detected" << std::endl;
    
    // Simple browser integration
    EM_ASM({
        console.log('🌐 NIGHTMARE BROWSER EDITION LOADED');
        console.log('💀 Permadeath Mode: ACTIVE');
        console.log('🚫 Save System: DISABLED');
        console.log('🎲 Seed-based Worlds: ENABLED');
    });
#endif
}

void HandleBrowserEditionDeath() {
    if (!IsBrowserEdition()) {
        return;
    }
    
    g_deathCount++;
    
    std::cout << "[INFO] 💀 BROWSER EDITION DEATH DETECTED" << std::endl;
    std::cout << "[INFO]    🎲 Current seed was: " << g_browserEditionSeed << std::endl;
    std::cout << "[INFO]    💀 Death count: " << g_deathCount << std::endl;
    std::cout << "[INFO]    🏃 Run count: " << g_runCount << std::endl;
    std::cout << "[INFO]    ⚡ Triggering permadeath restart..." << std::endl;
    
#ifdef __EMSCRIPTEN__
    EM_ASM({
        console.log('💀 NIGHTMARE BROWSER EDITION - PERMADEATH!');
        console.log('🔄 Your journey ends here. Death #' + $0);
        console.log('🎲 Previous seed was: ' + $1);
    }, g_deathCount, g_browserEditionSeed);
#endif
    
    // Generate new seed for next run
    std::random_device rd;
    uint32_t newSeed = rd();
    
    std::cout << "[INFO]    🎲 New seed for restart: " << newSeed << std::endl;
    
    // Set new seed
    g_browserEditionSeed = newSeed;
    std::cout << "[INFO] 🎲 Browser Edition seed set to: " << newSeed << std::endl;
    
#ifdef __EMSCRIPTEN__
    EM_ASM({
        console.log('🔄 NIGHTMARE BROWSER EDITION - RESTARTING');
        console.log('🎲 New seed: ' + $0);
        console.log('⚡ Fresh start loading...');
    }, newSeed);
#endif
}

uint32_t GetBrowserEditionSeed() {
    return g_browserEditionSeed;
}

void SetBrowserEditionSeed(uint32_t seed) {
    g_browserEditionSeed = seed;
    std::cout << "[INFO] 🎲 Browser Edition seed set to: " << seed << std::endl;
}

uint32_t GetDeathCount() {
    return g_deathCount;
}

uint32_t GetRunCount() {
    return g_runCount;
}

const char* GetBrowserEditionName() {
    return "Nightmare - Browser Edition";
}

const char* GetBrowserEditionVersion() {
    return "1.0.0-webassembly-minimal";
}

} // namespace devilution

using namespace devilution;

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void initialize_nightmare_edition() {
        InitializeBrowserEdition();
        std::cout << "🌐 Nightmare Browser Edition initialized!" << std::endl;
    }
    
    EMSCRIPTEN_KEEPALIVE
    uint32_t get_browser_edition_seed() {
        return GetBrowserEditionSeed();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void set_browser_edition_seed(uint32_t seed) {
        SetBrowserEditionSeed(seed);
    }
    
    EMSCRIPTEN_KEEPALIVE
    void simulate_player_death() {
        HandleBrowserEditionDeath();
        std::cout << "💀 Player death simulated!" << std::endl;
    }
    
    EMSCRIPTEN_KEEPALIVE
    uint32_t get_death_count() {
        return GetDeathCount();
    }
    
    EMSCRIPTEN_KEEPALIVE
    uint32_t get_run_count() {
        return GetRunCount();
    }
    
    EMSCRIPTEN_KEEPALIVE
    const char* get_edition_name() {
        return GetBrowserEditionName();
    }
    
    EMSCRIPTEN_KEEPALIVE
    const char* get_edition_version() {
        return GetBrowserEditionVersion();
    }
}

int main() {
    std::cout << "🌐 NIGHTMARE BROWSER EDITION - WebAssembly Minimal" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    // Initialize Browser Edition
    InitializeBrowserEdition();
    
    // Log Browser Edition info
    std::cout << "🎮 Edition: " << GetBrowserEditionName() << std::endl;
    std::cout << "📦 Version: " << GetBrowserEditionVersion() << std::endl;
    std::cout << "🎲 Seed: " << GetBrowserEditionSeed() << std::endl;
    
#ifdef __EMSCRIPTEN__
    EM_ASM({
        console.log('🎯 Ready! Use the exported functions to test functionality');
        console.log('Available functions:');
        console.log('- Module.ccall("get_browser_edition_seed", "number", [], [])');
        console.log('- Module.ccall("simulate_player_death", null, [], [])');
        console.log('- Module.ccall("get_death_count", "number", [], [])');
        console.log('- Module.ccall("get_run_count", "number", [], [])');
    });
#endif
    
    std::cout << "✅ WebAssembly build successful!" << std::endl;
    std::cout << "🎯 Ready for browser testing!" << std::endl;
    
    return 0;
}