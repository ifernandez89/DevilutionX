#pragma once

#include <string>

namespace devilution {

/**
 * @brief Area Dump System - Captures detailed information about the current game area
 * 
 * This system is designed to help identify corrupted objects, items, or monsters
 * that might be causing location-based crashes. When activated (F10 key), it will
 * dump comprehensive information about the current area to a debug file.
 */

/**
 * @brief Captures and saves detailed information about the current game area
 * 
 * This function will create a comprehensive dump of:
 * - Player position and state
 * - All items in the area (with validation)
 * - All monsters in the area (with validation) 
 * - Level geometry and objects
 * - Any other relevant game state
 * 
 * The dump is saved to debug_logs/area_dump_XXXXX.txt where XXXXX is a timestamp
 * 
 * @param trigger Optional string to identify what triggered the dump (default: "manual")
 */
void CaptureAreaDump(const std::string& trigger = "manual");

/**
 * @brief Checks if the area dump hotkey (F10) was pressed and triggers dump if so
 * 
 * This should be called from the main game loop to check for the debug hotkey
 */
void CheckAreaDumpHotkey();

/**
 * @brief Automatically captures area dump when save is loaded
 * 
 * This provides initial state mapping to help identify corrupted objects
 */
void CaptureAreaDumpOnLoad();

} // namespace devilution