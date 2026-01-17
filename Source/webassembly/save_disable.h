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

// External declaration for gbValidSaveFile
extern bool gbValidSaveFile;

/**
 * @brief Override for SaveGame function in Browser Edition
 * In Browser Edition, this becomes a no-op
 */
inline void SaveGameBrowserOverride() {
    if (IsBrowserEdition()) {
        LogInfo("🚫 SaveGame blocked in Browser Edition (permadeath mode)");
        return;
    }
}

/**
 * @brief Override for LoadGame function in Browser Edition
 * In Browser Edition, this returns an error
 */
inline tl::expected<void, std::string> LoadGameBrowserOverride(bool showProgress) {
    if (IsBrowserEdition()) {
        LogInfo("🚫 LoadGame blocked in Browser Edition (no saves available)");
        return tl::make_unexpected(std::string("Load game disabled in Browser Edition"));
    }
    // This should never be reached due to macro usage
    return {};
}

/**
 * @brief Override for save file existence check in Browser Edition
 * In Browser Edition, always sets gbValidSaveFile to false
 */
inline void SaveFileExistsBrowserOverride() {
    if (IsBrowserEdition()) {
        LogVerbose("🚫 Save file existence check - forcing false in Browser Edition");
        gbValidSaveFile = false;
        return;
    }
}

/**
 * @brief Check if continue game is available (Browser Edition override)
 * In Browser Edition, always returns false (no continue available)
 */
inline bool CanContinueGameBrowserOverride() {
    if (IsBrowserEdition()) {
        return false; // No continue in Browser Edition
    }
    return true; // Placeholder for non-browser builds
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
            ::devilution::SaveFileExistsBrowserOverride(); \
            return; \
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