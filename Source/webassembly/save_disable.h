/**
 * @file save_disable.h
 * @brief Save/Load system overrides for Browser Edition
 * 
 * This file provides macros and functions to completely disable
 * the save/load system in Browser Edition builds while maintaining
 * compatibility with the existing codebase.
 */

#pragma once

#include "browser_edition.h"

namespace devilution {

/**
 * @brief Override for SaveGame function in Browser Edition
 * In Browser Edition, this becomes a no-op
 */
inline void SaveGameBrowserOverride() {
    if (IsBrowserEdition()) {
        LogVerbose("🚫 SaveGame called in Browser Edition - ignoring (permadeath mode)");
        return;
    }
    // If not Browser Edition, call original SaveGame
    // This will be handled by conditional compilation
}

/**
 * @brief Override for LoadGame function in Browser Edition
 * In Browser Edition, this becomes a no-op that returns false
 */
inline bool LoadGameBrowserOverride(bool showProgress) {
    if (IsBrowserEdition()) {
        LogVerbose("🚫 LoadGame called in Browser Edition - ignoring (no saves available)");
        return false;
    }
    // If not Browser Edition, call original LoadGame
    // This will be handled by conditional compilation
    return true; // Placeholder
}

/**
 * @brief Check if save file exists (Browser Edition override)
 * In Browser Edition, always returns false (no saves exist)
 */
inline bool SaveFileExistsBrowserOverride() {
    if (IsBrowserEdition()) {
        return false; // No saves in Browser Edition
    }
    // If not Browser Edition, call original function
    return true; // Placeholder
}

/**
 * @brief Override for continue game functionality
 * In Browser Edition, always returns false (no continue available)
 */
inline bool CanContinueGameBrowserOverride() {
    if (IsBrowserEdition()) {
        return false; // No continue in Browser Edition
    }
    // If not Browser Edition, call original function
    return true; // Placeholder
}

} // namespace devilution

// Conditional macros for Browser Edition
#ifdef __EMSCRIPTEN__
    // In WebAssembly builds, override save/load functions
    #define BROWSER_EDITION_SAVE_OVERRIDE() \
        if (::devilution::IsBrowserEdition()) { \
            ::devilution::SaveGameBrowserOverride(); \
            return; \
        }
    
    #define BROWSER_EDITION_LOAD_OVERRIDE(showProgress) \
        if (::devilution::IsBrowserEdition()) { \
            return ::devilution::LoadGameBrowserOverride(showProgress); \
        }
    
    #define BROWSER_EDITION_SAVE_EXISTS_OVERRIDE() \
        if (::devilution::IsBrowserEdition()) { \
            return ::devilution::SaveFileExistsBrowserOverride(); \
        }
    
    #define BROWSER_EDITION_CAN_CONTINUE_OVERRIDE() \
        if (::devilution::IsBrowserEdition()) { \
            return ::devilution::CanContinueGameBrowserOverride(); \
        }
#else
    // In desktop builds, these macros do nothing
    #define BROWSER_EDITION_SAVE_OVERRIDE()
    #define BROWSER_EDITION_LOAD_OVERRIDE(showProgress)
    #define BROWSER_EDITION_SAVE_EXISTS_OVERRIDE()
    #define BROWSER_EDITION_CAN_CONTINUE_OVERRIDE()
#endif