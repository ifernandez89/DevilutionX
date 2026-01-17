/**
 * @file main_webassembly_test.cpp
 * @brief Simplified main for WebAssembly testing
 */

#include <iostream>
#include <emscripten.h>

// Mock the necessary includes for a minimal build
#ifdef __EMSCRIPTEN__
#include "Source/webassembly/browser_edition.h"
#endif

// Mock functions for testing
namespace devilution {
    void LogInfo(const char* format, ...) {
        std::cout << "[INFO] " << format << std::endl;
    }
    
    void LogVerbose(const char* format, ...) {
        std::cout << "[VERBOSE] " << format << std::endl;
    }
    
    void LogWarn(const char* format, ...) {
        std::cout << "[WARN] " << format << std::endl;
    }
    
    // Mock VFS initialization
    void InitializeVFSForPlatform() {
        LogInfo("🌐 VFS initialized for WebAssembly");
    }
}

using namespace devilution;

extern "C" {
    // Export functions to JavaScript
    EMSCRIPTEN_KEEPALIVE
    void initialize_nightmare_edition() {
#ifdef __EMSCRIPTEN__
        InitializeBrowserEdition();
#endif
        std::cout << "🌐 Nightmare Browser Edition initialized!" << std::endl;
    }
    
    EMSCRIPTEN_KEEPALIVE
    int get_browser_edition_seed() {
#ifdef __EMSCRIPTEN__
        return GetBrowserEditionSeed();
#else
        return 12345; // Mock seed for testing
#endif
    }
    
    EMSCRIPTEN_KEEPALIVE
    void simulate_player_death() {
#ifdef __EMSCRIPTEN__
        HandleBrowserEditionDeath();
#endif
        std::cout << "💀 Player death simulated!" << std::endl;
    }
}

int main() {
    std::cout << "🌐 NIGHTMARE BROWSER EDITION - WebAssembly Build" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    // Initialize VFS
    InitializeVFSForPlatform();
    
    // Initialize Browser Edition
#ifdef __EMSCRIPTEN__
    InitializeBrowserEdition();
    
    // Log Browser Edition info
    std::cout << "🎮 Edition: " << GetBrowserEditionName() << std::endl;
    std::cout << "📦 Version: " << GetBrowserEditionVersion() << std::endl;
    std::cout << "🎲 Seed: " << GetBrowserEditionSeed() << std::endl;
#endif
    
    // Set up browser integration
    EM_ASM({
        console.log('🌐 NIGHTMARE BROWSER EDITION LOADED');
        console.log('💀 Permadeath Mode: ACTIVE');
        console.log('🚫 Save System: DISABLED');
        console.log('🎲 Seed-based Worlds: ENABLED');
        
        // Add to global scope for testing
        window.NightmareBrowserEdition = {
            initialized: true,
            version: '1.0.0-webassembly-test',
            features: {
                permadeath: true,
                seedBased: true,
                noSaves: true,
                instantRestart: true
            }
        };
        
        // Test functions
        window.testNightmareEdition = function() {
            console.log('🧪 Testing Nightmare Browser Edition...');
            Module.ccall('initialize_nightmare_edition', null, [], []);
            var seed = Module.ccall('get_browser_edition_seed', 'number', [], []);
            console.log('🎲 Current seed:', seed);
            Module.ccall('simulate_player_death', null, [], []);
        };
        
        console.log('🎯 Ready! Call testNightmareEdition() to test functionality');
    });
    
    std::cout << "✅ WebAssembly build successful!" << std::endl;
    std::cout << "🎯 Ready for browser testing!" << std::endl;
    
    return 0;
}