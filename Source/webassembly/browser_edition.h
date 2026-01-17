/**
 * @file browser_edition.h
 * @brief Browser Edition specific configuration and overrides
 * 
 * This file contains all the configuration and overrides needed for
 * Nightmare Browser Edition - the permadeath, no-save WebAssembly version.
 * 
 * CORE PRINCIPLES:
 * - No save/load functionality
 * - Permadeath - death = restart
 * - Seed-based deterministic worlds
 * - Session-only state management
 */

#pragma once

#include <cstdint>  // For uint32_t

namespace devilution {

/**
 * @brief Browser Edition configuration flags
 */
struct BrowserEditionConfig {
    // Core features
    static constexpr bool ENABLE_SAVES = false;           // 💀 NO SAVES - Permadeath only
    static constexpr bool ENABLE_LOAD_GAME = false;       // 💀 NO LOADING - Fresh start only
    static constexpr bool ENABLE_CONTINUE = false;        // 💀 NO CONTINUE - Death = restart
    static constexpr bool ENABLE_MULTIPLAYER = false;     // 🌐 Single player only for Web
    
    // Gameplay features
    static constexpr bool FORCE_PERMADEATH = true;        // 💀 Death always restarts
    static constexpr bool SEED_BASED_WORLDS = true;       // 🎲 Deterministic world generation
    static constexpr bool SESSION_ONLY_STATE = true;      // 🔄 No persistent state
    static constexpr bool INSTANT_RESTART = true;         // ⚡ Fast restart on death
    
    // UI features
    static constexpr bool SHOW_SAVE_BUTTON = false;       // 🚫 Hide save UI elements
    static constexpr bool SHOW_LOAD_BUTTON = false;       // 🚫 Hide load UI elements
    static constexpr bool SHOW_CONTINUE_BUTTON = false;   // 🚫 Hide continue UI elements
    static constexpr bool SHOW_SEED_INFO = true;          // 🎲 Show current seed in UI
    
    // Performance features
    static constexpr bool PRELOAD_ALL_ASSETS = true;      // 🚀 Load everything at startup
    static constexpr bool DISABLE_FILE_LOGGING = true;    // 🌐 No file logging in browser
    static constexpr bool MEMORY_ONLY_LOGS = true;        // 📝 Keep logs in memory only
};

/**
 * @brief Check if we're running Browser Edition
 * @return true if this is a WebAssembly Browser Edition build
 */
constexpr bool IsBrowserEdition() {
#ifdef __EMSCRIPTEN__
    return true;
#else
    return false;
#endif
}

/**
 * @brief Initialize Browser Edition specific settings
 * This should be called early in the initialization process
 */
void InitializeBrowserEdition();

/**
 * @brief Handle player death in Browser Edition
 * In Browser Edition, death always triggers a complete restart
 */
void HandleBrowserEditionDeath();

/**
 * @brief Get current world seed for Browser Edition
 * @return Current seed used for world generation
 */
uint32_t GetBrowserEditionSeed();

/**
 * @brief Set world seed for Browser Edition
 * @param seed Seed to use for deterministic world generation
 */
void SetBrowserEditionSeed(uint32_t seed);

/**
 * @brief Restart game with new seed in Browser Edition
 * @param newSeed Optional new seed (0 = generate random seed)
 */
void RestartBrowserEdition(uint32_t newSeed = 0);

/**
 * @brief Check if save/load operations should be disabled
 * @return true if saves are disabled (Browser Edition)
 */
constexpr bool AreSavesDisabled() {
    return IsBrowserEdition() && !BrowserEditionConfig::ENABLE_SAVES;
}

/**
 * @brief Check if multiplayer should be disabled
 * @return true if multiplayer is disabled (Browser Edition)
 */
constexpr bool IsMultiplayerDisabled() {
    return IsBrowserEdition() && !BrowserEditionConfig::ENABLE_MULTIPLAYER;
}

/**
 * @brief Get Browser Edition display name
 * @return String to show in UI for this edition
 */
const char* GetBrowserEditionName();

/**
 * @brief Get Browser Edition version info
 * @return Version string for Browser Edition
 */
const char* GetBrowserEditionVersion();

} // namespace devilution