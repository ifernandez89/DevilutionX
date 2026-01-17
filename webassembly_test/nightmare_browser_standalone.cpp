/**
 * @file nightmare_browser_standalone.cpp
 * @brief Standalone Nightmare Browser Edition test for WebAssembly
 */

#include <iostream>
#include <random>
#include <ctime>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace devilution {

// Global state for Browser Edition
namespace {
    uint32_t g_browserEditionSeed = 0;
    bool g_browserEditionInitialized = false;
    uint32_t g_deathCount = 0;
    uint32_t g_runCount = 0;
}

// Mock logging functions
void LogInfo(const char* format, uint32_t value = 0) {
    std::cout << "[INFO] " << format;
    if (value != 0) std::cout << " " << value;
    std::cout << std::endl;
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
        LogInfo("   🎲 Generated random seed:", g_browserEditionSeed);
    }
    
    g_browserEditionInitialized = true;
    g_runCount++;
    
#ifdef __EMSCRIPTEN__
    // Set up browser-specific configurations
    LogInfo("   🌐 WebAssembly runtime detected");
    
    // Disable browser context menu on right-click for better game experience
    EM_ASM({
        document.addEventListener('contextmenu', function(e) {
            e.preventDefault();
        });
        
        // Update page title
        document.title = '🌐 Nightmare Browser Edition - Run #' + $0;
    }, g_runCount);
#endif
}

void HandleBrowserEditionDeath() {
    if (!IsBrowserEdition()) {
        return; // Not Browser Edition, use normal death handling
    }
    
    g_deathCount++;
    
    LogInfo("💀 BROWSER EDITION DEATH DETECTED");
    LogInfo("   🎲 Current seed was:", g_browserEditionSeed);
    LogInfo("   💀 Death count:", g_deathCount);
    LogInfo("   🏃 Run count:", g_runCount);
    LogInfo("   ⚡ Triggering permadeath restart...");
    
#ifdef __EMSCRIPTEN__
    // Show death message in browser console
    EM_ASM({
        console.log("💀 NIGHTMARE BROWSER EDITION - PERMADEATH!");
        console.log("🔄 Your journey ends here. Death #" + $0);
        console.log("🎲 Previous seed was: " + $1);
        
        // Update page title with death count
        document.title = '💀 Nightmare Browser Edition - Deaths: ' + $0;
        
        // Show death overlay on canvas if it exists
        var canvas = document.getElementById('canvas');
        if (canvas) {
            var ctx = canvas.getContext('2d');
            
            // Semi-transparent red overlay
            ctx.fillStyle = 'rgba(255, 0, 0, 0.8)';
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            
            // Death message
            ctx.fillStyle = 'white';
            ctx.font = 'bold 24px monospace';
            ctx.textAlign = 'center';
            ctx.fillText('💀 PERMADEATH', canvas.width/2, canvas.height/2 - 30);
            
            ctx.font = '16px monospace';
            ctx.fillText('Death #' + $0 + ' - Restarting...', canvas.width/2, canvas.height/2 + 10);
            
            ctx.font = '12px monospace';
            ctx.fillStyle = 'yellow';
            ctx.fillText('Previous seed: ' + $1, canvas.width/2, canvas.height/2 + 40);
        }
    }, g_deathCount, g_browserEditionSeed);
#endif
    
    // Generate new seed for next run
    std::random_device rd;
    uint32_t newSeed = rd();
    
    LogInfo("   🎲 New seed for restart:", newSeed);
    
    // Set new seed
    SetBrowserEditionSeed(newSeed);
    
#ifdef __EMSCRIPTEN__
    // Show restart message and simulate restart
    EM_ASM({
        console.log("🔄 NIGHTMARE BROWSER EDITION - RESTARTING");
        console.log("🎲 New seed: " + $0);
        console.log("⚡ Fresh start in 3 seconds...");
        
        setTimeout(function() {
            console.log("🌐 Restart complete (simulated)");
            console.log("🎮 New run started with seed: " + $0);
            
            // Clear death overlay
            var canvas = document.getElementById('canvas');
            if (canvas) {
                var ctx = canvas.getContext('2d');
                ctx.fillStyle = 'black';
                ctx.fillRect(0, 0, canvas.width, canvas.height);
                
                ctx.fillStyle = 'green';
                ctx.font = '20px monospace';
                ctx.textAlign = 'center';
                ctx.fillText('🎮 NEW RUN STARTED', canvas.width/2, canvas.height/2 - 10);
                ctx.font = '14px monospace';
                ctx.fillText('Seed: ' + $0, canvas.width/2, canvas.height/2 + 20);
            }
            
            // Update title for new run
            document.title = '🌐 Nightmare Browser Edition - Playing (Seed: ' + $0 + ')';
        }, 3000);
    }, newSeed);
#endif
}

uint32_t GetBrowserEditionSeed() {
    return g_browserEditionSeed;
}

void SetBrowserEditionSeed(uint32_t seed) {
    g_browserEditionSeed = seed;
    LogInfo("🎲 Browser Edition seed set to:", seed);
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
    return "1.0.0-webassembly-standalone";
}

} // namespace devilution

using namespace devilution;

extern "C" {
    // Export functions to JavaScript
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
    std::cout << "🌐 NIGHTMARE BROWSER EDITION - WebAssembly Standalone" << std::endl;
    std::cout << "=====================================================" << std::endl;
    
    // Initialize Browser Edition
    InitializeBrowserEdition();
    
    // Log Browser Edition info
    std::cout << "🎮 Edition: " << GetBrowserEditionName() << std::endl;
    std::cout << "📦 Version: " << GetBrowserEditionVersion() << std::endl;
    std::cout << "🎲 Seed: " << GetBrowserEditionSeed() << std::endl;
    
    // Set up browser integration
#ifdef __EMSCRIPTEN__
    EM_ASM({
        console.log('🌐 NIGHTMARE BROWSER EDITION LOADED');
        console.log('💀 Permadeath Mode: ACTIVE');
        console.log('🚫 Save System: DISABLED');
        console.log('🎲 Seed-based Worlds: ENABLED');
        
        // Add to global scope for testing
        window.NightmareBrowserEdition = {
            initialized: true,
            version: Module.ccall('get_edition_version', 'string', [], []),
            features: {
                permadeath: true,
                seedBased: true,
                noSaves: true,
                instantRestart: true
            },
            
            // API functions
            getSeed: function() {
                return Module.ccall('get_browser_edition_seed', 'number', [], []);
            },
            
            setSeed: function(seed) {
                Module.ccall('set_browser_edition_seed', null, ['number'], [seed]);
            },
            
            simulateDeath: function() {
                Module.ccall('simulate_player_death', null, [], []);
            },
            
            getStats: function() {
                return {
                    deaths: Module.ccall('get_death_count', 'number', [], []),
                    runs: Module.ccall('get_run_count', 'number', [], []),
                    currentSeed: this.getSeed()
                };
            },
            
            // Test function
            runTest: function() {
                console.log('🧪 Testing Nightmare Browser Edition...');
                console.log('📊 Current stats:', this.getStats());
                console.log('🎲 Current seed:', this.getSeed());
                
                console.log('💀 Simulating player death...');
                this.simulateDeath();
                
                setTimeout(() => {
                    console.log('📊 Stats after death:', this.getStats());
                    console.log('🎲 New seed:', this.getSeed());
                }, 1000);
            }
        };
        
        // Create a simple canvas for visual feedback
        if (!document.getElementById('canvas')) {
            var canvas = document.createElement('canvas');
            canvas.id = 'canvas';
            canvas.width = 640;
            canvas.height = 480;
            canvas.style.border = '2px solid #ff6b6b';
            canvas.style.backgroundColor = 'black';
            document.body.appendChild(canvas);
            
            var ctx = canvas.getContext('2d');
            ctx.fillStyle = 'green';
            ctx.font = '20px monospace';
            ctx.textAlign = 'center';
            ctx.fillText('🌐 NIGHTMARE BROWSER EDITION', canvas.width/2, canvas.height/2 - 20);
            ctx.font = '14px monospace';
            ctx.fillText('Ready for testing!', canvas.width/2, canvas.height/2 + 10);
            ctx.fillText('Call NightmareBrowserEdition.runTest()', canvas.width/2, canvas.height/2 + 30);
        }
        
        console.log('🎯 Ready! Call NightmareBrowserEdition.runTest() to test functionality');
    });
#endif
    
    std::cout << "✅ WebAssembly build successful!" << std::endl;
    std::cout << "🎯 Ready for browser testing!" << std::endl;
    
    return 0;
}