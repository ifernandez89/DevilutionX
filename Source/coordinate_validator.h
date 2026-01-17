#pragma once

#include "player.h"
#include "levels/gendung.h"

namespace devilution {

/**
 * @brief Coordinate Validator - Part of Architectural Health System
 * 
 * This system validates and corrects corrupted coordinates in the game state
 * to prevent location-based crashes. It's designed to be fail-safe and
 * automatically repair corrupted data without user intervention.
 */

/**
 * @brief Validates and corrects player coordinates if corrupted
 * 
 * This function checks if player coordinates are valid and within bounds.
 * If corrupted, it will attempt to find a safe spawn position and correct them.
 * 
 * @param player Reference to the player to validate
 * @return true if coordinates were valid or successfully corrected, false if critical failure
 */
bool ValidateAndFixPlayerCoordinates(Player &player);

/**
 * @brief Validates and corrects all players' coordinates
 * 
 * Called after loading a save game to ensure all player data is valid
 */
void ValidateAllPlayersCoordinates();

/**
 * @brief Validates item coordinates and removes invalid items
 * 
 * Scans all items and removes any with invalid coordinates to prevent crashes
 */
void ValidateAndFixItemCoordinates();

/**
 * @brief Comprehensive coordinate validation system
 * 
 * Validates all game entities (players, items, monsters, objects) and
 * corrects or removes corrupted data. This is the main entry point
 * called after loading a save game.
 */
void ValidateGameStateCoordinates();

} // namespace devilution