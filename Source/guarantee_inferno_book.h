/**
 * @file guarantee_inferno_book.h
 * 
 * 🔥 GUARANTEE INFERNO BOOK SYSTEM
 * 
 * Header for ensuring Book of Inferno availability
 */

#pragma once

#include "items.h"
#include "player.h"

namespace devilution {

/**
 * @brief Creates a guaranteed Book of Inferno item
 * @param playerLevel Level to create the book for
 * @return Book of Inferno item
 */
Item CreateInfernoBook(int playerLevel);

/**
 * @brief Checks if player has Book of Inferno in inventory
 * @param player Player to check
 * @return True if player has Book of Inferno
 */
bool PlayerHasInfernoBook(const Player &player);

/**
 * @brief Guarantees player gets Book of Inferno through various methods
 * @param player Player to guarantee book for
 */
void GuaranteeInfernoBookAccess(Player &player);

/**
 * @brief Enhanced book spawning that favors Inferno books
 */
void EnhanceBookSpawning();

/**
 * @brief Emergency function to give player Book of Inferno
 * @param player Player to give book to
 */
void EmergencyGiveInfernoBook(Player &player);

} // namespace devilution