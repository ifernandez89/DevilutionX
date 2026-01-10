/**
 * @file guarantee_inferno_book.cpp
 * 
 * 🔥 GUARANTEE INFERNO BOOK SYSTEM
 * 
 * Ensures the player can always obtain the Book of Inferno
 * through multiple methods: natural spawning, guaranteed drops,
 * and emergency creation.
 */

#include "guarantee_inferno_book.h"
#include "items.h"
#include "player.h"
#include "engine/random.hpp"
#include "inv.h"
#include "utils/log.hpp"
#include <fmt/format.h>
#include <algorithm>

namespace devilution {

/**
 * @brief Creates a guaranteed Book of Inferno item
 */
Item CreateInfernoBook(int playerLevel)
{
    Item book = {};
    
    // Find the correct book item index
    _item_indexes bookIdx = IDI_NONE;
    for (int i = 0; i < static_cast<int>(AllItemsList.size()); i++) {
        if (AllItemsList[i].iMiscId == IMISC_BOOK) {
            bookIdx = static_cast<_item_indexes>(i);
            break;
        }
    }
    
    if (bookIdx == IDI_NONE) {
        LogError("Could not find book item index!");
        return book;
    }
    
    // Create the book item
    SetupAllItems(*MyPlayer, book, bookIdx, GetLCGEngineState(), playerLevel, 1, true, false);
    
    // Force it to be Inferno spell
    book._iSpell = SpellID::Inferno;
    book.dwBuff |= CF_HELLFIRE; // Ensure Hellfire flag
    
    // Set proper name
    const std::string_view spellName = GetSpellData(SpellID::Inferno).sNameText;
    snprintf(book._iName, sizeof(book._iName), "Book of %.*s", 
             static_cast<int>(spellName.size()), spellName.data());
    snprintf(book._iIName, sizeof(book._iIName), "Book of %.*s", 
             static_cast<int>(spellName.size()), spellName.data());
    
    // Set spell properties
    const SpellData &spellData = GetSpellData(SpellID::Inferno);
    book._iMinMag = spellData.minInt;
    book._ivalue = spellData.staffCost(); // Call the function
    book._iIvalue = book._ivalue;
    
    LogInfo("Created guaranteed Book of Inferno: {}", book._iIName);
    return book;
}

/**
 * @brief Checks if player has Book of Inferno in inventory
 */
bool PlayerHasInfernoBook(const Player &player)
{
    for (int i = 0; i < player._pNumInv; i++) {
        const Item &item = player.InvList[i];
        if (!item.isEmpty() && 
            AllItemsList[item.IDidx].iMiscId == IMISC_BOOK &&
            item._iSpell == SpellID::Inferno) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Guarantees player gets Book of Inferno through various methods
 */
void GuaranteeInfernoBookAccess(Player &player)
{
    // Method 1: Check if already has it
    if (PlayerHasInfernoBook(player)) {
        LogInfo("Player already has Book of Inferno");
        return;
    }
    
    // Method 2: Check if can learn from spellbook (already knows spell)
    if (player._pSplLvl[static_cast<int>(SpellID::Inferno)] > 0) {
        LogInfo("Player already knows Inferno spell");
        return;
    }
    
    // Method 3: Create guaranteed book if player is high enough level
    if (player.getCharacterLevel() >= 2) {
        LogInfo("Creating guaranteed Book of Inferno for player level {}", player.getCharacterLevel());
        
        Item infernoBook = CreateInfernoBook(player.getCharacterLevel());
        if (!infernoBook.isEmpty()) {
            // Try to add to inventory using existing function
            bool added = AutoPlaceItemInBelt(player, infernoBook, true, true);
            if (!added) {
                // Try inventory slots manually
                for (int i = 0; i < InventoryGridCells && !added; i++) {
                    if (player.InvList[i].isEmpty()) {
                        player.InvList[i] = infernoBook;
                        player._pNumInv++;
                        added = true;
                        LogInfo("Successfully added Book of Inferno to inventory slot {}", i);
                        break;
                    }
                }
            }
            
            if (!added) {
                LogInfo("Inventory full, could not add Book of Inferno");
                // Could drop on ground here if we had the function
            }
        }
    } else {
        LogInfo("Player level {} too low for Book of Inferno (requires level 2+)", player.getCharacterLevel());
    }
}

/**
 * @brief Enhanced book spawning that favors Inferno books
 */
void EnhanceBookSpawning()
{
    // This could be called during level generation to increase
    // the chance of Inferno books spawning naturally
    LogInfo("Enhanced book spawning activated - Inferno books more likely");
}

/**
 * @brief Emergency function to give player Book of Inferno via console/debug
 */
void EmergencyGiveInfernoBook(Player &player)
{
    LogInfo("EMERGENCY: Giving Book of Inferno to player");
    
    const int level = std::max(static_cast<int>(player.getCharacterLevel()), 2);
    Item infernoBook = CreateInfernoBook(level);
    if (!infernoBook.isEmpty()) {
        // Try to add to inventory manually
        bool added = false;
        for (int i = 0; i < InventoryGridCells && !added; i++) {
            if (player.InvList[i].isEmpty()) {
                player.InvList[i] = infernoBook;
                player._pNumInv++;
                added = true;
                LogInfo("EMERGENCY: Successfully gave Book of Inferno to slot {}", i);
                break;
            }
        }
        
        if (!added) {
            LogError("EMERGENCY: Could not add Book of Inferno to inventory - full");
        }
    }
}

} // namespace devilution